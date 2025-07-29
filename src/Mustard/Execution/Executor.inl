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

namespace Mustard::inline Execution {

template<std::integral T>
    requires(Parallel::MPIPredefined<T> and sizeof(T) >= sizeof(short))
Executor<T>::Executor(std::string_view scheduler) :
    Executor{DecodeScheduler(scheduler)} {}

template<std::integral T>
    requires(Parallel::MPIPredefined<T> and sizeof(T) >= sizeof(short))
Executor<T>::Executor(std::string executionName, std::string taskName, std::string_view scheduler) :
    Executor{std::move(executionName), std::move(taskName), DecodeScheduler(scheduler)} {}

template<std::integral T>
    requires(Parallel::MPIPredefined<T> and sizeof(T) >= sizeof(short))
Executor<T>::Executor(std::unique_ptr<Scheduler<T>> scheduler) :
    Executor{"Execution", "Task", std::move(scheduler)} {}

template<std::integral T>
    requires(Parallel::MPIPredefined<T> and sizeof(T) >= sizeof(short))
Executor<T>::Executor(std::string executionName, std::string taskName, std::unique_ptr<Scheduler<T>> scheduler) :
    fScheduler{std::move(scheduler)},
    fExecuting{},
    fPrintProgress{true},
    fPrintProgressInterval{3},
    fExecutionName{std::move(executionName)},
    fTaskName{std::move(taskName)},
    fExecutionBeginTime{},
    fStopwatch{},
    fProcessorStopwatch{},
    fExecutionInfoGatheredByMaster{},
    fExecutionInfoReducedByMaster{} {}

template<std::integral T>
    requires(Parallel::MPIPredefined<T> and sizeof(T) >= sizeof(short))
auto Executor<T>::SwitchScheduler(std::unique_ptr<Scheduler<T>> scheduler) -> void {
    if (fExecuting) {
        Throw<std::logic_error>("Try switching scheduler during executing");
    }
    auto task{std::move(fScheduler->fTask)};
    fScheduler = std::move(scheduler);
    fScheduler->fTask = std::move(task);
}

template<std::integral T>
    requires(Parallel::MPIPredefined<T> and sizeof(T) >= sizeof(short))
auto Executor<T>::Execute(typename Scheduler<T>::Task task, std::invocable<T> auto&& F) -> T {
    // reset
    if (task.last < task.first) {
        Throw<std::invalid_argument>(fmt::format("task.last ({}) < task.first ({})", task.last, task.first));
    }
    if (task.last == task.first) {
        return 0;
    }
    const auto worldComm{mplr::comm_world()};
    const auto nTask{task.last - task.first};
    if (nTask < static_cast<T>(worldComm.size())) {
        Throw<std::runtime_error>(fmt::format("Number of tasks ({}) < number of processes ({})", nTask, worldComm.size()));
    }
    fScheduler->fTask = task;
    fScheduler->Reset();
    Expects(ExecutingTask() == Task().first);
    Expects(NLocalExecutedTask() == 0);
    Expects(fScheduler->NExecutedTaskEstimation().second == 0);
    // initialize
    fExecuting = true;
    fScheduler->PreLoopAction();
    worldComm.ibarrier().wait(mplr::duty_ratio::preset::moderate);
    fExecutionBeginTime = std::chrono::system_clock::now();
    fStopwatch.reset();
    fProcessorStopwatch.reset();
    PreLoopReport();
    // main loop
    while (ExecutingTask() != Task().last) {
        fScheduler->PreTaskAction();
        const auto taskID{ExecutingTask()};
        Expects(taskID <= Task().last);
        std::invoke(std::forward<decltype(F)>(F), taskID);
        ++fScheduler->fNLocalExecutedTask;
        fScheduler->PostTaskAction();
        PostTaskReport(taskID);
    }
    // finalize
    fExecuting = false;
    const std::tuple executionInfo{NLocalExecutedTask(), fStopwatch.read().count(), fProcessorStopwatch.read().count()};
    std::vector<std::tuple<T, StopwatchDuration::rep, StopwatchDuration::rep>> executionInfoGatheredByMaster(
        worldComm.rank() == 0 ? worldComm.size() : 0);
    auto gatherExecutionInfo{worldComm.igather(0, executionInfo, executionInfoGatheredByMaster.data())};
    fScheduler->PostLoopAction();
    gatherExecutionInfo.wait(mplr::duty_ratio::preset::relaxed);
    if (worldComm.rank() == 0) {
        fExecutionInfoGatheredByMaster.resize(worldComm.size());
        std::ranges::transform(executionInfoGatheredByMaster, fExecutionInfoGatheredByMaster.begin(),
                               [](auto&& t) {
                                   return ExecutionInfo{.nExecutedTask = get<0>(t),
                                                        .time = StopwatchDuration{get<1>(t)},
                                                        .processorTime = StopwatchDuration{get<2>(t)}};
                               });
        auto& [totalExecutedTask, maxTime, totalProcessorTime]{fExecutionInfoReducedByMaster};
        totalExecutedTask = muc::ranges::transform_reduce(
            fExecutionInfoGatheredByMaster, T{}, std::plus{}, [](auto&& a) { return a.nExecutedTask; });
        maxTime = std::ranges::max_element(fExecutionInfoGatheredByMaster, std::less{}, [](auto&& a) { return a.time; })->time;
        totalProcessorTime = muc::ranges::transform_reduce(
            fExecutionInfoGatheredByMaster, StopwatchDuration::zero(), std::plus{}, [](auto&& a) { return a.processorTime; });
    }
    worldComm.ibarrier().wait(mplr::duty_ratio::preset::relaxed);
    PostLoopReport();
    return NLocalExecutedTask();
}

template<std::integral T>
    requires(Parallel::MPIPredefined<T> and sizeof(T) >= sizeof(short))
auto Executor<T>::PrintExecutionSummary() const -> void {
    const auto worldComm{mplr::comm_world()};
    if (worldComm.rank() != 0) {
        return;
    }
    if (fExecutionInfoGatheredByMaster.empty() or fExecuting) {
        PrintWarning("Execution summary not available for now");
        return;
    }
    Print("+------------------+--------------> Summary <-------------+-------------------+\n"
          "| Rank in world    | Executed          | Wall time (s)    | Processor t. (s)  |\n"
          "+------------------+-------------------+------------------+-------------------+\n");
    Expects(ssize(fExecutionInfoGatheredByMaster) == worldComm.size());
    using Seconds = muc::chrono::seconds<double>;
    for (int rank{}; rank < worldComm.size(); ++rank) {
        const auto& [executed, time, processorTime]{fExecutionInfoGatheredByMaster[rank]};
        PrintLn("| {:16} | {:17} | {:16.3f} | {:17.3f} |",
                rank, executed, Seconds{time}.count(), Seconds{processorTime}.count());
    }
    const auto& [nTotalExecutedTask, maxTime, totalProcessorTime]{fExecutionInfoReducedByMaster};
    if (worldComm.size() > 1) {
        PrintLn("+------------------+-------------------+------------------+-------------------+\n"
                "| Total or max     | {:17} | {:16.3f} | {:17.3f} |",
                nTotalExecutedTask, Seconds{maxTime}.count(), Seconds{totalProcessorTime}.count());
    }
    PrintLn("+------------------+--------------> Summary <-------------+-------------------+");
}

template<std::integral T>
    requires(Parallel::MPIPredefined<T> and sizeof(T) >= sizeof(short))
auto Executor<T>::PreLoopReport() const -> void {
    if (not fPrintProgress) {
        return;
    }
    const auto worldComm{mplr::comm_world()};
    if (worldComm.rank() != 0) {
        return;
    }
    Print("+----------------------------------> Start <----------------------------------+\n"
          "| {:75} |\n"
          "+----------------------------------> Start <----------------------------------+\n",
          fmt::format("[{}] {} has started on {} process{}",
                      FormatToLocalTime(fExecutionBeginTime), fExecutionName, worldComm.size(), worldComm.size() > 1 ? "es" : ""));
}

template<std::integral T>
    requires(Parallel::MPIPredefined<T> and sizeof(T) >= sizeof(short))
auto Executor<T>::PostTaskReport(T iEnded) const -> void {
    if (not fPrintProgress) {
        return;
    }
    const auto [goodEstimation, nExecutedTask]{fScheduler->NExecutedTaskEstimation()};
    const auto elapsed{fStopwatch.read()};
    const auto speed{static_cast<double>(nExecutedTask) / elapsed.count()};
    if ((iEnded + 1) % std::max(1ll, std::llround(speed * fPrintProgressInterval.count())) != 0) {
        return;
    }
    const auto worldComm{mplr::comm_world()};
    const auto perSecondSpeed{muc::chrono::seconds<double>{1} / StopwatchDuration{1} * speed};
    const auto now{std::chrono::system_clock::now()};
    Print("MPI{}> [{}] {} {} has ended\n"
          "MPI{}>   {} elaps., {}\n",
          worldComm.rank(), FormatToLocalTime(now), fTaskName, iEnded,
          worldComm.rank(), ToDayHrMinSecMs(elapsed),
          [&, good{goodEstimation}, nExecuted{nExecutedTask}] {
              if (good) {
                  const StopwatchDuration eta{std::llround((NTask() - nExecuted) / speed)};
                  const auto progress{100. * nExecuted / NTask()};
                  return fmt::format("est. rem. {} ({:.3}/s), prog.: {} | {}/{} | {:.3}%",
                                     ToDayHrMinSecMs(eta), perSecondSpeed, NLocalExecutedTask(), nExecuted, NTask(), progress);
              } else {
                  return fmt::format("local prog.: {}", NLocalExecutedTask());
              }
          }());
}

template<std::integral T>
    requires(Parallel::MPIPredefined<T> and sizeof(T) >= sizeof(short))
auto Executor<T>::PostLoopReport() const -> void {
    if (not fPrintProgress) {
        return;
    }
    const auto worldComm{mplr::comm_world()};
    if (worldComm.rank() != 0) {
        return;
    }
    const auto& [_, maxTime, totalProcessorTime]{fExecutionInfoReducedByMaster};
    using Seconds = muc::chrono::seconds<double>;
    using std::chrono_literals::operator""s;
    const auto now{std::chrono::system_clock::now()};
    Print("+-----------------------------------> End <-----------------------------------+\n"
          "| {:75} |\n"
          "| {:75} |\n"
          "| {:75} |\n"
          "| {:75} |\n"
          "+-----------------------------------> End <-----------------------------------+\n",
          fmt::format("[{}] {} has ended on {} process{}", FormatToLocalTime(now), fExecutionName, worldComm.size(), worldComm.size() > 1 ? "es" : ""),
          fmt::format("      Start time: {}", FormatToLocalTime(fExecutionBeginTime)),
          fmt::format("       Wall time: {:.3f} seconds ({})", Seconds{maxTime}.count(), ToDayHrMinSecMs(maxTime)),
          fmt::format("  Processor time: {:.3f} seconds ({})", Seconds{totalProcessorTime}.count(), ToDayHrMinSecMs(totalProcessorTime)));
}

template<std::integral T>
    requires(Parallel::MPIPredefined<T> and sizeof(T) >= sizeof(short))
auto Executor<T>::DecodeScheduler(std::string_view scheduler) -> std::unique_ptr<Scheduler<T>> {
    static const std::map<std::string_view, std::function<auto()->std::unique_ptr<Scheduler<T>>>> schedulerMap{
        {"clmw", [] { return std::make_unique<ClusterAwareMasterWorkerScheduler<T>>(); }},
        {"mw",   [] { return std::make_unique<MasterWorkerScheduler<T>>(); }            },
        {"seq",  [] { return std::make_unique<SequentialScheduler<T>>(); }              },
        {"shm",  [] { return std::make_unique<SharedMemoryScheduler<T>>(); }            },
        {"stat", [] { return std::make_unique<StaticScheduler<T>>(); }                  }
    };
    try {
        return schedulerMap.at(scheduler)();
    } catch (const std::out_of_range&) {
        std::vector<std::string_view> available(schedulerMap.size());
        std::ranges::transform(schedulerMap, available.begin(), [](auto&& s) { return s.first; });
        Throw<std::out_of_range>(fmt::format("Scheduler '{}' not found, available are {}", scheduler, available));
    }
}

template<std::integral T>
    requires(Parallel::MPIPredefined<T> and sizeof(T) >= sizeof(short))
auto Executor<T>::DefaultSchedulerCode() -> std::string {
    if (const auto envScheduler{envparse::parse<envparse::not_set_option::left_blank>("${MUSTARD_EXECUTION_SCHEDULER}")};
        not envScheduler.empty()) {
        return envScheduler;
    }
    if (not mplr::available()) {
        return "seq";
    }
    const auto worldComm{mplr::comm_world()};
    if (worldComm.size() == 1) {
        return "seq";
    }
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    if (mpiEnv.ClusterSize() == 1) {
        return "shm";
    }
    if (worldComm.size() <= 128) {
        return "mw";
    }
    return "clmw";
}

template<std::integral T>
    requires(Parallel::MPIPredefined<T> and sizeof(T) >= sizeof(short))
auto Executor<T>::ToDayHrMinSecMs(StopwatchDuration duration) -> std::string {
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

} // namespace Mustard::inline Execution
