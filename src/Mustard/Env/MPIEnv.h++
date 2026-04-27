// -*- C++ -*-
//
// Copyright (C) 2020-2025  Mustard developers
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

#pragma once

#include "Mustard/CLI/CLI.h++"
#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/Env/ObjectRegistry/PassiveSingleton.h++"

#include "mplr/mplr.hpp"

#include "muc/optional"

#include <concepts>
#include <functional>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

namespace Mustard::Env {

/// @brief MPI-aware runtime environment for distributed Mustard applications.
/// @details This environment initializes MPI via MPLR and derives a two-level
/// communicator topology:
/// - Intra-node communicator (`IntraNodeComm`) involves processes sharing
///   the same node (shared-memory domain).
/// - Inter-node communicator (`InterNodeComm`) involves one leader process
///   per node for cross-node communication. Leader processes are processes
///   ranked 0 in their respective intra-node communicators, or practically,
///   they are processes that possess a valid inter-node communicator object.
///
/// The class also builds a node table that maps node names to world ranks,
/// allowing code to reason about cluster layout.
class MPIEnv : virtual public BasicEnv,
               public PassiveSingleton<MPIEnv> {
protected:
    /// @brief Constructs the environment without printing startup banner.
    /// @param argc Argument count passed to `main`.
    /// @param argv Argument vector passed to `main`.
    /// @param cli Optional CLI object for argument processing.
    /// @param verboseLevel Logging verbosity inherited from BasicEnv.
    /// @param showBannerHint Whether banner hint should still be tracked.
    /// @note Intended for derived/internal construction paths.
    MPIEnv(NoBanner, int argc, char* argv[],
           muc::optional_ref<CLI::CLI<>> cli,
           enum VerboseLevel verboseLevel,
           bool showBannerHint);

public:
    /// @brief Constructs the MPI environment and optionally prints banners.
    /// @param argc Argument count passed to `main`.
    /// @param argv Argument vector passed to `main`.
    /// @param cli Optional CLI object for argument processing.
    /// @param verboseLevel Logging verbosity inherited from BasicEnv.
    /// @param showBannerHint Whether start/exit banner behavior is enabled.
    MPIEnv(int argc, char* argv[],
           muc::optional_ref<CLI::CLI<>> cli = {},
           enum VerboseLevel verboseLevel = {},
           bool showBannerHint = true);

    /// @brief Synchronizes all processes and finalizes environment shutdown.
    /// @details An MPI non-blocking barrier is waited before teardown to keep
    /// shutdown output consistent across processes.
    virtual ~MPIEnv();

    using PassiveSingleton<MPIEnv>::Instance;
    using PassiveSingleton<MPIEnv>::Available;
    using PassiveSingleton<MPIEnv>::Expired;
    using PassiveSingleton<MPIEnv>::Instantiated;

    /// @brief Returns communicator grouping processes on the same physical node.
    /// @return Intra-node communicator derived from `MPI_COMM_WORLD` using
    /// shared-memory split.
    auto IntraNodeComm() const -> const auto& { return fIntraNodeComm; }
    /// @brief Returns communicator containing one leader rank per node.
    /// Leader processes are processes ranked 0 in their respective intra-node
    /// communicators, or practically, they are processes that possess a valid
    /// inter-node communicator object.
    /// @return Inter-node communicator; invalid for non-leader processes.
    auto InterNodeComm() const -> const auto& { return fInterNodeComm; }

    /// @brief Returns local node index in the cluster node list.
    auto LocalNodeIdx() const -> const auto& { return fLocalNodeIdx; }
    /// @brief Returns metadata of the node where current rank runs.
    auto LocalNode() const -> const auto& { return fNodeList[fLocalNodeIdx]; }
    /// @brief Returns metadata of all discovered nodes.
    auto NodeList() const -> const auto& { return fNodeList; }
    /// @brief Returns metadata for node @p id.
    /// @param id Node index in [0, ClusterSize()).
    auto Node(int id) const -> const auto& { return fNodeList.at(id); }
    /// @brief Returns total number of participating nodes.
    auto ClusterSize() const -> int { return fNodeList.size(); }
    /// @brief Checks whether all MPI processes are on a single node.
    auto OnSingleNode() const -> auto { return ClusterSize() == 1; }
    /// @brief Checks whether MPI processes span multiple nodes.
    auto OnCluster() const -> auto { return ClusterSize() != 1; }

protected:
    auto PrintStartBannerBody(int argc, char* argv[]) const -> void;

private:
    /// @brief Metadata aggregated per physical node.
    struct Node {
        std::string name;           ///< Hostname (MPI processor name) of the node.
        int size;                   ///< Number of world processes located on this node.
        std::vector<int> worldRank; ///< Sorted world-rank list belonging to this node.
    };

private:
    mplr::communicator fIntraNodeComm;
    mplr::communicator fInterNodeComm;

    int fLocalNodeIdx;
    std::vector<struct Node> fNodeList;
};

} // namespace Mustard::Env
