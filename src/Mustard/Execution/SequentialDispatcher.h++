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

#include "mplr/mplr.hpp"

#include <concepts>
#include <utility>

namespace Mustard::inline Execution {

/// @class SequentialDispatcher
/// @brief Dispatcher that executes all tasks sequentially on a single process.
/// @note Construction throws if the MPI world contains more than one process.
/// @note Recommended for single-process runs, small task counts, and debugging.
///       DefaultDispatcherCode() selects it when MPI is unavailable or the world
///       contains one process.
/// @tparam T Task index type
/// @see Executor
template<std::integral T>
class SequentialDispatcher : public Dispatcher<T> {
public:
    /// @brief Construct a sequential dispatcher.
    /// @throws std::runtime_error if MPI is available with more than one process
    SequentialDispatcher();

    /// @brief Set the next task index to the first task.
    virtual auto PreLoopAction() -> void override { this->fExecutingTask = this->fTask.first; }
    /// @brief Do nothing.
    virtual auto PreTaskAction() -> void override {}
    /// @brief Advance the next task index by one.
    virtual auto PostTaskAction() -> void override { this->fExecutingTask += 1; }
    /// @brief Do nothing.
    virtual auto PostLoopAction() -> void override {}

    /// @brief Estimate progress for progress reporting.
    /// @return Pair of a validity flag and an estimated executed-task count
    virtual auto NExecutedTaskEstimation() const -> std::pair<bool, T> override;
};

} // namespace Mustard::inline Execution

#include "Mustard/Execution/SequentialDispatcher.inl"
