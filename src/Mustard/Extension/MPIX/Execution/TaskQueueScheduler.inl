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
TaskQueueScheduler<T>::TaskQueueScheduler() :
    Scheduler<T>{},
    fMainTaskID{},
    fMainTaskIDWindowInfo{},
    fMainTaskIDWindow{},
    fBatchSize{},
    fBatchCounter{} {
    MPI_Info_create(&fMainTaskIDWindowInfo);
    MPI_Info_set(fMainTaskIDWindowInfo, "accumulate_ops", "same_op");
    MPI_Info_set(fMainTaskIDWindowInfo, "mpi_accumulate_granularity", std::to_string(sizeof(T)).c_str());
    MPI_Info_set(fMainTaskIDWindowInfo, "same_size", "true");
    MPI_Info_set(fMainTaskIDWindowInfo, "same_disp_unit", "true");
    MPI_Win_create(&fMainTaskID, sizeof(T), sizeof(T), fMainTaskIDWindowInfo, MPI_COMM_WORLD, &fMainTaskIDWindow);
}

template<std::integral T>
TaskQueueScheduler<T>::~TaskQueueScheduler() {
    MPI_Win_free(&fMainTaskIDWindow);
    MPI_Info_free(&fMainTaskIDWindowInfo);
}

template<std::integral T>
auto TaskQueueScheduler<T>::PreLoopAction() -> void {
    const auto& commWorld{mpl::environment::comm_world()};
    fBatchCounter = 0;
    fBatchSize = static_cast<T>(fgBalancingFactor / 2 * static_cast<double>(this->NTask()) / commWorld.size()) + 1;
    fMainTaskID = this->fTask.first + commWorld.size() * fBatchSize;
    this->fExecutingTask = this->fTask.first + commWorld.rank() * fBatchSize;
    if (commWorld.rank() == 0) {
        MPI_Win_fence(MPI_MODE_NOPRECEDE, fMainTaskIDWindow);
    } else {
        MPI_Win_fence(MPI_MODE_NOSTORE | MPI_MODE_NOPUT | MPI_MODE_NOPRECEDE, fMainTaskIDWindow);
    }
}

template<std::integral T>
auto TaskQueueScheduler<T>::PostTaskAction() -> void {
    if (++fBatchCounter == fBatchSize) {
        fBatchCounter = 0;
        MPI_Win_lock(MPI_LOCK_SHARED, 0, 0, fMainTaskIDWindow);
        MPI_Fetch_and_op(&fBatchSize, &this->fExecutingTask, MPIX::DataType<T>(), 0, 0, MPI_SUM, fMainTaskIDWindow);
        MPI_Win_unlock(0, fMainTaskIDWindow);
        this->fExecutingTask = std::min(this->fExecutingTask, this->fTask.last);
    } else {
        ++this->fExecutingTask;
    }
}

template<std::integral T>
auto TaskQueueScheduler<T>::PostLoopAction() -> void {
    if (mpl::environment::comm_world().rank() == 0) {
        MPI_Win_fence(MPI_MODE_NOSUCCEED, fMainTaskIDWindow);
    } else {
        MPI_Win_fence(MPI_MODE_NOSTORE | MPI_MODE_NOPUT | MPI_MODE_NOSUCCEED, fMainTaskIDWindow);
    }
}

template<std::integral T>
auto TaskQueueScheduler<T>::NExecutedTaskEstimation() const -> std::pair<bool, T> {
    return {this->fNLocalExecutedTask > 10 * fBatchSize,
            this->fExecutingTask - this->fTask.first};
}

} // namespace Mustard::inline Extension::MPIX::inline Execution
