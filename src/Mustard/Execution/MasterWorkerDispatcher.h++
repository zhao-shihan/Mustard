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

#include "Mustard/Execution/Dispatcher.h++"
#include "Mustard/IO/PrettyLog.h++"

#include "mplr/mplr.hpp"

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace Mustard::inline Execution {

/// @class MasterWorkerDispatcher
/// @brief Dynamic master-worker dispatcher.
///
/// Rank 0 runs a master thread that hands out batches of consecutive task
/// indices to workers on request. Workers execute a batch before requesting the
/// next one, which provides dynamic load balancing with relatively low
/// communication overhead.
///
/// @note Recommended for small to moderate MPI worlds, including multi-node
///       worlds where shared-memory windows are unavailable or not worthwhile.
///       DefaultDispatcherCode() selects it for worlds with at most 128 processes
///       or with every node having at most 8 processes.
/// @tparam T Task index type
/// @see Executor
template<std::integral T>
class MasterWorkerDispatcher : public Dispatcher<T> {
private:
    friend class Master;
    /// @brief Master-side request handler running on rank 0.
    class Master {
    public:
        /// @brief Construct the master for a dispatcher.
        /// @param s Owning dispatcher
        Master(MasterWorkerDispatcher<T>* s);

        /// @brief Start all receive requests.
        auto StartAll() -> void;
        /// @brief Serve worker requests until all batches have been handed out.
        auto operator()() -> void;

    private:
        MasterWorkerDispatcher<T>* fS; ///< Owning dispatcher
        mplr::prequest_pool fRecv;     ///< Pending receives from workers
        std::vector<T> fTaskIDSend;    ///< Task ID sent to each worker
        mplr::prequest_pool fSend;     ///< Pending sends to workers
    };

public:
    /// @brief Construct a master-worker dispatcher.
    MasterWorkerDispatcher();

    /// @brief Initialize batching, start the master thread, and synchronize.
    virtual auto PreLoopAction() -> void override;
    /// @brief Start a batch request when entering a new batch.
    virtual auto PreTaskAction() -> void override;
    /// @brief Request a new batch after finishing a batch, or advance locally.
    virtual auto PostTaskAction() -> void override;
    /// @brief Wait for outstanding requests and join the master thread.
    virtual auto PostLoopAction() -> void override;

    /// @brief Estimate progress for progress reporting.
    /// @return Pair of a validity flag and an estimated executed-task count
    virtual auto NExecutedTaskEstimation() const -> std::pair<bool, T> override;

private:
    mplr::communicator fComm;        ///< Communicator used by the master and workers
    T fBatchSize;                    ///< Number of tasks per batch
    std::unique_ptr<Master> fMaster; ///< Master handler (rank 0 only)
    std::jthread fMasterThread;      ///< Master thread (rank 0 only)

    mplr::prequest fSend; ///< Send request for the next batch
    T fTaskIDRecv;        ///< Task ID received from the master
    mplr::prequest fRecv; ///< Receive request for the next batch
    T fTaskCounter;       ///< Tasks executed in the current batch

    static constexpr long double fgImbalancingFactor{1e-3}; ///< Batch-size imbalancing factor
};

} // namespace Mustard::inline Execution

#include "Mustard/Execution/MasterWorkerDispatcher.inl"
