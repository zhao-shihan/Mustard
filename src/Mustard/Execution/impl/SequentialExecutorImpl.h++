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
#include "Mustard/Execution/impl/ExecutorImplBase.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Utility/ProgressBar.h++"

#include "muc/chrono"

#include "gsl/gsl"

#include <chrono>
#include <concepts>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace Mustard::inline Execution::impl {

/// @class SequentialExecutorImpl
/// @brief Sequential executor implementation.
///
/// Runs every task on the calling process and reports a single-process execution summary.
///
/// @tparam T Task index type
template<std::integral T>
class SequentialExecutorImpl final : public ExecutorImplBase<T> {
public:
    /// @brief Construct a sequential executor implementation.
    /// @param executionName Name shown in the start/end report
    /// @param opName Operation name used in progress messages
    /// @param taskName Task name used in progress messages
    /// @param dispatcher Dispatcher object (must not be null)
    SequentialExecutorImpl(std::string executionName, std::string opName, std::string taskName, std::unique_ptr<Dispatcher<T>> dispatcher);

    /// @brief Return the number of processes participating in execution.
    /// @return 1
    auto NProcess() const -> int { return 1; }

    /// @brief Run a callable over a task range.
    /// @param task Half-open task range
    /// @param F Callable invoked as `F(taskIndex)` for each assigned task
    /// @return Number of tasks executed by this process
    auto Run(struct Dispatcher<T>::Task task, std::invocable<T> auto&& F) -> T;
    /// @brief Print the execution summary.
    auto PrintExecutionSummary() const -> void;

private:
    ProgressBar fProgressBar; ///< Progress bar used during a run
};

} // namespace Mustard::inline Execution::impl

#include "Mustard/Execution/impl/SequentialExecutorImpl.inl"
