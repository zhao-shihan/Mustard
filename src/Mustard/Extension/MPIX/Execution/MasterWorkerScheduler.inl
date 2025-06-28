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
MasterWorkerScheduler<T>::Master::Master(MasterWorkerScheduler<T>* s) :
    NonMoveableBase{},
    fS{s},
    fSemaphoreRecv{},
    fRecv{},
    fTaskIDSend{},
    fSend{} {
    // Check MPI thread support
    switch (mpl::environment::threading_mode()) {
    case mpl::threading_modes::single:
        Throw<std::runtime_error>("The MPI library provides 'single' thread support, "
                                  "but dynamic scheduler requires 'multiple'");
    case mpl::threading_modes::funneled:
        Throw<std::runtime_error>("The MPI library provides 'funneled' thread support, "
                                  "but dynamic scheduler requires 'multiple'");
    case mpl::threading_modes::serialized:
        Throw<std::runtime_error>("The MPI library provides 'serialized' thread support, "
                                  "but dynamic scheduler requires 'multiple'");
    case mpl::threading_modes::multiple:
        break;
    }
    // Initialize communication requests
    const auto commSize{fS->fComm.size()};
    for (int src{}; src < commSize; ++src) {
        fRecv.push(fS->fComm.recv_init(fSemaphoreRecv, src));
    }
    fTaskIDSend.reserve(commSize);
    for (int dest{}; dest < commSize; ++dest) {
        fSend.push(fS->fComm.rsend_init(fTaskIDSend.emplace_back(), dest));
    }
}

template<std::integral T>
auto MasterWorkerScheduler<T>::Master::operator()() -> void {
    T mainTaskID{fS->fTask.first + fS->fComm.size() * fS->fBatchSize};
    while (true) {
        const auto [result, recvRank]{fRecv.waitsome()};
        if (result == mpl::test_result::no_active_requests) { break; }
        for (auto&& rank : recvRank) {
            fTaskIDSend[rank] = std::min(mainTaskID, fS->fTask.last);
            if (fTaskIDSend[rank] != fS->fTask.last) [[likely]] {
                mainTaskID += fS->fBatchSize;
                fRecv.start(rank);
            }
            fSend.wait(rank);
            fSend.start(rank);
        }
    }
    LazySpinWaitAll(fSend, 0.01);
}

template<std::integral T>
MasterWorkerScheduler<T>::MasterWorkerScheduler() :
    Scheduler<T>{},
    fComm{},
    fBatchSize{},
    fMaster{},
    fAsyncMaster{},
    fSemaphoreSend{},
    fSend{},
    fTaskIDRecv{},
    fRecv{},
    fTaskCounter{} {
    mpl::info commInfo;
    commInfo.set("mpi_assert_no_any_tag", "true");
    commInfo.set("mpi_assert_no_any_source", "true");
    commInfo.set("mpi_assert_exact_length", "true");
    commInfo.set("mpi_assert_allow_overtaking", "true");
    fComm = mpl::communicator{mpl::environment::comm_world(), commInfo};
    if (fComm.rank() == 0) {
        fMaster = std::make_unique<Master>(this);
    }
    fSend = fComm.rsend_init(fSemaphoreSend, 0);
    fRecv = fComm.recv_init(fTaskIDRecv, 0);
}

template<std::integral T>
auto MasterWorkerScheduler<T>::PreLoopAction() -> void {
    fBatchSize = std::max(1ll, std::llround(fgImbalancingFactor * this->NTask() / fComm.size()));
    this->fExecutingTask = this->fTask.first + fComm.rank() * fBatchSize;
    fTaskCounter = 0;

    if (fMaster) {
        fMaster->StartAll();
        fAsyncMaster = std::async(std::launch::async, std::ref(*fMaster));
    }
}

template<std::integral T>
auto MasterWorkerScheduler<T>::PreTaskAction() -> void {
    if (fTaskCounter == 0) {
        fRecv.start();
        fSend.start();
    }
}

template<std::integral T>
auto MasterWorkerScheduler<T>::PostTaskAction() -> void {
    if (++fTaskCounter == fBatchSize) {
        fSend.wait();
        fRecv.wait();
        this->fExecutingTask = fTaskIDRecv;
        fTaskCounter = 0;
    } else {
        ++this->fExecutingTask;
    }
}

template<std::integral T>
auto MasterWorkerScheduler<T>::PostLoopAction() -> void {
    LazySpinWait(fSend, 0.01);
    LazySpinWait(fRecv, 0.01);

    if (fMaster) {
        fAsyncMaster.get();
    }
}

template<std::integral T>
auto MasterWorkerScheduler<T>::NExecutedTaskEstimation() const -> std::pair<bool, T> {
    return {this->fNLocalExecutedTask > 10 * fBatchSize,
            this->fExecutingTask - this->fTask.first};
}

} // namespace Mustard::inline Extension::MPIX::inline Execution
