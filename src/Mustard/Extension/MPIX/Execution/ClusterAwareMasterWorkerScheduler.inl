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
ClusterAwareMasterWorkerScheduler<T>::MasterBase::MasterBase(ClusterAwareMasterWorkerScheduler<T>* s) :
    fS{s} {
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
}

template<std::integral T>
ClusterAwareMasterWorkerScheduler<T>::ClusterMaster::ClusterMaster(ClusterAwareMasterWorkerScheduler<T>* s) :
    MasterBase{s},
    fSemaphoreRecvFromNM{},
    fRecvFromNM{},
    fTaskIDSendToNM{},
    fSendToNM{} {
    const auto commSize{s->fInterNodeComm.size()};
    for (int src{}; src < commSize; ++src) {
        fRecvFromNM.push(s->fInterNodeComm.recv_init(fSemaphoreRecvFromNM, src));
    }
    fTaskIDSendToNM.reserve(commSize);
    for (int dest{}; dest < commSize; ++dest) {
        fSendToNM.push(s->fInterNodeComm.rsend_init(fTaskIDSendToNM.emplace_back(), dest));
    }
}

template<std::integral T>
auto ClusterAwareMasterWorkerScheduler<T>::ClusterMaster::StartAll() -> void {
    fRecvFromNM.startall();
    this->fS->fInterNodeComm.barrier();
}

template<std::integral T>
auto ClusterAwareMasterWorkerScheduler<T>::ClusterMaster::operator()() -> void {
    auto interNodeTaskID{muc::ranges::reduce(this->fS->fInterNodeBatchSize, this->fS->fTask.first)};
    while (true) {
        const auto [result, recvRank]{fRecvFromNM.waitsome(mpl::duty_ratio::preset::active)};
        if (result == mpl::test_result::no_active_requests) {
            break;
        }
        for (auto&& rank : recvRank) {
            fTaskIDSendToNM[rank] = std::min(interNodeTaskID, this->fS->fTask.last);
            if (fTaskIDSendToNM[rank] != this->fS->fTask.last) [[likely]] {
                interNodeTaskID += this->fS->fInterNodeBatchSize[rank];
                fRecvFromNM.start(rank);
            }
            fSendToNM.wait(rank);
            fSendToNM.start(rank);
        }
    }
    fSendToNM.waitall(mpl::duty_ratio::preset::moderate);
}

template<std::integral T>
ClusterAwareMasterWorkerScheduler<T>::NodeMaster::NodeMaster(ClusterAwareMasterWorkerScheduler<T>* s) :
    MasterBase{s},
    fClusterMaster{s->fInterNodeComm.rank() == 0 ? std::make_unique<ClusterMaster>(s) : nullptr},
    fClusterMasterThread{},
    fSemaphoreSendToCM{},
    fSendToCM{s->fInterNodeComm.rsend_init(fSemaphoreSendToCM, 0)},
    fTaskIDRecvFromCM{},
    fRecvFromCM{s->fInterNodeComm.recv_init(fTaskIDRecvFromCM, 0)},
    fSemaphoreRecvFromW{},
    fRecvFromW{},
    fTaskIDSendToW{},
    fSendToW{} {
    const auto nodeSize{s->fIntraNodeComm.size()};
    for (int src{}; src < nodeSize; ++src) {
        fRecvFromW.push(s->fIntraNodeComm.recv_init(fSemaphoreRecvFromW, src));
    }
    fTaskIDSendToW.reserve(nodeSize);
    for (int dest{}; dest < nodeSize; ++dest) {
        fSendToW.push(s->fIntraNodeComm.rsend_init(fTaskIDSendToW.emplace_back(), dest));
    }
}

template<std::integral T>
auto ClusterAwareMasterWorkerScheduler<T>::NodeMaster::StartAll() -> void {
    fRecvFromW.startall();

    if (fClusterMaster) {
        fClusterMaster->StartAll();
        fClusterMasterThread = std::jthread{std::ref(*fClusterMaster)};
    }
}

template<std::integral T>
auto ClusterAwareMasterWorkerScheduler<T>::NodeMaster::operator()() -> void {
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    const auto intraNodeFirstTaskID{std::reduce(
        this->fS->fInterNodeBatchSize.cbegin(), this->fS->fInterNodeBatchSize.cbegin() + mpiEnv.LocalNodeID(),
        this->fS->fTask.first)};
    auto intraNodeTaskID{intraNodeFirstTaskID + mpiEnv.LocalNode().size * this->fS->fIntraNodeBatchSize};
    auto intraNodeTaskEnd{intraNodeFirstTaskID + this->fS->fInterNodeBatchSize[mpiEnv.LocalNodeID()]};

    if (not fClusterMaster) {
        this->fS->fInterNodeComm.barrier();
    }
    fRecvFromCM.start();
    fSendToCM.start();
    while (true) {
        const auto [result, recvRank]{fRecvFromW.waitsome(mpl::duty_ratio::preset::active)};
        if (result == mpl::test_result::no_active_requests) {
            break;
        }
        for (auto&& rank : recvRank) {
            if (intraNodeTaskID == intraNodeTaskEnd and intraNodeTaskID != this->fS->fTask.last) {
                fSendToCM.wait();
                fRecvFromCM.wait();
                intraNodeTaskID = fTaskIDRecvFromCM;
                if (intraNodeTaskID != this->fS->fTask.last) {
                    intraNodeTaskEnd = intraNodeTaskID + this->fS->fInterNodeBatchSize[Env::MPIEnv::Instance().LocalNodeID()];
                    fRecvFromCM.start();
                    fSendToCM.start();
                } else {
                    intraNodeTaskEnd = intraNodeTaskID;
                }
            }
            fTaskIDSendToW[rank] = std::min(intraNodeTaskID, this->fS->fTask.last);
            if (fTaskIDSendToW[rank] != this->fS->fTask.last) [[likely]] {
                intraNodeTaskID += this->fS->fIntraNodeBatchSize;
                fRecvFromW.start(rank);
            }
            fSendToW.wait(rank);
            fSendToW.start(rank);
        }
    }
    fSendToCM.wait(mpl::duty_ratio::preset::moderate);
    fRecvFromCM.wait(mpl::duty_ratio::preset::moderate);
    fSendToW.waitall(mpl::duty_ratio::preset::moderate);

    if (fClusterMasterThread.joinable()) {
        fClusterMasterThread.join();
    }
}

template<std::integral T>
ClusterAwareMasterWorkerScheduler<T>::ClusterAwareMasterWorkerScheduler() :
    Scheduler<T>{},
    fIntraNodeComm{},
    fInterNodeComm{},
    fIntraNodeBatchSize{},
    fInterNodeBatchSizeMultiplicity{},
    fInterNodeBatchSize{},
    fNodeMaster{},
    fNodeMasterThread{},
    fSemaphoreSendToNM{},
    fSendToNM{},
    fTaskIDRecvFromNM{},
    fRecvFromNM{},
    fIntraNodeTaskCounter{} {
    mpl::info commInfo;
    commInfo.set("mpi_assert_no_any_tag", "true");
    commInfo.set("mpi_assert_no_any_source", "true");
    commInfo.set("mpi_assert_exact_length", "true");
    commInfo.set("mpi_assert_allow_overtaking", "true");
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    fIntraNodeComm = mpl::communicator{mpiEnv.IntraNodeComm(), commInfo};
    if (mpiEnv.InterNodeComm().is_valid()) {
        fInterNodeComm = mpl::communicator{mpiEnv.InterNodeComm(), commInfo};
        fNodeMaster = std::make_unique<NodeMaster>(this);
    }
    fInterNodeBatchSize.resize(mpiEnv.ClusterSize());
    fSendToNM = fIntraNodeComm.rsend_init(fSemaphoreSendToNM, 0);
    fRecvFromNM = fIntraNodeComm.recv_init(fTaskIDRecvFromNM, 0);
}

template<std::integral T>
auto ClusterAwareMasterWorkerScheduler<T>::PreLoopAction() -> void {
    const auto avgTaskPerProc{static_cast<long double>(this->NTask()) / mpl::environment::comm_world().size()};
    fInterNodeBatchSizeMultiplicity = std::min(muc::lltrunc(avgTaskPerProc), fgMaxInterNodeBatchSizeMultiplicity);
    Ensures(fInterNodeBatchSizeMultiplicity >= 1);
    fIntraNodeBatchSize = std::max(1ll, std::llround(fgImbalancingFactor * avgTaskPerProc / fInterNodeBatchSizeMultiplicity));
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    std::ranges::transform(
        mpiEnv.NodeList(), fInterNodeBatchSize.begin(),
        [this](auto&& node) {
            return fInterNodeBatchSizeMultiplicity * node.size * fIntraNodeBatchSize;
        });

    const auto intraNodeFirstTaskID{std::reduce(
        fInterNodeBatchSize.cbegin(), fInterNodeBatchSize.cbegin() + mpiEnv.LocalNodeID(),
        this->fTask.first)};
    this->fExecutingTask = intraNodeFirstTaskID + fIntraNodeComm.rank() * fIntraNodeBatchSize;
    fIntraNodeTaskCounter = 0;

    if (fNodeMaster) {
        fNodeMaster->StartAll();
        fNodeMasterThread = std::jthread{std::ref(*fNodeMaster)};
    }
}

template<std::integral T>
auto ClusterAwareMasterWorkerScheduler<T>::PreTaskAction() -> void {
    if (fIntraNodeTaskCounter == 0) {
        fRecvFromNM.start();
        fSendToNM.start();
    }
}

template<std::integral T>
auto ClusterAwareMasterWorkerScheduler<T>::PostTaskAction() -> void {
    if (++fIntraNodeTaskCounter == fIntraNodeBatchSize) {
        fSendToNM.wait();
        fRecvFromNM.wait();
        this->fExecutingTask = fTaskIDRecvFromNM;
        fIntraNodeTaskCounter = 0;
    } else {
        ++this->fExecutingTask;
    }
}

template<std::integral T>
auto ClusterAwareMasterWorkerScheduler<T>::PostLoopAction() -> void {
    fSendToNM.wait(mpl::duty_ratio::preset::moderate);
    fRecvFromNM.wait(mpl::duty_ratio::preset::moderate);

    if (fNodeMasterThread.joinable()) {
        fNodeMasterThread.join();
    }
}

template<std::integral T>
auto ClusterAwareMasterWorkerScheduler<T>::NExecutedTaskEstimation() const -> std::pair<bool, T> {
    return {this->fNLocalExecutedTask > 10 * fInterNodeBatchSizeMultiplicity * fIntraNodeBatchSize,
            this->fExecutingTask - this->fTask.first};
}

} // namespace Mustard::inline Extension::MPIX::inline Execution
