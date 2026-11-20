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
MasterWorkerDispatcher<T>::Master::Master(MasterWorkerDispatcher<T>* s) :
    fS{s},
    fRecv{},
    fTaskIDSend{},
    fSend{} {
    const auto commSize{fS->fComm.size()};
    fRecv.reserve(commSize);
    for (int src{}; src < commSize; ++src) {
        fRecv.emplace_back(mp1::recv_init(fS->fComm, mp1::empty_data, src));
    }
    fTaskIDSend.resize(commSize);
    fSend.reserve(commSize);
    for (int dest{}; dest < commSize; ++dest) {
        fSend.emplace_back(mp1::rsend_init(fS->fComm, fTaskIDSend[dest], dest));
    }
}

template<std::integral T>
auto MasterWorkerDispatcher<T>::Master::StartAll() -> void {
    mp1::start_all(fRecv);
}

template<std::integral T>
auto MasterWorkerDispatcher<T>::Master::operator()() -> void {
    T mainTaskID{fS->fTask.first + fS->fComm.size() * fS->fBatchSize};
    while (true) {
        const auto completed{mp1::wait_some(fRecv)};
        if (completed.empty()) {
            break;
        }
        for (auto&& [rank, status] : std::as_const(completed)) {
            const auto taskID{std::min(mainTaskID, fS->fTask.last)};
            if (taskID != fS->fTask.last) [[likely]] {
                // The worker's next request is a ready send: post fRecv before waiting.
                mainTaskID += fS->fBatchSize;
                fRecv[rank].start();
            }
            fSend[rank].wait();
            fTaskIDSend[rank] = taskID; // fTaskIDSend[rank] can only be reused after fSend[rank].wait() returns
            fSend[rank].start();
        }
    }
    mp1::wait_all(fSend);
}

template<std::integral T>
MasterWorkerDispatcher<T>::MasterWorkerDispatcher() :
    Dispatcher<T>{},
    fComm{},
    fBatchSize{},
    fMaster{},
    fMasterThread{},
    fSend{},
    fTaskIDRecv{},
    fRecv{},
    fTaskCounter{} {
    mp1::info commInfo;
    commInfo.set("mpi_assert_no_any_tag", "true");
    commInfo.set("mpi_assert_no_any_source", "true");
    commInfo.set("mpi_assert_exact_length", "true");
    commInfo.set("mpi_assert_allow_overtaking", "true");
    fComm = mp1::communicator::dup_with_info(mp1::comm_world(), commInfo);
    if (fComm.rank() == 0) {
        fMaster = std::make_unique<Master>(this);
    }
    fSend = mp1::rsend_init(fComm, mp1::empty_data, 0);
    fRecv = mp1::recv_init(fComm, fTaskIDRecv, 0);
}

template<std::integral T>
auto MasterWorkerDispatcher<T>::PreLoopAction() -> void {
    fBatchSize = std::max(1ll, std::llround(fgImbalancingFactor * this->NTask() / fComm.size()));
    this->fExecutingTask = this->fTask.first + fComm.rank() * fBatchSize;
    fTaskCounter = 0;

    if (fMaster) {
        fMaster->StartAll();
        fMasterThread = std::jthread{std::ref(*fMaster)};
    }
    mp1::barrier(fComm);
}

template<std::integral T>
auto MasterWorkerDispatcher<T>::PreTaskAction() -> void {
    if (fTaskCounter == 0) {
        fRecv.start();
        fSend.start();
    }
}

template<std::integral T>
auto MasterWorkerDispatcher<T>::PostTaskAction() -> void {
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
auto MasterWorkerDispatcher<T>::PostLoopAction() -> void {
    fSend.wait();
    fRecv.wait();

    if (fMasterThread.joinable()) {
        fMasterThread.join();
    }
}

template<std::integral T>
auto MasterWorkerDispatcher<T>::NExecutedTaskEstimation() const -> std::pair<bool, T> {
    return {this->fNLocalExecutedTask > 10 * fBatchSize,
            this->fExecutingTask - this->fTask.first};
}

} // namespace Mustard::inline Execution
