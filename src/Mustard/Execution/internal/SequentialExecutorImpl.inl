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
SequentialExecutorImpl<T>::SequentialExecutorImpl(std::string executionName, std::string taskName, std::unique_ptr<Scheduler<T>> scheduler) :
    ExecutorImplBase<T>{std::move(executionName), std::move(taskName), std::move(scheduler)},
    fProgressBar{} {
    using std::chrono_literals::operator""ms;
    this->fPrintProgressInterval = 33ms;
}

template<std::integral T>
auto SequentialExecutorImpl<T>::Execute(struct Scheduler<T>::Task task, std::invocable<T> auto&& F) -> T {
    // reset
    if (task.last < task.first) {
        Throw<std::invalid_argument>(fmt::format("task.last ({}) < task.first ({})", task.last, task.first));
    }
    if (task.last == task.first) {
        return 0;
    }
    const auto nTask{task.last - task.first};
    this->fScheduler->Task(task);
    this->fScheduler->Reset();
    Expects(this->ExecutingTask() == this->Task().first);
    Expects(this->NLocalExecutedTask() == 0);
    Expects(this->fScheduler->NExecutedTaskEstimation().second == 0);
    // initialize
    this->fExecuting = true;
    this->fScheduler->PreLoopAction();
    this->fExecutionBeginTime = std::chrono::system_clock::now();
    this->fStopwatch.reset();
    this->fProcessorStopwatch.reset();
    fProgressBar.Start(nTask);
    this->PreLoopReport();
    // main loop
    while (this->ExecutingTask() != this->Task().last) {
        this->fScheduler->PreTaskAction();
        const auto taskID{this->ExecutingTask()};
        Ensures(taskID <= this->Task().last);
        std::invoke(std::forward<decltype(F)>(F), taskID);
        this->fScheduler->IncrementNLocalExecutedTask();
        this->fScheduler->PostTaskAction();
        fProgressBar.Tick();
    }
    // finalize
    this->fExecuting = false;
    this->fExecutionInfo.nExecutedTask = this->NLocalExecutedTask();
    this->fExecutionInfo.time = this->fStopwatch.read();
    this->fExecutionInfo.processorTime = this->fProcessorStopwatch.read();
    this->fScheduler->PostLoopAction();
    fProgressBar.Complete();
    this->PostLoopReport();
    return this->NLocalExecutedTask();
}

template<std::integral T>
auto SequentialExecutorImpl<T>::PrintExecutionSummary() const -> void {
    const auto& [executed, time, processorTime]{this->fExecutionInfo};
    if (executed == 0 or this->fExecuting) {
        PrintWarning("Execution summary not available for now");
        return;
    }
    using Seconds = muc::chrono::seconds<double>;
    Print("+-------------------------+-------> Summary <-------+-------------------------+\n"
          "| Executed                | Wall time (s)           | Processor time (s)      |\n"
          "+-------------------------+-------------------------+-------------------------+\n"
          "| {:25} | {:25.3f} | {:25.3f} |\n"
          "+-------------------------+-------> Summary <-------+-------------------------+\n",
          executed, Seconds{time}.count(), Seconds{processorTime}.count());
}

} // namespace Mustard::inline Execution::internal
