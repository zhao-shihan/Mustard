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

#include "Mustard/Parallel/MPIPredefined.h++"
#include "Mustard/Utility/ClassSemanticHelper.h++"

#include <concepts>
#include <utility>

namespace Mustard::inline Execution {

/// @class Dispatcher
/// @brief Abstract task scheduler used by Executor.
///
/// A dispatcher owns a half-open task range @f$[\mathrm{first},\mathrm{last})@f$
/// and determines which task indices each process executes and in which order.
/// Executor drives the task loop and calls the dispatcher's pre/post-loop and
/// pre/post-task hooks.
///
/// @tparam T Task index type, an MPI-predefined integral type
/// @see Executor
template<std::integral T>
    requires(Parallel::MPIPredefined<T> and sizeof(T) >= sizeof(short))
class Dispatcher : public NoncopyableBase {
public:
    /// @brief Half-open task range @f$[\mathrm{first},\mathrm{last})@f$.
    struct Task {
        T first; ///< First task index (inclusive)
        T last;  ///< Last task index (exclusive)
    };

public:
    /// @brief Destroy the dispatcher.
    virtual ~Dispatcher() = default;

    /// @brief Return the current task range.
    /// @return Current half-open task range
    auto Task() const -> auto { return fTask; }
    /// @brief Return the number of tasks in the current range.
    /// @return @f$\mathrm{last}-\mathrm{first}@f$
    auto NTask() const -> auto { return fTask.last - fTask.first; }
    /// @brief Return the index of the next task to execute on this process.
    /// @return Next local task index
    auto ExecutingTask() const -> auto { return fExecutingTask; }
    /// @brief Return the number of tasks executed locally in the current run.
    /// @return Local executed-task count
    auto NLocalExecutedTask() const -> auto { return fNLocalExecutedTask; }

    /// @brief Set the task range.
    /// @param task New half-open task range
    auto Task(struct Task task) -> void { fTask = task; }
    /// @brief Reset the local execution cursor and executed-task counter.
    auto Reset() -> void;
    /// @brief Increment the local executed-task counter.
    auto IncrementNLocalExecutedTask() -> void { ++fNLocalExecutedTask; }

    /// @brief Hook called once before the task loop.
    virtual auto PreLoopAction() -> void = 0;
    /// @brief Hook called before each task.
    virtual auto PreTaskAction() -> void = 0;
    /// @brief Hook called after each task.
    virtual auto PostTaskAction() -> void = 0;
    /// @brief Hook called once after the task loop.
    virtual auto PostLoopAction() -> void = 0;

    /// @brief Estimate progress for progress reporting.
    /// @return Pair of a validity flag and an estimated executed-task count
    virtual auto NExecutedTaskEstimation() const -> std::pair<bool, T> = 0;

protected:
    struct Task fTask;     ///< Current task range
    T fExecutingTask;      ///< Next task index on this process
    T fNLocalExecutedTask; ///< Number of tasks executed locally
};

} // namespace Mustard::inline Execution

#include "Mustard/Execution/Dispatcher.inl"
