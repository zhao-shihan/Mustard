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

namespace Mustard::inline Execution {

template<std::integral T>
SharedMemoryScheduler<T>::SharedMemoryScheduler() :
    Scheduler<T>{},
    fMainTaskID{},
    fMainTaskIDWindow{MPI_WIN_NULL},
    fBatchSize{},
    fTaskCounter{} {
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    if (mpiEnv.OnCluster()) {
        Throw<std::runtime_error>("World communicator involves multiple shared memory domains");
    }

    MPI_Info info;
    MPI_Info_create(&info);
    auto _{gsl::finally([&info] { MPI_Info_free(&info); })};
    MPI_Info_set(info, "accumulate_ops", "same_op");
    MPI_Info_set(info, "mpi_accumulate_granularity", std::to_string(sizeof(T)).c_str());
    MPI_Info_set(info, "same_disp_unit", "true");
    const auto& intraNodeComm{mpiEnv.IntraNodeComm()};
    T* mainTaskID;
    MPI_Win_allocate_shared(intraNodeComm.rank() == 0 ? sizeof(T) : 0, sizeof(T), info,
                            intraNodeComm.native_handle(), &mainTaskID, &fMainTaskIDWindow);
    fMainTaskID = mainTaskID;
}

template<std::integral T>
SharedMemoryScheduler<T>::~SharedMemoryScheduler() {
    if (fMainTaskIDWindow != MPI_WIN_NULL) {
        MPI_Win_free(&fMainTaskIDWindow);
    }
}

template<std::integral T>
auto SharedMemoryScheduler<T>::PreLoopAction() -> void {
    const auto& intraNodeComm{Env::MPIEnv::Instance().IntraNodeComm()};
    fBatchSize = std::max(1ll, std::llround(fgImbalancingFactor * this->NTask() / intraNodeComm.size()));
    this->fExecutingTask = this->fTask.first + intraNodeComm.rank() * fBatchSize;
    fTaskCounter = 0;

    if (fMainTaskID) {
        MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0, fMainTaskIDWindow);
        *fMainTaskID = this->fTask.first + intraNodeComm.size() * fBatchSize;
        MPI_Win_unlock(0, fMainTaskIDWindow);
    }
}

template<std::integral T>
auto SharedMemoryScheduler<T>::PostTaskAction() -> void {
    if (++fTaskCounter == fBatchSize) {
        MPI_Win_lock(MPI_LOCK_SHARED, 0, 0, fMainTaskIDWindow);
        MPI_Fetch_and_op(&fBatchSize, &this->fExecutingTask, Parallel::MPIDataType<T>(), 0, 0, MPI_SUM, fMainTaskIDWindow);
        MPI_Win_unlock(0, fMainTaskIDWindow);
        this->fExecutingTask = std::min(this->fExecutingTask, this->fTask.last);
        fTaskCounter = 0;
    } else {
        ++this->fExecutingTask;
    }
}

template<std::integral T>
auto SharedMemoryScheduler<T>::NExecutedTaskEstimation() const -> std::pair<bool, T> {
    return {this->fNLocalExecutedTask > 10 * fBatchSize,
            this->fExecutingTask - this->fTask.first};
}

} // namespace Mustard::inline Execution
