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
    fComm{},
    fBatchSize{},
    fContext{} {
    if (fComm.Rank() == 0) {
        fContext.template emplace<Master>(this);
    } else {
        fContext.template emplace<Worker>(this);
    }
}

template<std::integral T>
DynamicScheduler<T>::Comm::Comm() :
    fComm{
        [] {
            MPI_Comm comm;
            MPI_Comm_dup(MPI_COMM_WORLD, // comm
                         &comm);         // newcomm
            return comm;
        }()},
    fRank{
        [this] {
            int rank;
            MPI_Comm_rank(fComm,  // comm
                          &rank); // rank
            return rank;
        }()},
    fSize{
        [this] {
            int size;
            MPI_Comm_size(fComm,  // comm
                          &size); // size
            return size;
        }()} {}

template<std::integral T>
DynamicScheduler<T>::Comm::~Comm() {
    auto comm{fComm};
    MPI_Comm_free(&comm);
}

template<std::integral T>
auto DynamicScheduler<T>::PreLoopAction() -> void {
    // width ~ BalanceFactor -> +/- BalanceFactor / 2
    fBatchSize = static_cast<T>(fgBalancingFactor / 2 * static_cast<double>(this->NTask()) / fComm.Size()) + 1;
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
    fRecv{},
    fTaskIDSend{},
    fSend{},
    fSupervisorThread{} {
    if (fDS->fComm.Size() > 1) {
        fRecv.reserve(fDS->fComm.Size() - 1);
        fTaskIDSend.reserve(fDS->fComm.Size() - 1);
        fSend.reserve(fDS->fComm.Size() - 1);
        for (int src{1}; src < fDS->fComm.Size(); ++src) {
            MPI_Recv_init(nullptr,                // buf
                          0,                      // count
                          MPI_BYTE,               // datatype
                          src,                    // source
                          0,                      // tag
                          fDS->fComm,             // comm
                          &fRecv.emplace_back()); // request
        }
        for (int dest{1}; dest < fDS->fComm.Size(); ++dest) {
            MPI_Rsend_init(&fTaskIDSend.emplace_back(), // buf
                           1,                           // count
                           DataType<T>(),               // datatype
                           dest,                        // dest
                           1,                           // tag
                           fDS->fComm,                  // comm
                           &fSend.emplace_back());      // request
        }
    }
}

template<std::integral T>
DynamicScheduler<T>::Master::Supervisor::~Supervisor() {
    if (fSupervisorThread.joinable()) { fSupervisorThread.join(); } // wait for last supervision to end if needed
    for (auto&& s : fSend) { MPI_Request_free(&s); }
    for (auto&& r : fRecv) { MPI_Request_free(&r); }
}

template<std::integral T>
auto DynamicScheduler<T>::Master::Supervisor::Start() -> void {
    fMainTaskID = fDS->fTask.first + fDS->fComm.Size() * fDS->fBatchSize;
    // No need of supervisor in sequential execution
    if (fDS->fComm.Size() == 1) { return; }
    // Check MPI thread support
    switch (Env::MPIEnv::Instance().MPIThreadSupport()) {
    case MPI_THREAD_SINGLE:
        throw std::runtime_error{PrettyException("The MPI library provides MPI_THREAD_SINGLE, "
                                                 "but dynamic scheduler requires MPI_THREAD_MULTIPLE")};
    case MPI_THREAD_FUNNELED:
        throw std::runtime_error{PrettyException("The MPI library provides MPI_THREAD_FUNNELED, "
                                                 "but dynamic scheduler requires MPI_THREAD_MULTIPLE")};
    case MPI_THREAD_SERIALIZED:
        throw std::runtime_error{PrettyException("The MPI library provides MPI_THREAD_SERIALIZED, "
                                                 "but dynamic scheduler requires MPI_THREAD_MULTIPLE")};
    }
    // wait for last supervision to end if needed
    if (fSupervisorThread.joinable()) { fSupervisorThread.join(); }
    // Start supervise
    fSupervisorThread = std::jthread{
        [this] {
            MPI_Startall(fRecv.size(),  // count
                         fRecv.data()); // array_of_requests
            // inform workers that receive have been posted
            MPI_Request firstSupervisorRecvReadyBcast;
            MPI_Ibcast(nullptr,                         // buffer
                       0,                               // count
                       MPI_BYTE,                        // datatype
                       0,                               // root
                       fDS->fComm,                      // comm
                       &firstSupervisorRecvReadyBcast); // request
            int completing{};
            do {
                int cgCount;
                std::vector<int> cgRank(fDS->fComm.Size() - 1);
                MPI_Waitsome(fRecv.size(),         // incount
                             fRecv.data(),         // array_of_requests
                             &cgCount,             // outcount
                             cgRank.data(),        // array_of_indices
                             MPI_STATUSES_IGNORE); // array_of_statuses
                for (int i{}; i < cgCount; ++i) {
                    const auto c{cgRank[i]};
                    fTaskIDSend[c] = FetchAddTaskID();
                    if (fTaskIDSend[c] != fDS->fTask.last) {
                        MPI_Start(&fRecv[c]);
                    } else {
                        ++completing;
                    }
                    MPI_Wait(&fSend[c],          // request
                             MPI_STATUS_IGNORE); // status
                    MPI_Start(&fSend[c]);
                }
            } while (completing != fDS->fComm.Size() - 1);
            MPI_Wait(&firstSupervisorRecvReadyBcast, // request
                     MPI_STATUS_IGNORE);             // status
            MPI_Waitall(fSend.size(),                // count
                        fSend.data(),                // array_of_requests
                        MPI_STATUSES_IGNORE);        // array_of_statuses
        }};
}

template<std::integral T>
auto DynamicScheduler<T>::Master::Supervisor::FetchAddTaskID() -> T {
    return std::min(fMainTaskID.fetch_add(fDS->fBatchSize, std::memory_order::relaxed),
                    fDS->fTask.last);
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
    fTaskIDRecv{},
    fRequest{},
    fBatchCounter{} {
    auto& [send, recv]{fRequest};
    MPI_Rsend_init(nullptr,      // buf
                   0,            // count
                   MPI_BYTE,     // datatype
                   0,            // dest
                   0,            // tag
                   fDS->fComm,   // comm
                   &send);       // request
    MPI_Recv_init(&fTaskIDRecv,  // buf
                  1,             // count
                  DataType<T>(), // datatype
                  0,             // source
                  1,             // tag
                  fDS->fComm,    // comm
                  &recv);        // request
}

template<std::integral T>
DynamicScheduler<T>::Worker::~Worker() {
    auto& [send, recv]{fRequest};
    MPI_Request_free(&recv);
    MPI_Request_free(&send);
}

template<std::integral T>
auto DynamicScheduler<T>::Worker::PreLoopAction() -> void {
    fDS->fExecutingTask = fDS->fTask.first + fDS->fComm.Rank() * fDS->fBatchSize;
    fBatchCounter = 0;
    // wait for supervisor to post receive
    MPI_Request firstSupervisorRecvReadyBcast;
    MPI_Ibcast(nullptr,                         // buffer
               0,                               // count
               MPI_BYTE,                        // datatype
               0,                               // root
               fDS->fComm,                      // comm
               &firstSupervisorRecvReadyBcast); // request
    MPI_Wait(&firstSupervisorRecvReadyBcast,    // request
             MPI_STATUS_IGNORE);                // status
}

template<std::integral T>
auto DynamicScheduler<T>::Worker::PreTaskAction() -> void {
    if (fBatchCounter == 0) {
        auto& [send, recv]{fRequest};
        MPI_Start(&recv);
        MPI_Start(&send);
    }
}

template<std::integral T>
auto DynamicScheduler<T>::Worker::PostTaskAction() -> void {
    if (++fBatchCounter == fDS->fBatchSize) {
        fBatchCounter = 0;
        MPI_Waitall(fRequest.size(),      // count
                    fRequest.data(),      // array_of_requests
                    MPI_STATUSES_IGNORE); // array_of_statuses
        fDS->fExecutingTask = fTaskIDRecv;
    } else {
        ++fDS->fExecutingTask;
    }
}

template<std::integral T>
auto DynamicScheduler<T>::Worker::PostLoopAction() -> void {
    MPI_Waitall(fRequest.size(),      // count
                fRequest.data(),      // array_of_requests
                MPI_STATUSES_IGNORE); // array_of_statuses
}

} // namespace Mustard::inline Extension::MPIX::inline Execution
