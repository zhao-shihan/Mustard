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

#include "Mustard/Extension/MPIX/Execution/Scheduler.h++"
#include "Mustard/Utility/NonMoveableBase.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "mpl/mpl.hpp"

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace Mustard::inline Extension::MPIX::inline Execution {

template<std::integral T>
class MasterWorkerScheduler : public Scheduler<T> {
private:
    class Master : public NonMoveableBase {
    public:
        Master(MasterWorkerScheduler<T>* s);

        auto StartAll() -> void { fRecv.startall(); }
        auto operator()() -> void;

    private:
        MasterWorkerScheduler<T>* fS;
        std::byte fSemaphoreRecv;
        mpl::prequest_pool fRecv;
        std::vector<T> fTaskIDSend;
        mpl::prequest_pool fSend;
    };
    friend class Master;

public:
    MasterWorkerScheduler();

private:
    virtual auto PreLoopAction() -> void override;
    virtual auto PreTaskAction() -> void override;
    virtual auto PostTaskAction() -> void override;
    virtual auto PostLoopAction() -> void override;

    virtual auto NExecutedTaskEstimation() const -> std::pair<bool, T> override;

private:
    mpl::communicator fComm;
    T fBatchSize;
    std::unique_ptr<Master> fMaster;
    std::jthread fMasterThread;

    std::byte fSemaphoreSend;
    mpl::prequest fSend;
    T fTaskIDRecv;
    mpl::prequest fRecv;
    T fTaskCounter;

    static constexpr long double fgImbalancingFactor{1e-3};
};

} // namespace Mustard::inline Extension::MPIX::inline Execution

#include "Mustard/Extension/MPIX/Execution/MasterWorkerScheduler.inl"
