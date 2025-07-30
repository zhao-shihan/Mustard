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

namespace Mustard::inline Execution::internal {

template<std::integral T>
    requires(Parallel::MPIPredefined<T> and sizeof(T) >= sizeof(short))
ExecutorImplBase<T>::ExecutorImplBase(std::string executionName, std::string taskName, std::unique_ptr<Scheduler<T>> scheduler) :
    fScheduler{std::move(scheduler)},
    fExecuting{},
    fPrintProgress{true},
    fPrintProgressInterval{},
    fExecutionName{std::move(executionName)},
    fTaskName{std::move(taskName)},
    fExecutionBeginTime{},
    fStopwatch{},
    fProcessorStopwatch{},
    fExecutionInfo{} {}

template<std::integral T>
    requires(Parallel::MPIPredefined<T> and sizeof(T) >= sizeof(short))
auto ExecutorImplBase<T>::SwitchScheduler(std::unique_ptr<Scheduler<T>> scheduler) -> void {
    if (fExecuting) {
        Throw<std::logic_error>("Try switching scheduler during executing");
    }
    const auto task{fScheduler->Task()};
    fScheduler = std::move(scheduler);
    fScheduler->Task(task);
}

template<std::integral T>
    requires(Parallel::MPIPredefined<T> and sizeof(T) >= sizeof(short))
auto ExecutorImplBase<T>::PreLoopReport() const -> void {
    if (not this->fPrintProgress) {
        return;
    }
    const auto worldComm{mplr::available() ? mplr::comm_world() : mplr::comm_null()};
    if (worldComm.is_valid() and worldComm.rank() != 0) {
        return;
    }
    auto startText{fmt::format("[{}] {} has started", FormatToLocalTime(fExecutionBeginTime), this->fExecutionName)};
    if (worldComm.is_valid()) {
        startText += fmt::format(" on {} process{}", worldComm.size(), worldComm.size() > 1 ? "es" : "");
    }
    Print("+----------------------------------> Start <----------------------------------+\n"
          "| {:75} |\n"
          "+----------------------------------> Start <----------------------------------+\n",
          startText);
}

template<std::integral T>
    requires(Parallel::MPIPredefined<T> and sizeof(T) >= sizeof(short))
auto ExecutorImplBase<T>::PostLoopReport() const -> void {
    if (not this->fPrintProgress) {
        return;
    }
    const auto worldComm{mplr::available() ? mplr::comm_world() : mplr::comm_null()};
    if (worldComm.is_valid() and worldComm.rank() != 0) {
        return;
    }
    const auto& [_, maxTime, totalProcessorTime]{fExecutionInfo};
    using Seconds = muc::chrono::seconds<double>;
    using std::chrono_literals::operator""s;
    const auto now{std::chrono::system_clock::now()};
    auto endText{fmt::format("[{}] {} has ended", FormatToLocalTime(now), this->fExecutionName)};
    if (worldComm.is_valid()) {
        endText += fmt::format(" on {} process{}", worldComm.size(), worldComm.size() > 1 ? "es" : "");
    }
    Print("+-----------------------------------> End <-----------------------------------+\n"
          "| {:75} |\n"
          "| {:75} |\n"
          "| {:75} |\n"
          "| {:75} |\n"
          "+-----------------------------------> End <-----------------------------------+\n",
          endText,
          fmt::format("      Start time: {}", FormatToLocalTime(fExecutionBeginTime)),
          fmt::format("       Wall time: {:.3f} seconds ({})", Seconds{maxTime}.count(), ToDayHrMinSecMs(maxTime)),
          fmt::format("  Processor time: {:.3f} seconds ({})", Seconds{totalProcessorTime}.count(), ToDayHrMinSecMs(totalProcessorTime)));
}

template<std::integral T>
    requires(Parallel::MPIPredefined<T> and sizeof(T) >= sizeof(short))
auto ExecutorImplBase<T>::ToDayHrMinSecMs(StopwatchDuration duration) -> std::string {
    Expects(duration.count() >= 0);

    constexpr auto msPerSecond{1000ll};
    constexpr auto msPerMinute{60 * msPerSecond};
    constexpr auto msPerHour{60 * msPerMinute};
    constexpr auto msPerDay{24 * msPerHour};

    const auto msTotal{std::chrono::round<std::chrono::duration<long long, std::milli>>(duration).count()};
    const auto [day, rem1]{std::div(msTotal, msPerDay)};
    const auto [hour, rem2]{std::div(rem1, msPerHour)};
    const auto [minute, rem3]{std::div(rem2, msPerMinute)};
    const auto [second, millisecond]{std::div(rem3, msPerSecond)};

    std::string result;
    do {
        result.reserve(16);
        if (day) {
            result += fmt::format("{}d ", day);
        }
        if (hour) {
            result += fmt::format("{}h ", hour);
        }
        if (minute) {
            result += fmt::format("{}m ", minute);
        }
        if (day) {
            break;
        }
        if (second) {
            result += fmt::format("{}s ", second);
        }
        if (hour) {
            break;
        }
        if (millisecond or result.empty()) {
            result += fmt::format("{}ms ", millisecond);
        }
    } while (false);
    result.pop_back();
    return result;
}

} // namespace Mustard::inline Execution::internal
