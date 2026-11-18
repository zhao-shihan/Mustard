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

#include "Mustard/CLI/BasicCLI.h++"
#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Testing/TestMPIEnv/TestMPIEnv.h++"

#include "mp1/mp1.hpp"

#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <vector>

namespace Mustard::Testing {

TestMPIEnv::TestMPIEnv() :
    Subprogram{"TestMPIEnv", "Test Mustard::Env::MPIEnv and its mp1 lifecycle."} {}

auto TestMPIEnv::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::MPIEnv env{argc, argv, cli};

    if (mp1::query_thread() != mp1::threading_mode::multiple) {
        Throw<std::runtime_error>("MPIEnv did not initialize MPI with MPI_THREAD_MULTIPLE.");
    }
    if (mp1::provided() != mp1::threading_mode::multiple) {
        Throw<std::runtime_error>("mp1 did not record MPI_THREAD_MULTIPLE as the provided level.");
    }

    const auto& worldComm{mp1::comm_world()};
    if (worldComm.size() <= 0) {
        Throw<std::runtime_error>("The MPI world communicator has an invalid size.");
    }

    const auto& intraNodeComm{env.IntraNodeComm()};
    const auto& interNodeComm{env.InterNodeComm()};
    if (intraNodeComm.size() <= 0 or intraNodeComm.rank() < 0 or intraNodeComm.rank() >= intraNodeComm.size()) {
        Throw<std::runtime_error>("The intra-node communicator has an invalid rank or size.");
    }
    if (interNodeComm.is_valid() != (intraNodeComm.rank() == 0)) {
        Throw<std::runtime_error>("The inter-node communicator validity does not match intra-node leadership.");
    }
    if (interNodeComm.is_valid()) {
        if (interNodeComm.size() != env.ClusterSize() or
            interNodeComm.rank() != env.LocalNodeIdx()) {
            Throw<std::runtime_error>("The inter-node communicator does not match the node table.");
        }
    }

    const auto& nodeList{env.NodeList()};
    if (nodeList.size() != static_cast<std::size_t>(env.ClusterSize()) or nodeList.empty()) {
        Throw<std::runtime_error>("The node table has an invalid size.");
    }
    std::vector<int> allWorldRank;
    for (int i{}; i < env.ClusterSize(); ++i) {
        const auto& node{env.Node(i)};
        if (node.name.empty() or node.size <= 0 or node.size != static_cast<int>(node.worldRank.size())) {
            Throw<std::runtime_error>("The node table contains an invalid node entry.");
        }
        if (not std::ranges::is_sorted(node.worldRank) or
            std::ranges::adjacent_find(node.worldRank) != node.worldRank.end()) {
            Throw<std::runtime_error>("A node's world-rank list is not strictly sorted.");
        }
        for (auto rank : node.worldRank) {
            if (rank < 0 or rank >= worldComm.size()) {
                Throw<std::runtime_error>("A node's world-rank entry is outside the world communicator.");
            }
        }
        allWorldRank.insert(allWorldRank.end(), node.worldRank.begin(), node.worldRank.end());
    }
    std::ranges::sort(allWorldRank);
    if (allWorldRank.size() != static_cast<std::size_t>(worldComm.size())) {
        Throw<std::runtime_error>("The node table does not cover the world communicator.");
    }
    for (int rank{}; rank < worldComm.size(); ++rank) {
        if (allWorldRank[rank] != rank) {
            Throw<std::runtime_error>("The node table does not contain every world rank exactly once.");
        }
    }

    const auto& localNode{env.LocalNode()};
    if (std::ranges::find(localNode.worldRank, worldComm.rank()) == localNode.worldRank.end()) {
        Throw<std::runtime_error>("LocalNode does not contain the calling world rank.");
    }
    if (env.LocalNodeIdx() < 0 or env.LocalNodeIdx() >= env.ClusterSize()) {
        Throw<std::runtime_error>("LocalNodeIdx is outside the node table.");
    }

    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
