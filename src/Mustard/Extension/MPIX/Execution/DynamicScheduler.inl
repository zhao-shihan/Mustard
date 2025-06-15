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
DynamicScheduler<T>::DynamicScheduler() :
    Scheduler<T>{},
    fComm{mpl::environment::comm_world()},
    fBatchSize{},
    fContext{fComm.rank() == 0 ?
                 decltype(fContext)(std::in_place_type<Master>, this) :
                 decltype(fContext)(std::in_place_type<Worker>, this)} {}

template<std::integral T>
auto DynamicScheduler<T>::PreLoopAction() -> void {
    // width ~ BalanceFactor -> +/- BalanceFactor / 2
    fBatchSize = static_cast<T>(fgBalancingFactor / 2 * static_cast<double>(this->NTask()) / fComm.size()) + 1;
    std::visit([](auto&& c) { c.PreLoopAction(); }, fContext);
}

template<std::integral T>
auto DynamicScheduler<T>::PreTaskAction() -> void {
    std::visit([](auto&& c) { c.PreTaskAction(); }, fContext);
}

template<std::integral T>
auto DynamicScheduler<T>::PostTaskAction() -> void {
    std::visit([](auto&& c) { c.PostTaskAction(); }, fContext);
}

template<std::integral T>
auto DynamicScheduler<T>::PostLoopAction() -> void {
    std::visit([](auto&& c) { c.PostLoopAction(); }, fContext);
}

template<std::integral T>
auto DynamicScheduler<T>::NExecutedTask() const -> std::pair<bool, T> {
    return {this->fNLocalExecutedTask > 10 * fBatchSize,
            this->fExecutingTask - this->fTask.first};
}

template<std::integral T>
DynamicScheduler<T>::Master::Supervisor::Supervisor(DynamicScheduler<T>* ds) :
    fDS{ds},
    fMainTaskID{},
    fSemaphoreRecv{},
    fRecv{},
    fTaskIDSend{},
    fSend{},
    fSupervisorThread{} {
    if (const auto commSize{fDS->fComm.size()};
        commSize > 1) {
        fTaskIDSend.reserve(commSize - 1);
        for (int src{1}; src < commSize; ++src) {
            fRecv.push(fDS->fComm.recv_init(fSemaphoreRecv, src));
        }
        for (int dest{1}; dest < commSize; ++dest) {
            fSend.push(fDS->fComm.rsend_init(fTaskIDSend.emplace_back(), dest));
        }
    }
}

template<std::integral T>
auto DynamicScheduler<T>::Master::Supervisor::Start() -> void {
    fMainTaskID = fDS->fTask.first + fDS->fComm.size() * fDS->fBatchSize;
    // No need of supervisor thread in sequential execution
    if (fDS->fComm.size() == 1) { return; }
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
    // wait for last supervision to end if needed
    if (fSupervisorThread.joinable()) { fSupervisorThread.join(); }
    // Start supervise
    fSupervisorThread = std::jthread{
        [this] {
            fRecv.startall();
            // inform workers that receive have been posted
            std::byte firstRecvReadySemaphore{};
            auto firstRecvReadyBcast{fDS->fComm.ibcast(0, firstRecvReadySemaphore)};
            int completing{};
            do {
                const auto [_, cgIndex]{fRecv.waitsome()};
                for (auto&& i : cgIndex) {
                    fTaskIDSend[i] = FetchAddTaskID();
                    if (fTaskIDSend[i] != fDS->fTask.last) {
                        fRecv.start(i);
                    } else {
                        ++completing;
                    }
                    fSend.wait(i);
                    fSend.start(i);
                }
            } while (completing != fDS->fComm.size() - 1);
            firstRecvReadyBcast.wait();
            fSend.waitall();
        }};
}

template<std::integral T>
auto DynamicScheduler<T>::Master::Supervisor::FetchAddTaskID() -> T {
    return std::min(fMainTaskID.fetch_add(fDS->fBatchSize), fDS->fTask.last);
}

template<std::integral T>
DynamicScheduler<T>::Master::Master(DynamicScheduler<T>* ds) :
    fDS{ds},
    fSupervisor{ds},
    fBatchCounter{} {}

template<std::integral T>
auto DynamicScheduler<T>::Master::PreLoopAction() -> void {
    fSupervisor.Start();
    fDS->fExecutingTask = fDS->fTask.first;
    fBatchCounter = 0;
}

template<std::integral T>
auto DynamicScheduler<T>::Master::PostTaskAction() -> void {
    if (++fBatchCounter == fDS->fBatchSize) {
        fBatchCounter = 0;
        fDS->fExecutingTask = fSupervisor.FetchAddTaskID();
    } else {
        ++fDS->fExecutingTask;
    }
}

template<std::integral T>
DynamicScheduler<T>::Worker::Worker(DynamicScheduler<T>* ds) :
    fDS{ds},
    fSemaphoreSend{},
    fSend{fDS->fComm.rsend_init(fSemaphoreSend, 0)},
    fTaskIDRecv{},
    fRecv{fDS->fComm.recv_init(fTaskIDRecv, 0)},
    fBatchCounter{} {}

template<std::integral T>
auto DynamicScheduler<T>::Worker::PreLoopAction() -> void {
    fDS->fExecutingTask = fDS->fTask.first + fDS->fComm.rank() * fDS->fBatchSize;
    fBatchCounter = 0;
    // wait for supervisor to post receive
    std::byte firstRecvReadySemaphore{};
    fDS->fComm.ibcast(0, firstRecvReadySemaphore).wait();
}

template<std::integral T>
auto DynamicScheduler<T>::Worker::PreTaskAction() -> void {
    if (fBatchCounter == 0) {
        fRecv.start();
        fSend.start();
    }
}

template<std::integral T>
auto DynamicScheduler<T>::Worker::PostTaskAction() -> void {
    if (++fBatchCounter == fDS->fBatchSize) {
        fBatchCounter = 0;
        fSend.wait();
        fRecv.wait();
        fDS->fExecutingTask = fTaskIDRecv;
    } else {
        ++fDS->fExecutingTask;
    }
}

template<std::integral T>
auto DynamicScheduler<T>::Worker::PostLoopAction() -> void {
    fSend.wait();
    fRecv.wait();
}

} // namespace Mustard::inline Extension::MPIX::inline Execution
