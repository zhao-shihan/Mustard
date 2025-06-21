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
MasterWorkerScheduler<T>::MasterWorkerScheduler() :
    Scheduler<T>{},
    fComm{mpl::environment::comm_world()},
    fBatchSize{},
    fMaster{fComm.rank() == 0 ? std::make_unique<Master>(this) : nullptr},
    fMasterFuture{},
    fSemaphoreSend{},
    fSend{fComm.rsend_init(fSemaphoreSend, 0)},
    fTaskIDRecv{},
    fRecv{fComm.recv_init(fTaskIDRecv, 0)},
    fBatchCounter{} {}

template<std::integral T>
auto MasterWorkerScheduler<T>::PreLoopAction() -> void {
    fBatchSize = static_cast<T>(fgImbalancingFactor / 2 * static_cast<double>(this->NTask()) / fComm.size()) + 1;
    if (fMaster) {
        fMasterFuture = std::async(std::launch::async, std::ref(*fMaster));
    }
    this->fExecutingTask = this->fTask.first + fComm.rank() * fBatchSize;
    // wait for master thread to post receive
    std::byte firstRecvReadySemaphore{};
    fComm.ibcast(0, firstRecvReadySemaphore).wait();
}

template<std::integral T>
auto MasterWorkerScheduler<T>::PreTaskAction() -> void {
    if (fBatchCounter == 0) {
        fRecv.start();
        fSend.start();
    }
}

template<std::integral T>
auto MasterWorkerScheduler<T>::PostTaskAction() -> void {
    if (++fBatchCounter == fBatchSize) {
        fBatchCounter = 0;
        fSend.wait();
        fRecv.wait();
        this->fExecutingTask = fTaskIDRecv;
    } else {
        ++this->fExecutingTask;
    }
}

template<std::integral T>
auto MasterWorkerScheduler<T>::PostLoopAction() -> void {
    fBatchCounter = 0;
    fSend.wait();
    fRecv.wait();
    if (fMaster) {
        fMasterFuture.get();
    }
}

template<std::integral T>
auto MasterWorkerScheduler<T>::NExecutedTaskEstimation() const -> std::pair<bool, T> {
    return {this->fNLocalExecutedTask > 10 * fBatchSize,
            this->fExecutingTask - this->fTask.first};
}

template<std::integral T>
MasterWorkerScheduler<T>::Master::Master(MasterWorkerScheduler<T>* s) :
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
    fRecv.startall();
    // inform workers that receive have been posted
    std::byte firstRecvReadySemaphore{};
    auto firstRecvReadyBcast{fS->fComm.ibcast(0, firstRecvReadySemaphore)};
    // scheduling loop
    T mainTaskID{fS->fTask.first + fS->fComm.size() * fS->fBatchSize};
    int completing{};
    do {
        const auto [_, cgIndex]{fRecv.waitsome()};
        for (auto&& i : cgIndex) {
            fTaskIDSend[i] = mainTaskID;
            mainTaskID = std::min(mainTaskID + fS->fBatchSize, fS->fTask.last);
            if (fTaskIDSend[i] != fS->fTask.last) [[likely]] {
                fRecv.start(i);
            } else {
                ++completing;
            }
            fSend.wait(i);
            fSend.start(i);
        }
    } while (completing != fS->fComm.size());
    // finalize
    firstRecvReadyBcast.wait();
    fSend.waitall();
}

} // namespace Mustard::inline Extension::MPIX::inline Execution
