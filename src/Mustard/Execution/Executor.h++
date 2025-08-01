// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
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

#include "Mustard/Execution/DefaultScheduler.h++"
#include "Mustard/Execution/Scheduler.h++"
#include "Mustard/Execution/internal/ParallelExecutorImpl.h++"
#include "Mustard/Execution/internal/SequentialExecutorImpl.h++"

#include "mplr/mplr.hpp"

#include <concepts>
#include <memory>
#include <string>
#include <string_view>
#include <variant>

namespace Mustard::inline Execution {

template<std::integral T>
class Executor final {
public:
    using Index = T;

public:
    Executor(std::string_view scheduler = DefaultSchedulerCode());
    Executor(std::string executionName, std::string taskName, std::string_view scheduler = DefaultSchedulerCode());
    Executor(std::unique_ptr<Scheduler<T>> scheduler);
    Executor(std::string executionName, std::string taskName, std::unique_ptr<Scheduler<T>> scheduler);

    auto SwitchScheduler(std::string_view scheduler) -> void;
    auto SwitchScheduler(std::unique_ptr<Scheduler<T>> scheduler) -> void;

    auto Task() const -> struct Scheduler<T>::Task;
    auto NTask() const -> T;
    auto ExecutingTask() const -> T;
    auto NLocalExecutedTask() const -> T;

    auto Executing() const -> bool;

    auto PrintProgress(bool print) -> void;
    auto PrintProgressInterval(muc::chrono::seconds<double> t) -> void;
    auto ExecutionName(std::string name) -> void;
    auto TaskName(std::string name) -> void;

    auto Execute(struct Scheduler<T>::Task task, std::invocable<T> auto&& F) -> T;
    auto Execute(T size, std::invocable<T> auto&& F) -> T;

    auto PrintExecutionSummary() const -> void;

private:
    using Impl = std::variant<internal::ParallelExecutorImpl<T>,
                              internal::SequentialExecutorImpl<T>>;

private:
    std::unique_ptr<Impl> fImpl;
};

} // namespace Mustard::inline Execution

#include "Mustard/Execution/Executor.inl"
