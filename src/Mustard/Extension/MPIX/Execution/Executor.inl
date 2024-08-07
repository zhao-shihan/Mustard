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
    fScheduler{std::make_unique_for_overwrite<S<T>>()},
    fExecuting{},
    fPrintProgress{true},
    fPrintProgressModulo{},
    fExecutionName{"Execution"},
    fTaskName{"Task"},
    fFinalPollingPeriod{20ms},
    fExecutionBeginSystemTime{},
    fWallTimeStopwatch{},
    fCPUTimeStopwatch{},
    fExecutionWallTime{},
    fExecutionCPUTime{},
    fNLocalExecutedTaskOfAllProcessKeptByMaster{},
    fExecutionWallTimeOfAllProcessKeptByMaster{},
    fExecutionCPUTimeOfAllProcessKeptByMaster{} {
    if (const auto& mpiEnv{Env::MPIEnv::Instance()};
        mpiEnv.OnCommWorldMaster()) {
        fNLocalExecutedTaskOfAllProcessKeptByMaster.resize(mpiEnv.CommWorldSize());
        fExecutionWallTimeOfAllProcessKeptByMaster.resize(mpiEnv.CommWorldSize());
        fExecutionCPUTimeOfAllProcessKeptByMaster.resize(mpiEnv.CommWorldSize());
    }
}

template<std::integral T>
    requires(Concept::MPIPredefined<T> and sizeof(T) >= sizeof(short))
template<template<typename> typename AScheduler>
    requires std::derived_from<AScheduler<T>, Scheduler<T>>
auto Executor<T>::SwitchScheduler() -> void {
    if (fExecuting) { throw std::logic_error{"switch scheduler kernel during processing"}; }
    auto task{std::move(fScheduler->fTask)};
    fScheduler = std::make_unique_for_overwrite<AScheduler<T>>();
    fScheduler->fTask = std::move(task);
}

template<std::integral T>
    requires(Concept::MPIPredefined<T> and sizeof(T) >= sizeof(short))
auto Executor<T>::Execute(typename Scheduler<T>::Task task, std::invocable<T> auto&& F) -> T {
    // reset
    if (task.last < task.first) { throw std::invalid_argument{"task.last < task.first"}; }
    if (task.last == task.first) { return 0; }
    if (task.last - task.first < static_cast<T>(Env::MPIEnv::Instance().CommWorldSize())) {
        throw std::runtime_error{"number of tasks < size of MPI_COMM_WORLD"};
    }
    fScheduler->fTask = task;
    fScheduler->Reset();
    assert(ExecutingTask() == Task().first);
    assert(NLocalExecutedTask() == 0);
    assert(fScheduler->NExecutedTask().second == 0);
    // initialize
    fExecuting = true;
    fScheduler->PreLoopAction();
    MPI_Barrier(MPI_COMM_WORLD);
    fExecutionBeginSystemTime = scsc::now();
    fWallTimeStopwatch = {};
    fCPUTimeStopwatch = {};
    PreLoopReport();
    // main loop
    while (ExecutingTask() != Task().last) {
        fScheduler->PreTaskAction();
        const auto taskID{ExecutingTask()};
        assert(taskID <= Task().last);
        std::invoke(std::forward<decltype(F)>(F), taskID);
        ++fScheduler->fNLocalExecutedTask;
        fScheduler->PostTaskAction();
        PostTaskReport(taskID);
    }
    // finalize
    fExecutionWallTime = fWallTimeStopwatch.s_elapsed();
    fExecutionCPUTime = fCPUTimeStopwatch.s_used();
    MPI_Request barrierRequest;
    MPI_Ibarrier(MPI_COMM_WORLD, &barrierRequest);
    struct GatheringDataType {
        T nLocalExecutedTask;
        double wallTime;
        double cpuTime;
    };
    MPI_Datatype gatheringDataType;
    MPI_Type_create_struct(3,                                                                       // count
                           std::array<int, 3>{1,                                                    // array_of_block_lengths
                                              1,                                                    // array_of_block_lengths
                                              1}                                                    // array_of_block_lengths
                               .data(),                                                             // array_of_block_lengths
                           std::array<MPI_Aint, 3>{offsetof(GatheringDataType, nLocalExecutedTask), // array_of_displacements
                                                   offsetof(GatheringDataType, wallTime),           // array_of_displacements
                                                   offsetof(GatheringDataType, cpuTime)}            // array_of_displacements
                               .data(),                                                             // array_of_displacements
                           std::array<MPI_Datatype, 3>{DataType<T>(),                               // array_of_types
                                                       MPI_DOUBLE,                                  // array_of_types
                                                       MPI_DOUBLE}                                  // array_of_types
                               .data(),                                                             // array_of_types
                           &gatheringDataType);                                                     // newtype
    GatheringDataType gatheringData{fScheduler->fNLocalExecutedTask, fExecutionWallTime, fExecutionCPUTime};
    std::vector<GatheringDataType> masterGatheredData;
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    if (mpiEnv.OnCommWorldMaster()) {
        masterGatheredData.resize(mpiEnv.CommWorldSize());
    }
    MPI_Request gatherRequest;
    MPI_Type_commit(&gatheringDataType);
    MPI_Igather(&gatheringData,            // sendbuf
                1,                         // sendcount
                gatheringDataType,         // sendtype
                masterGatheredData.data(), // recvbuf
                1,                         // recvcount
                gatheringDataType,         // recvtype
                0,                         // root
                MPI_COMM_WORLD,            // comm
                &gatherRequest);           // request
    MPI_Type_free(&gatheringDataType);
    fScheduler->PostLoopAction();
    fExecuting = false;
    while (true) {
        int reached;
        MPI_Test(&barrierRequest, &reached, MPI_STATUS_IGNORE);
        if (reached) { break; }
        std::this_thread::sleep_for(fFinalPollingPeriod);
    }
    MPI_Wait(&gatherRequest, MPI_STATUS_IGNORE);
    if (mpiEnv.OnCommWorldMaster()) {
        for (int rank{}; rank < mpiEnv.CommWorldSize(); ++rank) {
            fNLocalExecutedTaskOfAllProcessKeptByMaster[rank] = masterGatheredData[rank].nLocalExecutedTask;
            fExecutionWallTimeOfAllProcessKeptByMaster[rank] = masterGatheredData[rank].wallTime;
            fExecutionCPUTimeOfAllProcessKeptByMaster[rank] = masterGatheredData[rank].cpuTime;
        }
    }
    PostLoopReport();
    return NLocalExecutedTask();
}

template<std::integral T>
    requires(Concept::MPIPredefined<T> and sizeof(T) >= sizeof(short))
auto Executor<T>::PrintExecutionSummary() const -> void {
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    if (not mpiEnv.OnCommWorldMaster()) { return; }
    if (fExecuting) {
        Env::PrintLnWarning("Execution summary not available for now.");
        return;
    }
    Env::Print("+------------------+--------------> Summary <-------------+-------------------+\n"
               "| Rank in world    | Executed          | Wall time (s)    | CPU time (s)      |\n"
               "+------------------+-------------------+------------------+-------------------+\n");
    for (int rank{}; rank < mpiEnv.CommWorldSize(); ++rank) {
        const auto& executed{fNLocalExecutedTaskOfAllProcessKeptByMaster[rank]};
        const auto& wallTime{fExecutionWallTimeOfAllProcessKeptByMaster[rank]};
        const auto& cpuTime{fExecutionCPUTimeOfAllProcessKeptByMaster[rank]};
        Env::PrintLn("| {:16} | {:17} | {:16.3f} | {:17.3f} |", rank, executed, wallTime, cpuTime);
    }
    Env::PrintLn("+------------------+--------------> Summary <-------------+-------------------+");
}

template<std::integral T>
    requires(Concept::MPIPredefined<T> and sizeof(T) >= sizeof(short))
auto Executor<T>::PreLoopReport() const -> void {
    if (not fPrintProgress) { return; }
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    if (not mpiEnv.OnCommWorldMaster()) { return; }
    Env::Print("+----------------------------------> Start <----------------------------------+\n"
               "| {:75} |\n"
               "+----------------------------------> Start <----------------------------------+\n",
               fmt::format("{:%FT%T%z} > {} has started on {} process{}",
                           fmt::localtime(scsc::to_time_t(fExecutionBeginSystemTime)), fExecutionName, mpiEnv.CommWorldSize(), mpiEnv.Parallel() ? "es" : ""));
}

template<std::integral T>
    requires(Concept::MPIPredefined<T> and sizeof(T) >= sizeof(short))
auto Executor<T>::PostTaskReport(T iEnded) const -> void {
    if (not fPrintProgress or fPrintProgressModulo < 0) { return; }
    const auto [goodForEstmation, nExecutedTask]{fScheduler->NExecutedTask()};
    const auto secondsElapsed{fWallTimeStopwatch.s_elapsed()};
    const auto speed{nExecutedTask / secondsElapsed};
    if (fPrintProgressModulo == 0) {
        // adaptive mode, print every ~3s
        if ((iEnded + 1) % std::max(1ll, std::llround(speed * 3)) != 0) { return; }
    } else {
        // manual mode
        if ((iEnded + 1) % fPrintProgressModulo != 0) { return; }
    }
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    Env::Print("MPI{}> {:%FT%T%z} > {} {} has ended\n"
               "MPI{}>   {} elaps., {}\n",
               mpiEnv.CommWorldRank(), fmt::localtime(scsc::to_time_t(scsc::now())), fTaskName, iEnded,
               mpiEnv.CommWorldRank(), SToDHMS(secondsElapsed),
               [&, goodForEstmation{goodForEstmation}, nExecutedTask{nExecutedTask}] {
                   if (goodForEstmation) {
                       const auto eta{(NTask() - nExecutedTask) / speed};
                       const auto progress{static_cast<double>(nExecutedTask) / NTask()};
                       return fmt::format("est. rem. {} ({:.3}/s), prog.: {} | {}/{} | {:.3}%",
                                          SToDHMS(eta), speed, NLocalExecutedTask(), nExecutedTask, NTask(), 100 * progress);
                   } else {
                       return fmt::format("local prog.: {}", NLocalExecutedTask());
                   }
               }());
}

template<std::integral T>
    requires(Concept::MPIPredefined<T> and sizeof(T) >= sizeof(short))
auto Executor<T>::PostLoopReport() const -> void {
    if (not fPrintProgress) { return; }
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    if (not mpiEnv.OnCommWorldMaster()) { return; }
    const auto now{scsc::now()};
    const auto maxWallTime{*std::ranges::max_element(fExecutionWallTimeOfAllProcessKeptByMaster)};
    const auto totalCpuTime{muc::ranges::reduce(fExecutionCPUTimeOfAllProcessKeptByMaster)};
    Env::Print("+-----------------------------------> End <-----------------------------------+\n"
               "| {:75} |\n"
               "| {:75} |\n"
               "| {:75} |\n"
               "| {:75} |\n"
               "+-----------------------------------> End <-----------------------------------+\n",
               fmt::format("{:%FT%T%z} > {} has ended on {} process{}", fmt::localtime(scsc::to_time_t(now)), fExecutionName, mpiEnv.CommWorldSize(), mpiEnv.Parallel() ? "es" : ""),
               fmt::format("  Start time: {:%FT%T%z}", fmt::localtime(scsc::to_time_t(fExecutionBeginSystemTime))),
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
