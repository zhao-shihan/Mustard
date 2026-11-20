// -*- C++ -*-
//
// Copyright (C) 2020-2026  Shihan Zhao and contributors
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
#include "Mustard/Env/impl/EnvBase.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/IO/Print.h++"

#include "TROOT.h"

#include "gsl/gsl"

#include "fmt/format.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <iterator>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace Mustard::Env {

MPIEnv::MPIEnv(NoBanner, int argc, char* argv[],
               muc::optional_ref<CLI::CLI<>> cli,
               enum VerboseLevel verboseLevel,
               bool showBannerHint) :
    impl::EnvBase(impl::EnableMPI{argc, argv}),
    BasicEnv{{}, argc, argv, cli, verboseLevel, showBannerHint},
    PassiveSingleton<MPIEnv>{this},
    fIntraNodeComm{mp1::communicator::split_type(mp1::comm_world(), mp1::comm_type_shared)},
    fInterNodeComm{mp1::communicator::split(mp1::comm_world(), fIntraNodeComm.rank() == 0 ? 0 : MPI_UNDEFINED)},
    fLocalNodeIdx{},
    fNodeList{} {
    const auto& worldComm{mp1::comm_world()};
    fLocalNodeIdx = fInterNodeComm.is_valid() ? fInterNodeComm.rank() : 0;
    mp1::bcast(fIntraNodeComm, 0, fLocalNodeIdx);

    auto nNode{fInterNodeComm.is_valid() ? fInterNodeComm.size() : 0};
    mp1::bcast(fIntraNodeComm, 0, nNode);
    fNodeList.resize(nNode);

    constexpr std::size_t maxProcName{MPI_MAX_PROCESSOR_NAME};
    using NodeNameType = std::array<char, maxProcName>;
    std::vector<char> nodeName(nNode * maxProcName);
    if (fInterNodeComm.is_valid()) {
        NodeNameType localNodeName{};
        const auto localNodeNameString{mp1::processor_name()};
        std::ranges::copy_n(localNodeNameString.cbegin(),
                            std::min(localNodeNameString.size(), localNodeName.size()),
                            localNodeName.begin());
        mp1::allgather(fInterNodeComm, localNodeName, nodeName);
    }
    mp1::bcast(fIntraNodeComm, 0, nodeName);

    std::vector<int> nodeSize(nNode);
    std::vector<int> localWorldRank(fInterNodeComm.is_valid() ? fIntraNodeComm.size() : 0);
    const auto localWorldRankValue{worldComm.rank()};
    mp1::gather(fIntraNodeComm, 0, localWorldRankValue, localWorldRank);
    if (fInterNodeComm.is_valid()) {
        const auto localWorldRankCount{gsl::narrow<int>(localWorldRank.size())};
        mp1::allgather(fInterNodeComm, localWorldRankCount, nodeSize);
    }
    mp1::bcast(fIntraNodeComm, 0, nodeSize);

    std::vector<mp1::count_t> disp(nNode);
    for (auto i{1}; i < nNode; ++i) {
        disp[i] = disp[i - 1] + nodeSize[i - 1];
    }
    std::vector<int> flatWorldRank(worldComm.size());
    if (fInterNodeComm.is_valid()) {
        std::vector<mp1::layout<int>> worldRankLayout(nNode);
        std::ranges::transform(nodeSize, worldRankLayout.begin(),
                               [](int size) { return mp1::layout<int>::contiguous(size); });
        mp1::allgatherv(fInterNodeComm, localWorldRank, flatWorldRank,
                        worldRankLayout,
                        disp);
    }
    mp1::bcast(fIntraNodeComm, 0, flatWorldRank);

    const std::span nodeNameView{nodeName};
    for (int i{}; i < nNode; ++i) {
        auto& node{fNodeList[i]};
        const auto nameSpan{nodeNameView.subspan(i * maxProcName, maxProcName)};
        const auto nameEnd{std::ranges::find(nameSpan, '\0')};
        node.name = std::string_view{nameSpan.begin(), nameEnd};
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
    if (fShowBanner and mp1::comm_world().rank() == 0) {
        PrintStartBannerSplitLine();
        PrintStartBannerBody(argc, argv);
        PrintStartBannerSplitLine();
    }
}

MPIEnv::~MPIEnv() {
    const auto& worldComm{mp1::comm_world()};
    // Wait all processes before finalizing
    mp1::barrier(worldComm);
    // Show exit banner
    if (fShowBanner and worldComm.rank() == 0) {
        PrintExitBanner();
    }
    fShowBanner = false;
}

auto MPIEnv::PrintStartBannerBody(int argc, char* argv[]) const -> void {
    BasicEnv::PrintStartBannerBody(argc, argv);
    // MPI library version
    const auto mpiLibVersion{mp1::get_library_version()};
    // MPI version at runtime
    const auto mpiRuntimeVersion{mp1::get_version()};
    // Messages
    const auto& worldComm{mp1::comm_world()};
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
