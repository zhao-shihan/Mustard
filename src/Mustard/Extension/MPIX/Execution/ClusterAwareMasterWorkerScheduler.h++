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

#pragma once

#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Extension/MPIX/Execution/Scheduler.h++"
#include "Mustard/Utility/NonMoveableBase.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "mpl/mpl.hpp"

#include "muc/math"
#include "muc/numeric"

#include "gsl/gsl"

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace Mustard::inline Extension::MPIX::inline Execution {

template<std::integral T>
class ClusterAwareMasterWorkerScheduler : public Scheduler<T> {
private:
    class MasterBase : public NonMoveableBase {
    protected:
        MasterBase(ClusterAwareMasterWorkerScheduler<T>* s);
        ~MasterBase() = default;

    protected:
        ClusterAwareMasterWorkerScheduler<T>* fS;
    };

    class ClusterMaster : public MasterBase {
    public:
        ClusterMaster(ClusterAwareMasterWorkerScheduler<T>* s);

        auto StartAll() -> void;
        auto operator()() -> void;

    private:
        std::byte fSemaphoreRecvFromNM;
        mpl::prequest_pool fRecvFromNM;
        std::vector<T> fTaskIDSendToNM;
        mpl::prequest_pool fSendToNM;
    };
    friend class ClusterMaster;

    class NodeMaster : public MasterBase {
    public:
        NodeMaster(ClusterAwareMasterWorkerScheduler<T>* s);

        auto StartAll() -> void;
        auto operator()() -> void;

    private:
        std::unique_ptr<ClusterMaster> fClusterMaster;
        std::jthread fClusterMasterThread;

        std::byte fSemaphoreSendToCM;
        mpl::prequest fSendToCM;
        T fTaskIDRecvFromCM;
        mpl::prequest fRecvFromCM;

        std::byte fSemaphoreRecvFromW;
        mpl::prequest_pool fRecvFromW;
        std::vector<T> fTaskIDSendToW;
        mpl::prequest_pool fSendToW;
    };
    friend class NodeMaster;

public:
    ClusterAwareMasterWorkerScheduler();

private:
    virtual auto PreLoopAction() -> void override;
    virtual auto PreTaskAction() -> void override;
    virtual auto PostTaskAction() -> void override;
    virtual auto PostLoopAction() -> void override;

    virtual auto NExecutedTaskEstimation() const -> std::pair<bool, T> override;

private:
    mpl::communicator fIntraNodeComm;
    mpl::communicator fInterNodeComm;

    T fIntraNodeBatchSize;
    T fInterNodeBatchSizeMultiplicity;
    std::vector<T> fInterNodeBatchSize;

    std::unique_ptr<NodeMaster> fNodeMaster;
    std::jthread fNodeMasterThread;

    std::byte fSemaphoreSendToNM;
    mpl::prequest fSendToNM;
    T fTaskIDRecvFromNM;
    mpl::prequest fRecvFromNM;

    T fIntraNodeTaskCounter;

    static constexpr long double fgImbalancingFactor{1e-3};
    static constexpr long long fgMaxInterNodeBatchSizeMultiplicity{10};
};

} // namespace Mustard::inline Extension::MPIX::inline Execution

#include "Mustard/Extension/MPIX/Execution/ClusterAwareMasterWorkerScheduler.inl"
