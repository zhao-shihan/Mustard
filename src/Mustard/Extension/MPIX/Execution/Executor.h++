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
#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Extension/MPIX/Execution/ClusterAwareMasterWorkerScheduler.h++"
#include "Mustard/Extension/MPIX/Execution/MasterWorkerScheduler.h++"
#include "Mustard/Extension/MPIX/Execution/Scheduler.h++"
#include "Mustard/Extension/MPIX/Execution/SequentialScheduler.h++"
#include "Mustard/Extension/MPIX/Execution/SharedMemoryScheduler.h++"
#include "Mustard/Extension/MPIX/Execution/StaticScheduler.h++"
#include "Mustard/Utility/PrettyLog.h++"
#include "Mustard/Utility/Print.h++"

#include "mpl/mpl.hpp"

#include "muc/math"
#include "muc/numeric"
#include "muc/time"

#include "envparse/parse.h++"

#include "gsl/gsl"

#include "fmt/chrono.h"
#include "fmt/format.h"
#include "fmt/std.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <concepts>
#include <cstdio>
#include <functional>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <thread>
#include <tuple>
#include <utility>

namespace Mustard::inline Extension::MPIX::inline Execution {

template<std::integral T>
    requires(Concept::MPIPredefined<T> and sizeof(T) >= sizeof(short))
class Executor final {
public:
    using Index = T;

public:
    Executor();
    Executor(std::string executionName, std::string taskName);
    Executor(std::unique_ptr<Scheduler<T>> scheduler);
    Executor(std::string executionName, std::string taskName, std::unique_ptr<Scheduler<T>> scheduler);

    auto SwitchScheduler(std::unique_ptr<Scheduler<T>> scheduler) -> void;
    auto ResetScheduler() -> void { SwitchScheduler(DefaultScheduler()); }

    auto PrintProgress(bool a) -> void { fPrintProgress = a; }
    auto PrintProgressModulo(long long mod) -> void { fPrintProgressModulo = mod; }
    auto ExecutionName(std::string name) -> void { fExecutionName = std::move(name); }
    auto TaskName(std::string name) -> void { fTaskName = std::move(name); }

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

    static auto DefaultScheduler() -> std::unique_ptr<Scheduler<T>>;
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

    scsc::time_point fExecutionBeginSystemTime;
    muc::wall_time_stopwatch<> fWallTimeStopwatch;
    muc::cpu_time_stopwatch<> fCPUTimeStopwatch;
    double fExecutionWallTime;
    double fExecutionCPUTime;

    std::vector<std::tuple<T, double, double>> fExecutionInfoGatheredByMaster;
};

} // namespace Mustard::inline Extension::MPIX::inline Execution

#include "Mustard/Extension/MPIX/Execution/Executor.inl"
