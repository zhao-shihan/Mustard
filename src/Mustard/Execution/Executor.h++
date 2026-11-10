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
#include "Mustard/Execution/impl/ExecutorImplBase.h++"
#include "Mustard/Execution/impl/ParallelExecutorImpl.h++"
#include "Mustard/Execution/impl/SequentialExecutorImpl.h++"

#include "mplr/mplr.hpp"

#include "gsl/gsl"

#include <concepts>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace Mustard::inline Execution {

/// @class Executor
/// @brief Execute a callable over a range of task indices, possibly across MPI processes.
///
/// An executor owns a Dispatcher that decides how task indices are assigned to
/// processes. The implementation is parallel when MPI is available with more
/// than one process, and sequential otherwise. Run invokes the callable with
/// each task index and returns when all assigned tasks have completed.
///
/// In the parallel case, Run is collective: every process in the world
/// communicator must call it with the same task range.
///
/// @tparam T Task index type; it must be an MPI-predefined integral type for parallel execution
/// @see Dispatcher
/// @see Dispatcher::Task
template<std::integral T>
class Executor final {
public:
    /// @brief Task index type.
    using Index = T;
    /// @brief Per-run execution statistics.
    using ExecutionInfoType = typename impl::ExecutorImplBase<T>::ExecutionInfoType;

public:
    /// @brief Construct an executor with the default names and dispatcher.
    /// @param dispatcher Dispatcher code (see MakeCodedDispatcher())
    /// @note The default names are `"Run"`, `"Execution"`, and `"task"`.
    /// @throws std::out_of_range if @p dispatcher is not recognized
    Executor(std::string_view dispatcher = DefaultDispatcherCode());
    /// @brief Construct an executor with explicit names and dispatcher code.
    /// @param executionName Name shown in the start/end report
    /// @param opName Operation name used in progress messages
    /// @param taskName Task name used in progress messages
    /// @param dispatcher Dispatcher code (see MakeCodedDispatcher())
    /// @throws std::out_of_range if @p dispatcher is not recognized
    Executor(std::string executionName, std::string opName, std::string taskName, std::string_view dispatcher = DefaultDispatcherCode());
    /// @brief Construct an executor with the default names and a dispatcher object.
    /// @param dispatcher Dispatcher object (must not be null)
    /// @note The default names are `"Run"`, `"Execution"`, and `"task"`.
    /// @throws std::invalid_argument if @p dispatcher is null
    Executor(std::unique_ptr<Dispatcher<T>> dispatcher);
    /// @brief Construct an executor with explicit names and a dispatcher object.
    /// @param executionName Name shown in the start/end report
    /// @param opName Operation name used in progress messages
    /// @param taskName Task name used in progress messages
    /// @param dispatcher Dispatcher object (must not be null)
    /// @throws std::invalid_argument if @p dispatcher is null
    Executor(std::string executionName, std::string opName, std::string taskName, std::unique_ptr<Dispatcher<T>> dispatcher);

    /// @brief Replace the dispatcher by its code, preserving the current task range.
    /// @param dispatcher Dispatcher code
    /// @warning The dispatcher must be compatible with the executor's sequential or parallel implementation; compatibility is not checked.
    /// @throws std::logic_error if called while Run is executing
    /// @throws std::out_of_range if @p dispatcher is not recognized
    auto SwitchDispatcher(std::string_view dispatcher) -> void;
    /// @brief Replace the dispatcher, preserving the current task range.
    /// @param dispatcher Dispatcher object
    /// @warning The dispatcher must be compatible with the executor's sequential or parallel implementation; compatibility is not checked.
    /// @throws std::logic_error if called while Run is executing
    auto SwitchDispatcher(std::unique_ptr<Dispatcher<T>> dispatcher) -> void;

    /// @brief Return the number of processes participating in execution.
    /// @return Process count (1 for sequential execution)
    auto NProcess() const -> int;

    /// @brief Return the current task range.
    /// @return Current half-open task range
    auto Task() const -> struct Dispatcher<T>::Task;
    /// @brief Return the number of tasks in the current range.
    /// @return Number of tasks
    auto NTask() const -> T;
    /// @brief Return the next task index on this process.
    /// @return Next local task index
    auto ExecutingTask() const -> T;
    /// @brief Return the number of tasks executed locally in the current run.
    /// @return Local executed-task count
    auto NLocalExecutedTask() const -> T;

    /// @brief Return whether Run is currently executing.
    /// @return true while a run is in progress
    auto Executing() const -> bool;

    /// @brief Return whether progress output is enabled.
    /// @return true if progress output is enabled
    auto PrintProgress() const -> bool;
    /// @brief Enable or disable progress output.
    /// @param print true to enable progress output
    auto PrintProgress(bool print) -> void;
    /// @brief Return the minimum interval between progress messages.
    /// @return Progress interval
    auto PrintProgressInterval() const -> muc::chrono::seconds<double>;
    /// @brief Set the minimum interval between progress messages.
    /// @param t Progress interval; negative values are treated as zero
    auto PrintProgressInterval(muc::chrono::seconds<double> t) -> void;

    /// @brief Return the execution name.
    /// @return Execution name
    auto ExecutionName() const -> const std::string&;
    /// @brief Set the execution name shown in the start/end report.
    /// @param name New execution name
    auto ExecutionName(std::string name) -> void;
    /// @brief Return the operation name.
    /// @return Operation name
    auto OperationName() const -> const std::string&;
    /// @brief Set the operation name used in progress messages.
    /// @param name New operation name
    auto OperationName(std::string name) -> void;
    /// @brief Return the task name.
    /// @return Task name
    auto TaskName() const -> const std::string&;
    /// @brief Set the task name used in progress messages.
    /// @param name New task name
    auto TaskName(std::string name) -> void;

    /// @brief Run a callable over a task range.
    /// @param task Half-open task range @f$[\mathrm{first},\mathrm{last})@f$
    /// @param F Callable invoked as `F(taskIndex)` for each assigned task
    /// @return Number of tasks executed by this process
    /// @throws std::invalid_argument if `task.last < task.first`
    /// @throws std::runtime_error if parallel execution has fewer tasks than processes
    /// @note This function is collective in the parallel case; all processes must call it with the same range.
    auto Run(struct Dispatcher<T>::Task task, std::invocable<T> auto&& F) -> T;
    /// @brief Run a callable over tasks @f$[0,\mathrm{size})@f$.
    /// @param size Number of tasks
    /// @param F Callable invoked as `F(taskIndex)` for each assigned task
    /// @return Number of tasks executed by this process
    /// @throws std::invalid_argument if `size < 0`
    /// @throws std::runtime_error if parallel execution has fewer tasks than processes
    auto Run(T size, std::invocable<T> auto&& F) -> T;
    /// @brief Run with temporary execution, operation, and task names.
    /// @param executionName Temporary execution name
    /// @param operationName Temporary operation name
    /// @param taskName Temporary task name
    /// @param args Arguments forwarded to an existing Run overload
    /// @return Return value of the forwarded Run overload
    /// @note The previous names are restored after the call, including when it throws.
    /// @see Run
    auto Run(std::string executionName, std::string operationName, std::string taskName, auto&&... args) -> T;

    /// @brief Return the statistics of the most recent run.
    /// @return Execution statistics (aggregated totals and maxima in parallel mode)
    auto ExecutionInfo() const -> const ExecutionInfoType&;
    /// @brief Print a per-rank execution summary of the most recent run.
    auto PrintExecutionSummary() const -> void;

private:
    /// @brief Visit the active implementation with a const visitor.
    /// @param visitor Callable accepting the implementation
    /// @return Result of the visitor
    template<typename F>
    auto VisitImpl(F&& visitor) const -> decltype(auto);
    /// @brief Visit the active implementation with a mutable visitor.
    /// @param visitor Callable accepting the implementation
    /// @return Result of the visitor
    template<typename F>
    auto VisitImpl(F&& visitor) -> decltype(auto);

private:
    /// @brief Active implementation variant.
    using Impl = std::variant<impl::ParallelExecutorImpl<T>,
                              impl::SequentialExecutorImpl<T>>;

private:
    std::unique_ptr<Impl> fImpl; ///< Active sequential or parallel implementation
};

} // namespace Mustard::inline Execution

#include "Mustard/Execution/Executor.inl"
