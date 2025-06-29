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

namespace Mustard::inline Extension::MPIX::inline Execution {

using std::chrono_literals::operator""ms;

template<std::integral T>
    requires(Concept::MPIPredefined<T> and sizeof(T) >= sizeof(short))
template<template<typename> typename S>
    requires std::derived_from<S<T>, Scheduler<T>>
Executor<T>::Executor(ScheduleBy<S>) :
    Executor{"Execution", "Task", ScheduleBy<S>{}} {}

template<std::integral T>
    requires(Concept::MPIPredefined<T> and sizeof(T) >= sizeof(short))
template<template<typename> typename S>
    requires std::derived_from<S<T>, Scheduler<T>>
Executor<T>::Executor(std::string executionName, std::string taskName, ScheduleBy<S>) :
    fScheduler{std::make_unique_for_overwrite<S<T>>()},
    fExecuting{},
    fPrintProgress{true},
    fPrintProgressModulo{},
    fExecutionName{std::move(executionName)},
    fTaskName{std::move(taskName)},
    fExecutionBeginSystemTime{},
    fWallTimeStopwatch{},
    fCPUTimeStopwatch{},
    fExecutionWallTime{},
    fExecutionCPUTime{},
    fExecutionInfoGatheredByMaster{} {}

template<std::integral T>
    requires(Concept::MPIPredefined<T> and sizeof(T) >= sizeof(short))
template<template<typename> typename AScheduler>
    requires std::derived_from<AScheduler<T>, Scheduler<T>>
auto Executor<T>::SwitchScheduler() -> void {
    if (fExecuting) { Throw<std::logic_error>("Try switching scheduler during executing"); }
    auto task{std::move(fScheduler->fTask)};
    fScheduler = std::make_unique_for_overwrite<AScheduler<T>>();
    fScheduler->fTask = std::move(task);
}

template<std::integral T>
    requires(Concept::MPIPredefined<T> and sizeof(T) >= sizeof(short))
auto Executor<T>::Execute(typename Scheduler<T>::Task task, std::invocable<T> auto&& F) -> T {
    // reset
    if (task.last < task.first) {
        Throw<std::invalid_argument>(fmt::format("task.last ({}) < task.first ({})", task.last, task.first));
    }
    if (task.last == task.first) {
        return 0;
    }
    const auto& worldComm{mpl::environment::comm_world()};
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
    worldComm.ibarrier().wait(mpl::duty_ratio::preset::moderate);
    fExecutionBeginSystemTime = scsc::now();
    fWallTimeStopwatch.reset();
    fCPUTimeStopwatch.reset();
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
    fExecutionWallTime = fWallTimeStopwatch.s_elapsed();
    fExecutionCPUTime = fCPUTimeStopwatch.s_used();
    if (worldComm.rank() == 0) {
        fExecutionInfoGatheredByMaster.resize(worldComm.size());
    }
    const std::tuple executionInfo{fScheduler->fNLocalExecutedTask, fExecutionWallTime, fExecutionCPUTime};
    auto gatherExecutionInfo{worldComm.igather(0, executionInfo, fExecutionInfoGatheredByMaster.data())};
    fScheduler->PostLoopAction();
    fExecuting = false;
    gatherExecutionInfo.wait(mpl::duty_ratio::preset::relaxed);
    worldComm.ibarrier().wait(mpl::duty_ratio::preset::relaxed);
    PostLoopReport();
    return NLocalExecutedTask();
}

template<std::integral T>
    requires(Concept::MPIPredefined<T> and sizeof(T) >= sizeof(short))
auto Executor<T>::PrintExecutionSummary() const -> void {
    const auto& worldComm{mpl::environment::comm_world()};
    if (worldComm.rank() != 0) { return; }
    if (fExecutionInfoGatheredByMaster.empty() or fExecuting) {
        PrintWarning("Execution summary not available for now");
        return;
    }
    Print("+------------------+--------------> Summary <-------------+-------------------+\n"
          "| Rank in world    | Executed          | Wall time (s)    | CPU time (s)      |\n"
          "+------------------+-------------------+------------------+-------------------+\n");
    Expects(ssize(fExecutionInfoGatheredByMaster) == worldComm.size());
    for (int rank{}; rank < worldComm.size(); ++rank) {
        const auto& [executed, wallTime, cpuTime]{fExecutionInfoGatheredByMaster[rank]};
        PrintLn("| {:16} | {:17} | {:16.3f} | {:17.3f} |", rank, executed, wallTime, cpuTime);
    }
    PrintLn("+------------------+--------------> Summary <-------------+-------------------+");
}

template<std::integral T>
    requires(Concept::MPIPredefined<T> and sizeof(T) >= sizeof(short))
auto Executor<T>::PreLoopReport() const -> void {
    if (not fPrintProgress) { return; }
    const auto& worldComm{mpl::environment::comm_world()};
    if (worldComm.rank() != 0) { return; }
    Print("+----------------------------------> Start <----------------------------------+\n"
          "| {:75} |\n"
          "+----------------------------------> Start <----------------------------------+\n",
          fmt::format("[{:%FT%T%z}] {} has started on {} process{}",
                      muc::localtime(scsc::to_time_t(fExecutionBeginSystemTime)), fExecutionName, worldComm.size(), worldComm.size() > 1 ? "es" : ""));
}

template<std::integral T>
    requires(Concept::MPIPredefined<T> and sizeof(T) >= sizeof(short))
auto Executor<T>::PostTaskReport(T iEnded) const -> void {
    if (not fPrintProgress or fPrintProgressModulo < 0) { return; }
    const auto [goodEstimation, nExecutedTask]{fScheduler->NExecutedTaskEstimation()};
    const auto secondsElapsed{fWallTimeStopwatch.s_elapsed()};
    const auto speed{nExecutedTask / secondsElapsed};
    if (fPrintProgressModulo == 0) {
        // adaptive mode, print every ~3s
        if ((iEnded + 1) % std::max(1ll, std::llround(speed * 3)) != 0) { return; }
    } else {
        // manual mode
        if ((iEnded + 1) % fPrintProgressModulo != 0) { return; }
    }
    const auto& worldComm{mpl::environment::comm_world()};
    Print("MPI{}> [{:%FT%T%z}] {} {} has ended\n"
          "MPI{}>   {} elaps., {}\n",
          worldComm.rank(), muc::localtime(scsc::to_time_t(scsc::now())), fTaskName, iEnded,
          worldComm.rank(), SToDHMS(secondsElapsed),
          [&, good{goodEstimation}, nExecuted{nExecutedTask}] {
              if (good) {
                  const auto eta{(NTask() - nExecuted) / speed};
                  const auto progress{100. * nExecuted / NTask()};
                  return fmt::format("est. rem. {} ({:.3}/s), prog.: {} | {}/{} | {:.3}%",
                                     SToDHMS(eta), speed, NLocalExecutedTask(), nExecuted, NTask(), progress);
              } else {
                  return fmt::format("local prog.: {}", NLocalExecutedTask());
              }
          }());
}

template<std::integral T>
    requires(Concept::MPIPredefined<T> and sizeof(T) >= sizeof(short))
auto Executor<T>::PostLoopReport() const -> void {
    if (not fPrintProgress) { return; }
    const auto& worldComm{mpl::environment::comm_world()};
    if (worldComm.rank() != 0) { return; }
    const auto now{scsc::now()};
    const auto maxWallTime{get<1>(*std::ranges::max_element(fExecutionInfoGatheredByMaster, std::less{},
                                                            [](auto&& a) { return std::get<1>(a); }))};
    const auto totalCpuTime{muc::ranges::transform_reduce(fExecutionInfoGatheredByMaster, 0., std::plus{},
                                                          [](auto&& a) { return std::get<2>(a); })};
    Print("+-----------------------------------> End <-----------------------------------+\n"
          "| {:75} |\n"
          "| {:75} |\n"
          "| {:75} |\n"
          "| {:75} |\n"
          "+-----------------------------------> End <-----------------------------------+\n",
          fmt::format("[{:%FT%T%z}] {} has ended on {} process{}", muc::localtime(scsc::to_time_t(now)), fExecutionName, worldComm.size(), worldComm.size() > 1 ? "es" : ""),
          fmt::format("  Start time: {:%FT%T%z}", muc::localtime(scsc::to_time_t(fExecutionBeginSystemTime))),
          fmt::format("   Wall time: {:.3f} seconds{}", maxWallTime, maxWallTime <= 60 ? "" : " (" + SToDHMS(maxWallTime) + ')'),
          fmt::format("    CPU time: {:.3f} seconds{}", totalCpuTime, totalCpuTime <= 60 ? "" : " (" + SToDHMS(totalCpuTime) + ')'));
}

template<std::integral T>
    requires(Concept::MPIPredefined<T> and sizeof(T) >= sizeof(short))
auto Executor<T>::SToDHMS(double s) -> std::string {
    const auto totalSeconds{std::llround(s)};
    const auto div86400{muc::div(totalSeconds, 86400ll)};
    const auto div3600{muc::div(div86400.rem, 3600ll)};
    const auto div60{muc::div(div3600.rem, 60ll)};
    const auto& [day, hour, minute, second]{std::tie(div86400.quot, div3600.quot, div60.quot, div60.rem)};
    if (day > 0) { return fmt::format("{}d {}h {}m", day, hour, minute); }
    if (hour > 0) { return fmt::format("{}h {}m", hour, minute); }
    if (minute > 0) { return fmt::format("{}m {}s", minute, second); }
    return fmt::format("{}s", second);
}

} // namespace Mustard::inline Extension::MPIX::inline Execution
