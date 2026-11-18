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
ClusterAwareMasterWorkerDispatcher<T>::ClusterMaster::ClusterMaster(ClusterAwareMasterWorkerDispatcher<T>* s) :
    fS{s},
    fRecvFromNM{},
    fTaskIDSendToNM{},
    fSendToNM{} {
    const auto commSize{s->fInterNodeComm.size()};
    fRecvFromNM.reserve(commSize);
    for (int src{}; src < commSize; ++src) {
        fRecvFromNM.emplace_back(mp1::recv_init(s->fInterNodeComm, mp1::empty_data, src));
    }
    fTaskIDSendToNM.resize(commSize);
    fSendToNM.reserve(commSize);
    for (int dest{}; dest < commSize; ++dest) {
        fSendToNM.emplace_back(mp1::rsend_init(s->fInterNodeComm, fTaskIDSendToNM[dest], dest));
    }
}

template<std::integral T>
auto ClusterAwareMasterWorkerDispatcher<T>::ClusterMaster::StartAll() -> void {
    mp1::start_all(fRecvFromNM);
}

template<std::integral T>
auto ClusterAwareMasterWorkerDispatcher<T>::ClusterMaster::operator()() -> void {
    auto interNodeTaskID{muc::ranges::reduce(fS->fInterNodeBatchSize, fS->fTask.first)};
    while (true) {
        const auto completed{mp1::wait_some(fRecvFromNM)};
        if (completed.empty()) {
            break;
        }
        for (auto&& [rank, status] : std::as_const(completed)) {
            const auto taskID{std::min(interNodeTaskID, fS->fTask.last)};
            if (taskID != fS->fTask.last) [[likely]] {
                // The node master's next request is a ready send: post fRecvFromNM before waiting.
                interNodeTaskID += fS->fInterNodeBatchSize[rank];
                fRecvFromNM[rank].start();
            }
            fSendToNM[rank].wait();
            fTaskIDSendToNM[rank] = taskID; // fTaskIDSendToNM[rank] can only be reused after fSendToNM[rank].wait() returns
            fSendToNM[rank].start();
        }
    }
    mp1::wait_all(fSendToNM);
}

template<std::integral T>
ClusterAwareMasterWorkerDispatcher<T>::NodeMaster::NodeMaster(ClusterAwareMasterWorkerDispatcher<T>* s) :
    fS{s},
    fClusterMaster{s->fInterNodeComm.rank() == 0 ? std::make_unique<ClusterMaster>(s) : nullptr},
    fClusterMasterThread{},
    fSendToCM{mp1::rsend_init(s->fInterNodeComm, mp1::empty_data, 0)},
    fTaskIDRecvFromCM{},
    fRecvFromCM{mp1::recv_init(s->fInterNodeComm, fTaskIDRecvFromCM, 0)},
    fRecvFromW{},
    fTaskIDSendToW{},
    fSendToW{} {
    const auto nodeSize{s->fIntraNodeComm.size()};
    fRecvFromW.reserve(nodeSize);
    for (int src{}; src < nodeSize; ++src) {
        fRecvFromW.emplace_back(mp1::recv_init(s->fIntraNodeComm, mp1::empty_data, src));
    }
    fTaskIDSendToW.resize(nodeSize);
    fSendToW.reserve(nodeSize);
    for (int dest{}; dest < nodeSize; ++dest) {
        fSendToW.emplace_back(mp1::rsend_init(s->fIntraNodeComm, fTaskIDSendToW[dest], dest));
    }
}

template<std::integral T>
auto ClusterAwareMasterWorkerDispatcher<T>::NodeMaster::StartAll() -> void {
    mp1::start_all(fRecvFromW);

    if (fClusterMaster) {
        fClusterMaster->StartAll();
        fClusterMasterThread = std::jthread{std::ref(*fClusterMaster)};
    }
    mp1::barrier(fS->fInterNodeComm);
}

template<std::integral T>
auto ClusterAwareMasterWorkerDispatcher<T>::NodeMaster::operator()() -> void {
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    const auto intraNodeFirstTaskID{std::reduce(
        fS->fInterNodeBatchSize.cbegin(), fS->fInterNodeBatchSize.cbegin() + mpiEnv.LocalNodeIdx(),
        fS->fTask.first)};
    auto intraNodeTaskID{intraNodeFirstTaskID + mpiEnv.LocalNode().size * fS->fIntraNodeBatchSize};
    auto intraNodeTaskEnd{intraNodeFirstTaskID + fS->fInterNodeBatchSize[mpiEnv.LocalNodeIdx()]};

    fRecvFromCM.start();
    fSendToCM.start();
    while (true) {
        const auto completed{mp1::wait_some(fRecvFromW)};
        if (completed.empty()) {
            break;
        }
        for (auto&& [rank, status] : std::as_const(completed)) {
            if (intraNodeTaskID == intraNodeTaskEnd and intraNodeTaskID != fS->fTask.last) {
                fSendToCM.wait();
                fRecvFromCM.wait();
                intraNodeTaskID = fTaskIDRecvFromCM;
                if (intraNodeTaskID != fS->fTask.last) {
                    intraNodeTaskEnd = intraNodeTaskID + fS->fInterNodeBatchSize[mpiEnv.LocalNodeIdx()];
                    fRecvFromCM.start();
                    fSendToCM.start();
                } else {
                    intraNodeTaskEnd = intraNodeTaskID;
                }
            }
            const auto taskID{std::min(intraNodeTaskID, fS->fTask.last)};
            if (taskID != fS->fTask.last) [[likely]] {
                // The worker's next request is a ready send: post fRecvFromW before waiting.
                intraNodeTaskID += fS->fIntraNodeBatchSize;
                fRecvFromW[rank].start();
            }
            fSendToW[rank].wait();
            fTaskIDSendToW[rank] = taskID; // fTaskIDSendToW[rank] can only be reused after fSendToW[rank].wait() returns
            fSendToW[rank].start();
        }
    }
    fSendToCM.wait();
    fRecvFromCM.wait();
    mp1::wait_all(fSendToW);

    if (fClusterMasterThread.joinable()) {
        fClusterMasterThread.join();
    }
}

template<std::integral T>
ClusterAwareMasterWorkerDispatcher<T>::ClusterAwareMasterWorkerDispatcher() :
    Dispatcher<T>{},
    fIntraNodeComm{},
    fInterNodeComm{},
    fIntraNodeBatchSize{},
    fInterNodeBatchSizeMultiple{},
    fInterNodeBatchSize{},
    fNodeMaster{},
    fNodeMasterThread{},
    fSendToNM{},
    fTaskIDRecvFromNM{},
    fRecvFromNM{},
    fIntraNodeTaskCounter{} {
    mp1::info commInfo;
    commInfo.set("mpi_assert_no_any_tag", "true");
    commInfo.set("mpi_assert_no_any_source", "true");
    commInfo.set("mpi_assert_exact_length", "true");
    commInfo.set("mpi_assert_allow_overtaking", "true");
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    fIntraNodeComm = mp1::communicator::dup_with_info(mpiEnv.IntraNodeComm(), commInfo);
    if (mpiEnv.InterNodeComm().is_valid()) {
        fInterNodeComm = mp1::communicator::dup_with_info(mpiEnv.InterNodeComm(), commInfo);
        fNodeMaster = std::make_unique<NodeMaster>(this);
    }
    fInterNodeBatchSize.resize(mpiEnv.ClusterSize());
    fSendToNM = mp1::rsend_init(fIntraNodeComm, mp1::empty_data, 0);
    fRecvFromNM = mp1::recv_init(fIntraNodeComm, fTaskIDRecvFromNM, 0);
}

template<std::integral T>
auto ClusterAwareMasterWorkerDispatcher<T>::PreLoopAction() -> void {
    const auto avgTaskPerProc{static_cast<long double>(this->NTask()) / mp1::comm_world().size()};
    fInterNodeBatchSizeMultiple = std::min(muc::lltrunc(avgTaskPerProc), fgMaxInterNodeBatchSizeMultiple);
    Ensures(fInterNodeBatchSizeMultiple >= 1);
    fIntraNodeBatchSize = std::max(1ll, std::llround(fgImbalancingFactor * avgTaskPerProc / fInterNodeBatchSizeMultiple));
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    std::ranges::transform(
        mpiEnv.NodeList(), fInterNodeBatchSize.begin(),
        [this](auto&& node) {
            return fInterNodeBatchSizeMultiple * node.size * fIntraNodeBatchSize;
        });

    const auto intraNodeFirstTaskID{std::reduce(
        fInterNodeBatchSize.cbegin(), fInterNodeBatchSize.cbegin() + mpiEnv.LocalNodeIdx(),
        this->fTask.first)};
    this->fExecutingTask = intraNodeFirstTaskID + fIntraNodeComm.rank() * fIntraNodeBatchSize;
    fIntraNodeTaskCounter = 0;

    if (fNodeMaster) {
        fNodeMaster->StartAll();
        fNodeMasterThread = std::jthread{std::ref(*fNodeMaster)};
    }
    mp1::barrier(fIntraNodeComm);
}

template<std::integral T>
auto ClusterAwareMasterWorkerDispatcher<T>::PreTaskAction() -> void {
    if (fIntraNodeTaskCounter == 0) {
        fRecvFromNM.start();
        fSendToNM.start();
    }
}

template<std::integral T>
auto ClusterAwareMasterWorkerDispatcher<T>::PostTaskAction() -> void {
    if (++fIntraNodeTaskCounter != fIntraNodeBatchSize) {
        ++this->fExecutingTask;
        return;
    }
    fSendToNM.wait();
    fRecvFromNM.wait();
    this->fExecutingTask = fTaskIDRecvFromNM;
    fIntraNodeTaskCounter = 0;
}

template<std::integral T>
auto ClusterAwareMasterWorkerDispatcher<T>::PostLoopAction() -> void {
    fSendToNM.wait();
    fRecvFromNM.wait();

    if (fNodeMasterThread.joinable()) {
        fNodeMasterThread.join();
    }
}

template<std::integral T>
auto ClusterAwareMasterWorkerDispatcher<T>::NExecutedTaskEstimation() const -> std::pair<bool, T> {
    return {this->fNLocalExecutedTask > 10 * fInterNodeBatchSizeMultiple * fIntraNodeBatchSize,
            this->fExecutingTask - this->fTask.first};
}

} // namespace Mustard::inline Execution
