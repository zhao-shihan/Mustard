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

#pragma once

#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Execution/Dispatcher.h++"
#include "Mustard/IO/PrettyLog.h++"

#include "mplr/mplr.hpp"

#include "muc/math"
#include "muc/numeric"

#include "gsl/gsl"

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace Mustard::inline Execution {

/// @class ClusterAwareMasterWorkerDispatcher
/// @brief Hierarchical dynamic dispatcher for multi-node clusters.
///
/// The scheduler has three levels: a cluster master distributes inter-node
/// batches to node masters, each node master distributes intra-node batches to
/// its workers, and workers execute a batch before requesting the next one.
/// This keeps most scheduling traffic inside each shared-memory domain while
/// still balancing load across nodes.
///
/// @note Recommended for large multi-node clusters where inter-node communication
///       is expensive. DefaultDispatcherCode() selects it for worlds with more than
///       128 processes when at least one node has more than 8 processes.
/// @tparam T Task index type
/// @see Executor
template<std::integral T>
class ClusterAwareMasterWorkerDispatcher : public Dispatcher<T> {
private:
    friend class ClusterMaster;
    /// @brief Cluster-level master that distributes tasks to node masters.
    class ClusterMaster {
    public:
        /// @brief Construct the cluster master.
        /// @param s Owning dispatcher
        ClusterMaster(ClusterAwareMasterWorkerDispatcher<T>* s);

        /// @brief Start all receive requests from node masters.
        auto StartAll() -> void;
        /// @brief Serve node-master requests until all inter-node batches are assigned.
        auto operator()() -> void;

    private:
        ClusterAwareMasterWorkerDispatcher<T>* fS; ///< Owning dispatcher
        mplr::prequest_pool fRecvFromNM;           ///< Pending receives from node masters
        std::vector<T> fTaskIDSendToNM;            ///< Task ID sent to each node master
        mplr::prequest_pool fSendToNM;             ///< Pending sends to node masters
    };

    friend class NodeMaster;
    /// @brief Node-level master that distributes tasks within one node.
    class NodeMaster {
    public:
        /// @brief Construct the node master.
        /// @param s Owning dispatcher
        NodeMaster(ClusterAwareMasterWorkerDispatcher<T>* s);

        /// @brief Start receive requests, start the cluster master on the root node, and synchronize.
        auto StartAll() -> void;
        /// @brief Serve worker requests and fetch more work from the cluster master when needed.
        auto operator()() -> void;

    private:
        ClusterAwareMasterWorkerDispatcher<T>* fS; ///< Owning dispatcher

        std::unique_ptr<ClusterMaster> fClusterMaster; ///< Cluster master (root node only)
        std::jthread fClusterMasterThread;             ///< Cluster master thread (root node only)

        mplr::prequest fSendToCM;   ///< Send request to the cluster master
        T fTaskIDRecvFromCM;        ///< Task ID received from the cluster master
        mplr::prequest fRecvFromCM; ///< Receive request from the cluster master

        mplr::prequest_pool fRecvFromW; ///< Pending receives from workers
        std::vector<T> fTaskIDSendToW;  ///< Task ID sent to each worker
        mplr::prequest_pool fSendToW;   ///< Pending sends to workers
    };

public:
    /// @brief Construct a cluster-aware dispatcher.
    ClusterAwareMasterWorkerDispatcher();

private:
    /// @brief Compute batches, start the node master, and synchronize.
    virtual auto PreLoopAction() -> void override;
    /// @brief Start a batch request when entering a new batch.
    virtual auto PreTaskAction() -> void override;
    /// @brief Request a new batch after finishing a batch, or advance locally.
    virtual auto PostTaskAction() -> void override;
    /// @brief Wait for outstanding requests and join the node master thread.
    virtual auto PostLoopAction() -> void override;

    /// @brief Estimate progress for progress reporting.
    /// @return Pair of a validity flag and an estimated executed-task count
    virtual auto NExecutedTaskEstimation() const -> std::pair<bool, T> override;

private:
    mplr::communicator fIntraNodeComm; ///< Communicator within a shared-memory domain
    mplr::communicator fInterNodeComm; ///< Communicator between node masters

    T fIntraNodeBatchSize;              ///< Tasks per intra-node batch
    T fInterNodeBatchSizeMultiple;      ///< Multiple of intra-node batches per inter-node batch
    std::vector<T> fInterNodeBatchSize; ///< Inter-node batch size for each node

    std::unique_ptr<NodeMaster> fNodeMaster; ///< Node master (node root only)
    std::jthread fNodeMasterThread;          ///< Node master thread (node root only)

    mplr::prequest fSendToNM;   ///< Send request to the node master
    T fTaskIDRecvFromNM;        ///< Task ID received from the node master
    mplr::prequest fRecvFromNM; ///< Receive request from the node master

    T fIntraNodeTaskCounter; ///< Tasks executed in the current intra-node batch

    static constexpr long double fgImbalancingFactor{1e-3};         ///< Batch-size imbalancing factor
    static constexpr long long fgMaxInterNodeBatchSizeMultiple{10}; ///< Maximum inter-node batch multiple
};

} // namespace Mustard::inline Execution

#include "Mustard/Execution/ClusterAwareMasterWorkerDispatcher.inl"
