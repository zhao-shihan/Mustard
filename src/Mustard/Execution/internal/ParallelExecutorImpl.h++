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

#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Execution/Scheduler.h++"
#include "Mustard/Execution/internal/ExecutorImplBase.h++"
#include "Mustard/Utility/FormatToLocalTime.h++"
#include "Mustard/Utility/PrettyLog.h++"
#include "Mustard/Utility/Print.h++"

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

namespace Mustard::inline Execution::internal {

template<std::integral T>
class ParallelExecutorImpl final : public ExecutorImplBase<T> {
public:
    ParallelExecutorImpl(std::string executionName, std::string taskName, std::unique_ptr<Scheduler<T>> scheduler);

    auto NProcess() const -> int { return mplr::comm_world().size(); }

    auto Execute(struct Scheduler<T>::Task task, std::invocable<T> auto&& F) -> T;
    auto PrintExecutionSummary() const -> void;

private:
    auto PostTaskReport(T iEnded) const -> void;

private:
    using StopwatchDuration = typename ExecutorImplBase<T>::StopwatchDuration;
    using ExecutionInfo = typename ExecutorImplBase<T>::ExecutionInfo;

private:
    std::vector<ExecutionInfo> fExecutionInfoList;
};

} // namespace Mustard::inline Execution::internal

#include "Mustard/Execution/internal/ParallelExecutorImpl.inl"
