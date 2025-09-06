// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/IO/Print.h++"

#include "TROOT.h"

#include "muc/algorithm"
#include "muc/numeric"
#include "muc/utility"

#include "gsl/gsl"

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
#include <string_view>
#include <thread>
#include <utility>

namespace Mustard::Env {

MPIEnv::MPIEnv(NoBanner, int& argc, char**& argv,
               muc::optional_ref<CLI::CLI<>> cli,
               enum VerboseLevel verboseLevel,
               bool showBannerHint) :
    BasicEnv{{}, argc, argv, cli, verboseLevel, showBannerHint},
    PassiveSingleton<MPIEnv>{this},
    fIntraNodeComm{},
    fInterNodeComm{},
    fLocalNodeID{},
    fNodeList{} {
    mplr::init(argc, argv);

    const auto worldComm{mplr::comm_world()};
    fIntraNodeComm = mplr::communicator{mplr::communicator::split_shared_memory, worldComm};
    enum struct IntraNodeColor { Leader = 0,
                                 Other = MPI_UNDEFINED };
    fInterNodeComm = mplr::communicator{mplr::communicator::split, worldComm,
                                        fIntraNodeComm.rank() == 0 ?
                                            IntraNodeColor::Leader :
                                            IntraNodeColor::Other};

    fLocalNodeID = fInterNodeComm.is_valid() ? fInterNodeComm.rank() : 0;
    fIntraNodeComm.bcast(0, fLocalNodeID);

    auto nNode{fInterNodeComm.is_valid() ? fInterNodeComm.size() : 0};
    fIntraNodeComm.bcast(0, nNode);
    fNodeList.resize(nNode);

    using NodeNameType = std::array<char, MPI_MAX_PROCESSOR_NAME>;
    std::vector<NodeNameType> nodeName(nNode);
    if (fInterNodeComm.is_valid()) {
        NodeNameType localNodeName{};
        std::ranges::copy(mplr::processor_name(), localNodeName.begin());
        fInterNodeComm.allgather(localNodeName, nodeName.data());
    }
    fIntraNodeComm.bcast(0, nodeName.data(), mplr::vector_layout<NodeNameType>(nNode));

    std::vector<int> nodeSize(nNode);
    std::vector<int> localWorldRank(fInterNodeComm.is_valid() ? fIntraNodeComm.size() : 0);
    fIntraNodeComm.gather(0, worldComm.rank(), localWorldRank.data());
    if (fInterNodeComm.is_valid()) {
        fInterNodeComm.allgather(gsl::narrow<int>(localWorldRank.size()), nodeSize.data());
    }
    fIntraNodeComm.bcast(0, nodeSize.data(), mplr::vector_layout<int>(nNode));

    mplr::displacements disp(nNode);
    for (auto i{1}; i < nNode; ++i) {
        disp[i] = disp[i - 1] + nodeSize[i - 1];
    }
    std::vector<int> flatWorldRank(worldComm.size());
    if (fInterNodeComm.is_valid()) {
        mplr::contiguous_layouts<int> worldRankLayout(nNode);
        std::ranges::transform(nodeSize, worldRankLayout.begin(), [](int size) { return mplr::contiguous_layout<int>(size); });
        fInterNodeComm.allgatherv(localWorldRank.data(), mplr::contiguous_layout<int>(localWorldRank.size()),
                                  flatWorldRank.data(), worldRankLayout, disp);
    }
    fIntraNodeComm.bcast(0, flatWorldRank.data(), mplr::vector_layout<int>{flatWorldRank.size()});

    for (int i{}; i < nNode; ++i) {
        auto& node{fNodeList[i]};
        const auto nameEnd{std::ranges::find(std::as_const(nodeName[i]), '\0')};
        node.name = std::string_view{nodeName[i].cbegin(), nameEnd};
        node.size = nodeSize[i];
        node.worldRank.resize(node.size);
        std::ranges::copy_n(flatWorldRank.cbegin() + disp[i], nodeSize[i], node.worldRank.begin());
    }

    // Disable ROOT implicit multi-threading since we are using MPI
    if (ROOT::IsImplicitMTEnabled()) {
        ROOT::DisableImplicitMT();
    }
}

MPIEnv::MPIEnv(int argc, char* argv[],
               muc::optional_ref<CLI::CLI<>> cli,
               enum VerboseLevel verboseLevel,
               bool showBannerHint) :
    MPIEnv{{}, argc, argv, cli, verboseLevel, showBannerHint} {
    if (fShowBanner and mplr::comm_world().rank() == 0) {
        PrintStartBannerSplitLine();
        PrintStartBannerBody(argc, argv);
        PrintStartBannerSplitLine();
    }
}

MPIEnv::~MPIEnv() {
    const auto worldComm{mplr::comm_world()};
    // Wait all processes before finalizing
    worldComm.ibarrier().wait(mplr::duty_ratio::preset::relaxed);
    // Show exit banner
    if (fShowBanner and worldComm.rank() == 0) {
        PrintExitBanner();
    }
    fShowBanner = false;
}

auto MPIEnv::PrintStartBannerBody(int argc, char* argv[]) const -> void {
    BasicEnv::PrintStartBannerBody(argc, argv);
    // MPI library version
    const auto mpiLibVersion{mplr::get_library_version()};
    // MPI version at runtime
    const auto mpiRuntimeVersion{mplr::get_version()};
    // Messages
    const auto worldComm{mplr::comm_world()};
    Print(fmt::emphasis::bold,
          "\n"
          " Parallelized with MPI, running {}\n",
          worldComm.size() == 1 ? "sequentially" : "in parallel");
    PrintLn<'I'>(" Compiled with MPI {}.{}, running with MPI {}.{}", MPI_VERSION, MPI_SUBVERSION, mpiRuntimeVersion.first, mpiRuntimeVersion.second);
    Print<'V'>("--------------------> MPI library information (begin) <--------------------\n"
               "{}\n"
               "-------------------->  MPI library information (end)  <--------------------\n"
               "\n",
               mpiLibVersion);
    Print(fmt::emphasis::bold, " Size of the MPI world communicator: {}\n", worldComm.size());
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
        for (auto&& node : fNodeList) {
            std::string rankString;
            auto AddRankInterval{[&](int start, int end) {
                rankString.append(start == end ?
                                      fmt::format("{},", start) :
                                      fmt::format("{}-{},", start, end));
            }};
            auto currentBegin{node.worldRank.front()};
            auto currentEnd{currentBegin};
            for (auto it{std::next(node.worldRank.cbegin())}; it != node.worldRank.cend(); ++it) {
                if (*it == currentEnd + 1) {
                    currentEnd = *it;
                } else {
                    AddRankInterval(currentBegin, currentEnd);
                    currentBegin = currentEnd = *it;
                }
            }
            AddRankInterval(currentBegin, currentEnd);
            rankString.erase(rankString.length() - 1); // remove last ','
            const auto nRank{node.size};
            VPrint(stdout, fmt::emphasis::bold, format, fmt::make_format_args(node.name, rankString, nRank));
        }
    }
}

} // namespace Mustard::Env
