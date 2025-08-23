// -*- C++ -*-
//
// Copyright 2020-2025  The Mustard development team
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

#include "Mustard/Execution/Scheduler.h++"
#include "Mustard/Execution/internal/ExecutorImplBase.h++"
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

namespace Mustard::inline Execution::internal {

template<std::integral T>
class SequentialExecutorImpl final : public ExecutorImplBase<T> {
public:
    SequentialExecutorImpl(std::string executionName, std::string taskName, std::unique_ptr<Scheduler<T>> scheduler);

    auto NProcess() const -> int { return 1; }

    auto operator()(struct Scheduler<T>::Task task, std::invocable<T> auto&& F) -> T;
    auto PrintExecutionSummary() const -> void;

private:
    ProgressBar fProgressBar;
};

} // namespace Mustard::inline Execution::internal

#include "Mustard/Execution/internal/SequentialExecutorImpl.inl"
