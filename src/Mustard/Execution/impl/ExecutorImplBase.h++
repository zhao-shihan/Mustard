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

#include "Mustard/Execution/DefaultDispatcher.h++"
#include "Mustard/Execution/Dispatcher.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/IO/Print.h++"
#include "Mustard/Parallel/MPIPredefined.h++"
#include "Mustard/Utility/FormatToLocalTime.h++"

#include "mplr/mplr.hpp"

#include "muc/chrono"

#include "gsl/gsl"

#include "fmt/chrono.h"
#include "fmt/format.h"

#include <cmath>
#include <concepts>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace Mustard::inline Execution::impl {

/// @class ExecutorImplBase
/// @brief Common state and behavior of sequential and parallel executor implementations.
///
/// This internal base owns the dispatcher, the user-visible execution/operation/
/// task names, progress settings, and the timing state shared by both executor
/// implementations.
///
/// @tparam T Task index type
/// @see Executor
template<std::integral T>
    requires(Parallel::MPIPredefined<T> and sizeof(T) >= sizeof(short))
class ExecutorImplBase {
protected:
    /// @brief Duration type used by the execution stopwatches.
    using StopwatchDuration = muc::chrono::stopwatch::duration;

public:
    /// @brief Statistics of one completed run.
    struct ExecutionInfoType {
        T nExecutedTask;                 ///< Number of executed tasks (total in parallel mode)
        StopwatchDuration wallTime;      ///< Wall-clock time (maximum in parallel mode)
        StopwatchDuration processorTime; ///< Processor time (total in parallel mode)
    };

public:
    /// @brief Construct an executor implementation.
    /// @param executionName Name shown in the start/end report
    /// @param opName Operation name used in progress messages
    /// @param taskName Task name used in progress messages
    /// @param dispatcher Dispatcher object (must not be null)
    /// @throws std::invalid_argument if @p dispatcher is null
    ExecutorImplBase(std::string executionName, std::string opName, std::string taskName, std::unique_ptr<Dispatcher<T>> dispatcher);

    /// @brief Replace the dispatcher by its code, preserving the current task range.
    /// @param dispatcher Dispatcher code
    /// @throws std::out_of_range if @p dispatcher is not recognized
    auto SwitchDispatcher(std::string_view dispatcher) -> void { SwitchDispatcher(MakeCodedDispatcher<T>(dispatcher)); }
    /// @brief Replace the dispatcher, preserving the current task range.
    /// @param dispatcher Dispatcher object
    /// @throws std::logic_error if called while a run is executing
    auto SwitchDispatcher(std::unique_ptr<Dispatcher<T>> dispatcher) -> void;

    /// @brief Return the current task range.
    /// @return Current half-open task range
    auto Task() const -> auto { return fDispatcher->Task(); }
    /// @brief Return the number of tasks in the current range.
    /// @return Number of tasks
    auto NTask() const -> auto { return fDispatcher->NTask(); }
    /// @brief Return the next local task index.
    /// @return Next local task index
    auto ExecutingTask() const -> auto { return fDispatcher->ExecutingTask(); }
    /// @brief Return the number of tasks executed locally in the current run.
    /// @return Local executed-task count
    auto NLocalExecutedTask() const -> auto { return fDispatcher->NLocalExecutedTask(); }

    /// @brief Return whether a run is currently executing.
    /// @return true while a run is in progress
    auto Executing() const -> bool { return fExecuting; }

    /// @brief Return whether progress output is enabled.
    /// @return true if progress output is enabled
    auto PrintProgress() const -> auto { return fPrintProgress; }
    /// @brief Enable or disable progress output.
    /// @param a true to enable progress output
    auto PrintProgress(bool a) -> void { fPrintProgress = a; }
    /// @brief Return the minimum interval between progress messages.
    /// @return Progress interval
    auto PrintProgressInterval() const -> auto { return fPrintProgressInterval; }
    /// @brief Set the minimum interval between progress messages.
    /// @param t Progress interval; negative values are treated as zero
    auto PrintProgressInterval(muc::chrono::seconds<double> t) -> void { fPrintProgressInterval = std::max({}, t); }

    /// @brief Return the execution name.
    /// @return Execution name
    auto ExecutionName() const -> const auto& { return fExecutionName; }
    /// @brief Set the execution name.
    /// @param name New execution name
    auto ExecutionName(std::string name) -> void { fExecutionName = std::move(name); }
    /// @brief Return the operation name.
    /// @return Operation name
    auto OperationName() const -> const auto& { return fOperationName; }
    /// @brief Set the operation name.
    /// @param name New operation name
    auto OperationName(std::string name) -> void { fOperationName = std::move(name); }
    /// @brief Return the task name.
    /// @return Task name
    auto TaskName() const -> const auto& { return fTaskName; }
    /// @brief Set the task name.
    /// @param name New task name
    auto TaskName(std::string name) -> void { fTaskName = std::move(name); }

    /// @brief Return the statistics of the most recent run.
    /// @return Execution statistics
    auto ExecutionInfo() const -> const auto& { return fExecutionInfo; }

protected:
    /// @brief Print the start-of-run banner.
    auto PreLoopReport() const -> void;
    /// @brief Print the end-of-run banner.
    auto PostLoopReport() const -> void;

protected:
    /// @brief Format a duration as a compact day/hour/minute/second/millisecond string.
    /// @param s Duration to format
    /// @return Formatted duration
    static auto ToDayHrMinSecMs(StopwatchDuration s) -> std::string;

protected:
    std::unique_ptr<Dispatcher<T>> fDispatcher; ///< Active dispatcher

    bool fExecuting; ///< Whether a run is in progress

    bool fPrintProgress;                                 ///< Whether progress output is enabled
    muc::chrono::seconds<double> fPrintProgressInterval; ///< Minimum progress-message interval

    std::string fExecutionName; ///< Execution name
    std::string fOperationName; ///< Operation name
    std::string fTaskName;      ///< Task name

    std::chrono::system_clock::time_point fExecutionBeginTime; ///< Wall-clock start time
    muc::chrono::stopwatch fStopwatch;                         ///< Wall-clock stopwatch
    muc::chrono::processor_stopwatch fProcessorStopwatch;      ///< Processor-time stopwatch

    ExecutionInfoType fExecutionInfo; ///< Statistics of the most recent run
};

} // namespace Mustard::inline Execution::impl

#include "Mustard/Execution/impl/ExecutorImplBase.inl"
