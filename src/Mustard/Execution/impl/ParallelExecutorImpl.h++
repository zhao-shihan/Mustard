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
#include "Mustard/Execution/impl/ExecutorImplBase.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/IO/Print.h++"
#include "Mustard/Utility/FormatToLocalTime.h++"

#include "mplr/mplr.hpp"

#include "muc/chrono"
#include "muc/math"
#include "muc/numeric"

#include "gsl/gsl"

#include "fmt/chrono.h"
#include "fmt/format.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <concepts>
#include <functional>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace Mustard::inline Execution::impl {

/// @class ParallelExecutorImpl
/// @brief Parallel executor implementation over the MPI world communicator.
///
/// Runs tasks on all processes, gathers per-rank statistics, and requires at least one task per process.
///
/// @tparam T Task index type
template<std::integral T>
class ParallelExecutorImpl final : public ExecutorImplBase<T> {
public:
    /// @brief Construct a parallel executor implementation.
    /// @param executionName Name shown in the start/end report
    /// @param opName Operation name used in progress messages
    /// @param taskName Task name used in progress messages
    /// @param dispatcher Dispatcher object (must not be null)
    ParallelExecutorImpl(std::string executionName, std::string opName, std::string taskName, std::unique_ptr<Dispatcher<T>> dispatcher);

    /// @brief Return the number of processes participating in execution.
    /// @return MPI world size
    auto NProcess() const -> int { return mplr::comm_world().size(); }

    /// @brief Run a callable over a task range.
    /// @param task Half-open task range
    /// @param F Callable invoked as `F(taskIndex)` for each assigned task
    /// @return Number of tasks executed by this process
    /// @throws std::runtime_error if the range contains fewer tasks than processes
    auto Run(struct Dispatcher<T>::Task task, std::invocable<T> auto&& F) -> T;
    /// @brief Print the per-rank execution summary.
    auto PrintExecutionSummary() const -> void;

private:
    /// @brief Print progress after task @p iEnded when the progress interval has elapsed.
    /// @param iEnded Index of the task just completed
    auto PostTaskReport(T iEnded) const -> void;

private:
    /// @brief Duration type used by the execution stopwatches.
    using typename ExecutorImplBase<T>::StopwatchDuration;
    /// @brief Per-run execution statistics type.
    using typename ExecutorImplBase<T>::ExecutionInfoType;

private:
    std::vector<ExecutionInfoType> fExecutionInfoList; ///< Per-rank execution statistics
};

} // namespace Mustard::inline Execution::impl

#include "Mustard/Execution/impl/ParallelExecutorImpl.inl"
