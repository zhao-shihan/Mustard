// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
//
// This file is part of Mustard, an offline software framework for HEP experiments.
//
// Mustard is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// Mustard is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// Mustard. If not, see <https://www.gnu.org/licenses/>.

#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Utility/PrettyLog.h++"
#include "Mustard/Utility/Print.h++"

#include "TROOT.h"

#include "mpi.h"

#include "muc/algorithm"
#include "muc/utility"

#include "fmt/format.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <thread>
#include <utility>

namespace Mustard::Env {

MPIEnv::MPIEnv(NoBanner, int argc, char* argv[],
               std::optional<std::reference_wrapper<CLI::CLI<>>> cli,
               enum VerboseLevel verboseLevel,
               bool showBannerHint) :
    BasicEnv{{}, argc, argv, cli, verboseLevel, showBannerHint},
    PassiveSingleton<MPIEnv>{this},
    fCluster{},
    fCommNode{} {
    const auto& commWorld{mpl::environment::comm_world()};
    if (mpl::environment::threading_mode() == mpl::threading_modes::single) {
        Throw<std::runtime_error>("The MPI library not even support funneled threading");
    }

    std::array<char, MPI_MAX_PROCESSOR_NAME> localNodeName{};
    std::ranges::copy(mpl::environment::processor_name(), localNodeName.begin());
    std::vector<std::array<char, MPI_MAX_PROCESSOR_NAME>> nodeName(commWorld.size());
    commWorld.allgather(localNodeName, nodeName.data());

    std::vector<std::pair<std::string_view, int>> nodeRankMap(commWorld.size());
    for (int rank{}; rank < commWorld.size(); ++rank) {
        nodeRankMap[rank] = {nodeName[rank].data(), rank};
    }
    muc::timsort(nodeRankMap);

    auto currentNodeName{nodeRankMap.front().first};
    fCluster.node.push_back({std::string{currentNodeName}, {}});
    for (auto&& [nodeName, rank] : std::as_const(nodeRankMap)) {
        if (nodeName != currentNodeName) {
            currentNodeName = nodeName;
            fCluster.node.push_back({std::string{currentNodeName}, {}});
        }
        fCluster.node.back().worldRank.emplace_back(rank);
    }
    muc::timsort(fCluster.node,
                 [](auto&& node1, auto&& node2) {
                     return node1.worldRank < node2.worldRank;
                 });

    const auto localNode{std::ranges::find_if(
        std::as_const(fCluster.node),
        [rank = commWorld.rank()](auto&& node) {
            return muc::ranges::contains(node.worldRank, rank);
        })};
    fCluster.local = localNode - fCluster.node.cbegin();

    fCommNode = mpl::communicator{mpl::communicator::split, commWorld, fCluster.local};

    // Disable ROOT implicit multi-threading
    if (ROOT::IsImplicitMTEnabled()) {
        ROOT::DisableImplicitMT();
    }
}

MPIEnv::MPIEnv(int argc, char* argv[],
               std::optional<std::reference_wrapper<CLI::CLI<>>> cli,
               enum VerboseLevel verboseLevel,
               bool showBannerHint) :
    MPIEnv{{}, argc, argv, cli, verboseLevel, showBannerHint} {
    if (fShowBanner and mpl::environment::comm_world().rank() == 0) {
        PrintStartBannerSplitLine();
        PrintStartBannerBody(argc, argv);
        PrintStartBannerSplitLine();
    }
}

MPIEnv::~MPIEnv() {
    const auto& commWorld{mpl::environment::comm_world()};
    // Wait all processes before finalizing
    auto barrier{commWorld.ibarrier()};
    while (not barrier.test()) {
        using std::chrono_literals::operator""ms;
        std::this_thread::sleep_for(20ms);
    }
    // Show exit banner
    if (fShowBanner and commWorld.rank() == 0) {
        PrintExitBanner();
    }
    fShowBanner = false;
}

auto MPIEnv::PrintStartBannerBody(int argc, char* argv[]) const -> void {
    BasicEnv::PrintStartBannerBody(argc, argv);
    // MPI library version
    char mpiLibVersion[MPI_MAX_LIBRARY_VERSION_STRING];
    int mpiLibVersionStringLength;
    MPI_Get_library_version(mpiLibVersion, &mpiLibVersionStringLength);
    // MPI version at runtime
    std::pair<int, int> mpiRuntimeVersion;
    MPI_Get_version(&mpiRuntimeVersion.first, &mpiRuntimeVersion.second);
    // Messages
    const auto& commWorld{mpl::environment::comm_world()};
    Print(fmt::emphasis::bold,
          "\n"
          " Parallelized with MPI, running {}\n",
          commWorld.size() == 1 ? "sequentially" : "in parallel");
    PrintLn<'I'>(" Compiled with MPI {}.{}, running with MPI {}.{}", MPI_VERSION, MPI_SUBVERSION, mpiRuntimeVersion.first, mpiRuntimeVersion.second);
    Print<'V'>("--------------------> MPI library information (begin) <--------------------\n"
               "{}\n"
               "-------------------->  MPI library information (end)  <--------------------\n"
               "\n",
               mpiLibVersion);
    Print(fmt::emphasis::bold, " Size of the MPI world communicator: {}\n", commWorld.size());
    if (OnSingleNode()) {
        Print(fmt::emphasis::bold, " Running on '{}'\n", LocalNode().name);
    } else {
        Print(fmt::emphasis::bold, " Running on {} nodes:\n", ClusterSize());
        const auto maxNameWidth{std::ranges::max_element(
                                    NodeList(),
                                    [](auto&& node1, auto&& node2) {
                                        return node1.name.size() < node2.name.size();
                                    })
                                    ->name.size()};
        const auto format{fmt::format("  {{:{}}}: {{}} ({{}})\n", maxNameWidth)};
        for (int nodeID{}; nodeID < ClusterSize(); ++nodeID) {
            const auto& node{Node(nodeID)};
            std::vector<std::string> rankStringPart;
            auto rankStart{node.worldRank.front()};
            auto rankEnd{node.worldRank.front()};
            for (auto&& r : node.worldRank) {
                if (r == rankEnd + 1) {
                    rankEnd = r;
                } else {
                    if (rankStart == rankEnd) {
                        rankStringPart.push_back(std::to_string(rankStart));
                    } else {
                        rankStringPart.push_back(fmt::format("{}-{}", rankStart, rankEnd));
                    }
                    rankStart = rankEnd = r;
                }
            }
            if (rankStart == rankEnd) {
                rankStringPart.push_back(std::to_string(rankStart));
            } else {
                rankStringPart.push_back(fmt::format("{}-{}", rankStart, rankEnd));
            }
            const auto rankString{fmt::format("{}", fmt::join(rankStringPart, ","))};
            const auto nRank{node.worldRank.size()};
            VPrint(stdout, fmt::emphasis::bold, format, fmt::make_format_args(node.name, rankString, nRank));
        }
    }
}

} // namespace Mustard::Env
