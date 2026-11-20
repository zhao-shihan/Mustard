// -*- C++ -*-
//
// Copyright (C) 2020-2026  Shihan Zhao and contributors
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
SharedMemoryDispatcher<T>::SharedMemoryDispatcher() :
    Dispatcher<T>{},
    fMainTaskIDWindow{},
    fBatchSize{},
    fTaskCounter{} {
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    if (mpiEnv.OnCluster()) {
        Throw<std::runtime_error>("World communicator involves multiple shared memory domains.");
    }
    mp1::info winInfo;
    winInfo.set("accumulate_ops", "same_op");
    winInfo.set("mpi_accumulate_granularity", std::to_string(sizeof(T)));
    winInfo.set("same_disp_unit", "true");
    const auto& intraNodeComm{mpiEnv.IntraNodeComm()};
    fMainTaskIDWindow = mp1::window::allocate_shared(
        intraNodeComm,
        {.size = intraNodeComm.rank() == 0 ? static_cast<MPI_Aint>(sizeof(T)) : 0,
         .disp_unit = sizeof(T),
         .info = winInfo});
}

template<std::integral T>
auto SharedMemoryDispatcher<T>::PreLoopAction() -> void {
    const auto& intraNodeComm{Env::MPIEnv::Instance().IntraNodeComm()};
    fBatchSize = std::max(1ll, std::llround(fgImbalancingFactor * this->NTask() / intraNodeComm.size()));
    this->fExecutingTask = this->fTask.first + intraNodeComm.rank() * fBatchSize;
    fTaskCounter = 0;

    mp1::lock(fMainTaskIDWindow, mp1::lock_type::shared, 0, MPI_MODE_NOCHECK);
    if (intraNodeComm.rank() == 0) {
        *static_cast<T*>(fMainTaskIDWindow.base()) = this->fTask.first + intraNodeComm.size() * fBatchSize;
        mp1::sync(fMainTaskIDWindow);
    }
}

template<std::integral T>
auto SharedMemoryDispatcher<T>::PostTaskAction() -> void {
    if (++fTaskCounter == fBatchSize) {
        mp1::fetch_and_op(fMainTaskIDWindow, &fBatchSize, &this->fExecutingTask, 0, 0, std::plus{});
        mp1::flush(fMainTaskIDWindow, 0);
        this->fExecutingTask = std::min(this->fExecutingTask, this->fTask.last);
        fTaskCounter = 0;
    } else {
        ++this->fExecutingTask;
    }
}

template<std::integral T>
auto SharedMemoryDispatcher<T>::PostLoopAction() -> void {
    mp1::unlock(fMainTaskIDWindow, 0);
}

template<std::integral T>
auto SharedMemoryDispatcher<T>::NExecutedTaskEstimation() const -> std::pair<bool, T> {
    return {this->fNLocalExecutedTask > 10 * fBatchSize,
            this->fExecutingTask - this->fTask.first};
}

} // namespace Mustard::inline Execution
