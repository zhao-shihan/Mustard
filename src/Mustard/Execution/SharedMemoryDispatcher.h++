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
#include "Mustard/Parallel/MPIDataType.h++"

#include "mpi.h"

#include "gsl/gsl"

#include <algorithm>
#include <cmath>
#include <concepts>
#include <stdexcept>
#include <string>
#include <utility>

namespace Mustard::inline Execution {

/// @class SharedMemoryDispatcher
/// @brief Dynamic dispatcher using an MPI shared-memory window.
///
/// Processes in one shared-memory domain reserve batches of tasks by atomically
/// incrementing a counter in a shared MPI window. This gives dynamic load
/// balancing with very low communication overhead, but requires the whole MPI
/// world to be a single shared-memory domain.
///
/// @note Recommended for a single shared-memory domain (typically one node) with
///       many tasks of variable cost, where low-latency dynamic balancing matters.
///       DefaultDispatcherCode() selects it for a single-cluster world with more
///       than one process.
/// @tparam T Task index type
/// @see Executor
template<std::integral T>
class SharedMemoryDispatcher : public Dispatcher<T> {
public:
    /// @brief Construct a shared-memory dispatcher and allocate its task counter.
    /// @throws std::runtime_error if the MPI world spans multiple shared-memory domains
    SharedMemoryDispatcher();
    /// @brief Free the shared-memory window.
    ~SharedMemoryDispatcher();

    /// @brief Initialize the shared task counter and per-process batch state.
    virtual auto PreLoopAction() -> void override;
    /// @brief Do nothing.
    virtual auto PreTaskAction() -> void override {}
    /// @brief Reserve the next batch after finishing the current one, or advance locally.
    virtual auto PostTaskAction() -> void override;
    /// @brief Do nothing.
    virtual auto PostLoopAction() -> void override {}

    /// @brief Estimate progress for progress reporting.
    /// @return Pair of a validity flag and an estimated executed-task count
    virtual auto NExecutedTaskEstimation() const -> std::pair<bool, T> override;

private:
    volatile T* fMainTaskID;   ///< Shared next-task counter, allocated on the window root
    MPI_Win fMainTaskIDWindow; ///< Window exposing the shared counter
    T fBatchSize;              ///< Number of tasks reserved per batch
    T fTaskCounter;            ///< Tasks executed in the current batch

    static constexpr long double fgImbalancingFactor{1e-4}; ///< Batch-size imbalancing factor
};

} // namespace Mustard::inline Execution

#include "Mustard/Execution/SharedMemoryDispatcher.inl"
