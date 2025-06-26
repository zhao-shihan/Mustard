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

#include "Mustard/Concept/MPIPredefined.h++"
#include "Mustard/Extension/MPIX/Execution/MasterWorkerScheduler.h++"
#include "Mustard/Extension/MPIX/Execution/Scheduler.h++"
#include "Mustard/Extension/MPIX/LazySpinWait.h++"
#include "Mustard/Utility/PrettyLog.h++"
#include "Mustard/Utility/Print.h++"

#include "mpl/mpl.hpp"

#include "muc/math"
#include "muc/numeric"
#include "muc/time"

#include "gsl/gsl"

#include "fmt/chrono.h"
#include "fmt/format.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <concepts>
#include <cstdio>
#include <functional>
#include <memory>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <thread>
#include <tuple>
#include <utility>

namespace Mustard::inline Extension::MPIX::inline Execution {

template<template<typename> typename>
struct ScheduleBy {};

template<std::integral T>
    requires(Concept::MPIPredefined<T> and sizeof(T) >= sizeof(short))
class Executor final {
public:
    using Index = T;

public:
    template<template<typename> typename S = MasterWorkerScheduler>
        requires std::derived_from<S<T>, Scheduler<T>>
    Executor(ScheduleBy<S> = {});
    template<template<typename> typename S = MasterWorkerScheduler>
        requires std::derived_from<S<T>, Scheduler<T>>
    Executor(std::string executionName, std::string taskName, ScheduleBy<S> = {});

    template<template<typename> typename AScheduler>
        requires std::derived_from<AScheduler<T>, Scheduler<T>>
    auto SwitchScheduler() -> void;

    auto PrintProgress(bool a) -> void { fPrintProgress = a; }
    auto PrintProgressModulo(long long mod) -> void { fPrintProgressModulo = mod; }
    auto ExecutionName(std::string name) -> void { fExecutionName = std::move(name); }
    auto TaskName(std::string name) -> void { fTaskName = std::move(name); }
    auto FinalPollInterval(std::chrono::milliseconds t) -> void { fFinalPollInterval = std::move(t); }

    auto Task() const -> auto { return fScheduler->fTask; }
    auto NTask() const -> T { return fScheduler->NTask(); }
    auto Executing() const -> bool { return fExecuting; }

    auto Execute(typename Scheduler<T>::Task task, std::invocable<T> auto&& F) -> T;
    auto Execute(T size, std::invocable<T> auto&& F) -> T { return Execute({0, size}, std::forward<decltype(F)>(F)); }

    auto ExecutingTask() const -> T { return fScheduler->fExecutingTask; }
    auto NLocalExecutedTask() const -> T { return fScheduler->fNLocalExecutedTask; }

    auto PrintExecutionSummary() const -> void;

private:
    auto PreLoopReport() const -> void;
    auto PostTaskReport(T iEnded) const -> void;
    auto PostLoopReport() const -> void;

    static auto SToDHMS(double s) -> std::string;

private:
    using scsc = std::chrono::system_clock;

private:
    std::unique_ptr<Scheduler<T>> fScheduler;

    bool fExecuting;

    bool fPrintProgress;
    long long fPrintProgressModulo;

    std::string fExecutionName;
    std::string fTaskName;

    std::chrono::milliseconds fFinalPollInterval;

    scsc::time_point fExecutionBeginSystemTime;
    muc::wall_time_stopwatch<> fWallTimeStopwatch;
    muc::cpu_time_stopwatch<> fCPUTimeStopwatch;
    double fExecutionWallTime;
    double fExecutionCPUTime;

    std::vector<std::tuple<T, double, double>> fExecutionInfoGatheredByMaster;
};

} // namespace Mustard::inline Extension::MPIX::inline Execution

#include "Mustard/Extension/MPIX/Execution/Executor.inl"
