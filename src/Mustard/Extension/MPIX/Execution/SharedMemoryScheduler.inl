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

template<std::integral T>
SharedMemoryScheduler<T>::SharedMemoryScheduler() :
    Scheduler<T>{},
    fMainTaskID{},
    fMainTaskIDWindow{new MPI_Win{MPI_WIN_NULL}, MPIWinDeleter{}},
    fBatchSize{},
    fBatchCounter{} {
    const auto intraNodeComm{Env::MPIEnv::Instance().IntraNodeComm()};
    if (intraNodeComm.size() != mpl::environment::comm_world().size()) {
        Throw<std::runtime_error>("World communicator involves multiple shared memory domains");
    }
    MPI_Info info;
    MPI_Info_create(&info);
    auto _{gsl::finally([&info] { MPI_Info_free(&info); })};
    MPI_Info_set(info, "accumulate_ops", "same_op");
    MPI_Info_set(info, "mpi_accumulate_granularity", std::to_string(sizeof(T)).c_str());
    MPI_Info_set(info, "same_disp_unit", "true");
    MPI_Win_allocate_shared(intraNodeComm.rank() == 0 ? sizeof(T) : 0, sizeof(T),
                            info, MPI_COMM_WORLD, &fMainTaskID, fMainTaskIDWindow.get());
}

template<std::integral T>
auto SharedMemoryScheduler<T>::PreLoopAction() -> void {
    const auto& intraNodeComm{Env::MPIEnv::Instance().IntraNodeComm()};
    fBatchSize = static_cast<T>(fgImbalancingFactor / 2 * static_cast<double>(this->NTask()) / intraNodeComm.size()) + 1;
    this->fExecutingTask = this->fTask.first + intraNodeComm.rank() * fBatchSize;
    if (fMainTaskID) {
        *fMainTaskID = this->fTask.first + intraNodeComm.size() * fBatchSize;
        MPI_Win_fence(MPI_MODE_NOPRECEDE, *fMainTaskIDWindow);
    } else {
        MPI_Win_fence(MPI_MODE_NOSTORE | MPI_MODE_NOPUT | MPI_MODE_NOPRECEDE, *fMainTaskIDWindow);
    }
}

template<std::integral T>
auto SharedMemoryScheduler<T>::PostTaskAction() -> void {
    if (++fBatchCounter == fBatchSize) {
        fBatchCounter = 0;
        MPI_Win_lock(MPI_LOCK_SHARED, 0, 0, *fMainTaskIDWindow);
        MPI_Fetch_and_op(&fBatchSize, &this->fExecutingTask, MPIX::DataType<T>(), 0, 0, MPI_SUM, *fMainTaskIDWindow);
        MPI_Win_unlock(0, *fMainTaskIDWindow);
        this->fExecutingTask = std::min(this->fExecutingTask, this->fTask.last);
    } else {
        ++this->fExecutingTask;
    }
}

template<std::integral T>
auto SharedMemoryScheduler<T>::PostLoopAction() -> void {
    fBatchCounter = 0;
    if (fMainTaskID) {
        MPI_Win_fence(MPI_MODE_NOSUCCEED, *fMainTaskIDWindow);
    } else {
        MPI_Win_fence(MPI_MODE_NOSTORE | MPI_MODE_NOPUT | MPI_MODE_NOSUCCEED, *fMainTaskIDWindow);
    }
}

template<std::integral T>
auto SharedMemoryScheduler<T>::NExecutedTaskEstimation() const -> std::pair<bool, T> {
    return {this->fNLocalExecutedTask > 10 * fBatchSize,
            this->fExecutingTask - this->fTask.first};
}

template<std::integral T>
struct SharedMemoryScheduler<T>::MPIWinDeleter {
    auto operator()(MPI_Win* win) const -> void {
        Expects(win != nullptr);
        if (*win != MPI_WIN_NULL) {
            MPI_Win_free(win);
        }
        delete win;
    }
};

} // namespace Mustard::inline Extension::MPIX::inline Execution
