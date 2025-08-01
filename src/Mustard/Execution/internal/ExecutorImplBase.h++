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
#include "Mustard/Parallel/MPIPredefined.h++"
#include "Mustard/Utility/FormatToLocalTime.h++"
#include "Mustard/Utility/PrettyLog.h++"
#include "Mustard/Utility/Print.h++"

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

namespace Mustard::inline Execution::internal {

template<std::integral T>
    requires(Parallel::MPIPredefined<T> and sizeof(T) >= sizeof(short))
class ExecutorImplBase {
public:
    ExecutorImplBase(std::string executionName, std::string taskName, std::unique_ptr<Scheduler<T>> scheduler);

    auto SwitchScheduler(std::string_view scheduler) -> void { SwitchScheduler(MakeCodedScheduler<T>(scheduler)); }
    auto SwitchScheduler(std::unique_ptr<Scheduler<T>> scheduler) -> void;

    auto Task() const -> auto { return fScheduler->Task(); }
    auto NTask() const -> auto { return fScheduler->NTask(); }
    auto ExecutingTask() const -> auto { return fScheduler->ExecutingTask(); }
    auto NLocalExecutedTask() const -> auto { return fScheduler->NLocalExecutedTask(); }

    auto Executing() const -> bool { return fExecuting; }

    auto PrintProgress(bool a) -> void { fPrintProgress = a; }
    auto PrintProgressInterval(muc::chrono::seconds<double> t) -> void { fPrintProgressInterval = std::max({}, t); }
    auto ExecutionName(std::string name) -> void { fExecutionName = std::move(name); }
    auto TaskName(std::string name) -> void { fTaskName = std::move(name); }

protected:
    auto PreLoopReport() const -> void;
    auto PostLoopReport() const -> void;

protected:
    using StopwatchDuration = muc::chrono::stopwatch::duration;

protected:
    static auto ToDayHrMinSecMs(StopwatchDuration s) -> std::string;

protected:
    struct ExecutionInfo {
        T nExecutedTask;
        StopwatchDuration time;
        StopwatchDuration processorTime;
    };

protected:
    std::unique_ptr<Scheduler<T>> fScheduler;

    bool fExecuting;

    bool fPrintProgress;
    muc::chrono::seconds<double> fPrintProgressInterval;

    std::string fExecutionName;
    std::string fTaskName;

    std::chrono::system_clock::time_point fExecutionBeginTime;
    muc::chrono::stopwatch fStopwatch;
    muc::chrono::processor_stopwatch fProcessorStopwatch;

    ExecutionInfo fExecutionInfo;
};

} // namespace Mustard::inline Execution::internal

#include "Mustard/Execution/internal/ExecutorImplBase.inl"
