// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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
ParallelExecutorImpl<T>::ParallelExecutorImpl(std::string executionName, std::string taskName, std::unique_ptr<Scheduler<T>> scheduler) :
    ExecutorImplBase<T>{std::move(executionName), std::move(taskName), std::move(scheduler)},
    fExecutionInfoList{} {
    using std::chrono_literals::operator""s;
    this->fPrintProgressInterval = 3s;
}

template<std::integral T>
auto ParallelExecutorImpl<T>::operator()(struct Scheduler<T>::Task task, std::invocable<T> auto&& F) -> T {
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
    this->fScheduler->Task(task);
    this->fScheduler->Reset();
    Expects(this->ExecutingTask() == this->Task().first);
    Expects(this->NLocalExecutedTask() == 0);
    Expects(this->fScheduler->NExecutedTaskEstimation().second == 0);
    // initialize
    this->fExecuting = true;
    this->fScheduler->PreLoopAction();
    worldComm.ibarrier().wait(mplr::duty_ratio::preset::moderate);
    this->fExecutionBeginTime = std::chrono::system_clock::now();
    this->fStopwatch.reset();
    this->fProcessorStopwatch.reset();
    this->PreLoopReport();
    // main loop
    while (this->ExecutingTask() != this->Task().last) {
        this->fScheduler->PreTaskAction();
        const auto taskID{this->ExecutingTask()};
        Ensures(taskID <= this->Task().last);
        std::invoke(std::forward<decltype(F)>(F), taskID);
        this->fScheduler->IncrementNLocalExecutedTask();
        this->fScheduler->PostTaskAction();
        PostTaskReport(taskID);
    }
    // finalize
    this->fExecuting = false;
    const std::tuple localExecutionInfo{this->NLocalExecutedTask(), this->fStopwatch.read().count(), this->fProcessorStopwatch.read().count()};
    std::vector<std::tuple<T, typename StopwatchDuration::rep, typename StopwatchDuration::rep>> executionInfoList(
        worldComm.rank() == 0 ? worldComm.size() : 0);
    auto gatherExecutionInfo{worldComm.igather(0, localExecutionInfo, executionInfoList.data())};
    this->fScheduler->PostLoopAction();
    gatherExecutionInfo.wait(mplr::duty_ratio::preset::relaxed);
    if (worldComm.rank() == 0) {
        fExecutionInfoList.resize(worldComm.size());
        std::ranges::transform(executionInfoList, fExecutionInfoList.begin(),
                               [](auto&& t) -> ExecutionInfo {
                                   return {.nExecutedTask = get<0>(t),
                                           .time = StopwatchDuration{get<1>(t)},
                                           .processorTime = StopwatchDuration{get<2>(t)}};
                               });
        auto& [totalExecutedTask, maxTime, totalProcessorTime]{this->fExecutionInfo};
        totalExecutedTask = muc::ranges::transform_reduce(
            fExecutionInfoList, T{}, std::plus{}, [](auto&& a) { return a.nExecutedTask; });
        maxTime = std::ranges::max_element(fExecutionInfoList, std::less{}, [](auto&& a) { return a.time; })->time;
        totalProcessorTime = muc::ranges::transform_reduce(
            fExecutionInfoList, StopwatchDuration::zero(), std::plus{}, [](auto&& a) { return a.processorTime; });
    }
    this->PostLoopReport();
    return this->NLocalExecutedTask();
}

template<std::integral T>
auto ParallelExecutorImpl<T>::PrintExecutionSummary() const -> void {
    const auto worldComm{mplr::comm_world()};
    if (worldComm.rank() != 0) {
        return;
    }
    if (fExecutionInfoList.empty() or this->fExecuting) {
        PrintWarning("Execution summary not available for now");
        return;
    }
    Print("+------------------+--------------> Summary <-------------+-------------------+\n"
          "| Rank in world    | Executed          | Wall time (s)    | Processor t. (s)  |\n"
          "+------------------+-------------------+------------------+-------------------+\n");
    Expects(ssize(fExecutionInfoList) == worldComm.size());
    using Seconds = muc::chrono::seconds<double>;
    for (int rank{}; rank < worldComm.size(); ++rank) {
        const auto& [executed, time, processorTime]{fExecutionInfoList[rank]};
        PrintLn("| {:16} | {:17} | {:16.3f} | {:17.3f} |",
                rank, executed, Seconds{time}.count(), Seconds{processorTime}.count());
    }
    const auto& [nTotalExecutedTask, maxTime, totalProcessorTime]{this->fExecutionInfo};
    if (worldComm.size() > 1) {
        PrintLn("+------------------+-------------------+------------------+-------------------+\n"
                "| Total or max     | {:17} | {:16.3f} | {:17.3f} |",
                nTotalExecutedTask, Seconds{maxTime}.count(), Seconds{totalProcessorTime}.count());
    }
    PrintLn("+------------------+--------------> Summary <-------------+-------------------+");
}

template<std::integral T>
auto ParallelExecutorImpl<T>::PostTaskReport(T iEnded) const -> void {
    if (not this->fPrintProgress) {
        return;
    }
    const auto [goodEstimation, nExecutedTask]{this->fScheduler->NExecutedTaskEstimation()};
    const auto elapsed{this->fStopwatch.read()};
    const auto speed{static_cast<double>(nExecutedTask) / elapsed.count()};
    const std::chrono::duration<double, typename StopwatchDuration::period> printInterval{this->fPrintProgressInterval};
    if ((iEnded + 1) % std::max(1ll, muc::llround(speed * printInterval.count())) != 0) {
        return;
    }
    const auto worldComm{mplr::comm_world()};
    const auto perSecondSpeed{muc::chrono::seconds<double>{1} / StopwatchDuration{1} * speed};
    const auto now{std::chrono::system_clock::now()};
    Print("MPI{}> [{}] {} {} has ended\n"
          "MPI{}>   {} elaps., {}\n",
          worldComm.rank(), FormatToLocalTime(now), this->fTaskName, iEnded,
          worldComm.rank(), this->ToDayHrMinSecMs(elapsed),
          [&, good{goodEstimation}, nExecuted{nExecutedTask}] {
              if (good) {
                  const StopwatchDuration eta{muc::llround((this->NTask() - nExecuted) / speed)};
                  const auto progress{100. * nExecuted / this->NTask()};
                  return fmt::format("est. rem. {} ({:.3}/s), prog.: {} | {}/{} | {:.3}%",
                                     this->ToDayHrMinSecMs(eta), perSecondSpeed, this->NLocalExecutedTask(), nExecuted, this->NTask(), progress);
              } else {
                  return fmt::format("local prog.: {}", this->NLocalExecutedTask());
              }
          }());
}

} // namespace Mustard::inline Execution::internal
