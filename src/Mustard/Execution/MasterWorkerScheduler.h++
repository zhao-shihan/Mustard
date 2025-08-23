// -*- C++ -*-
//
// Copyright 2020-2025  The Mustard development team
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

#include "Mustard/Execution/Scheduler.h++"
#include "Mustard/IO/PrettyLog.h++"

#include "mplr/mplr.hpp"

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

namespace Mustard::inline Execution {

template<std::integral T>
class MasterWorkerScheduler : public Scheduler<T> {
private:
    friend class Master;
    class Master {
    public:
        Master(MasterWorkerScheduler<T>* s);

        auto StartAll() -> void;
        auto operator()() -> void;

    private:
        MasterWorkerScheduler<T>* fS;
        std::byte fSemaphoreRecv;
        mplr::prequest_pool fRecv;
        std::vector<T> fTaskIDSend;
        mplr::prequest_pool fSend;
    };

public:
    MasterWorkerScheduler();

    virtual auto PreLoopAction() -> void override;
    virtual auto PreTaskAction() -> void override;
    virtual auto PostTaskAction() -> void override;
    virtual auto PostLoopAction() -> void override;

    virtual auto NExecutedTaskEstimation() const -> std::pair<bool, T> override;

private:
    mplr::communicator fComm;
    T fBatchSize;
    std::unique_ptr<Master> fMaster;
    std::jthread fMasterThread;

    std::byte fSemaphoreSend;
    mplr::prequest fSend;
    T fTaskIDRecv;
    mplr::prequest fRecv;
    T fTaskCounter;

    static constexpr long double fgImbalancingFactor{1e-3};
};

} // namespace Mustard::inline Execution

#include "Mustard/Execution/MasterWorkerScheduler.inl"
