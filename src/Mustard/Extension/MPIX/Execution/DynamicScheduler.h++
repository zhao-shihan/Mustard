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
#include "Mustard/Extension/MPIX/DataType.h++"
#include "Mustard/Extension/MPIX/Execution/Scheduler.h++"
#include "Mustard/Utility/NonMoveableBase.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "mpi.h"

#include "muc/utility"

#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <variant>
#include <vector>

namespace Mustard::inline Extension::MPIX::inline Execution {

template<std::integral T>
class DynamicScheduler : public Scheduler<T> {
public:
    DynamicScheduler();

private:
    virtual auto PreLoopAction() -> void override;
    virtual auto PreTaskAction() -> void override;
    virtual auto PostTaskAction() -> void override;
    virtual auto PostLoopAction() -> void override;

    virtual auto NExecutedTask() const -> std::pair<bool, T> override;

private:
    class Comm final {
    public:
        Comm();
        ~Comm();

        operator MPI_Comm() const { return fComm; }
        auto Rank() const -> int { return fRank; }
        auto Size() const -> int { return fSize; }

    private:
        const MPI_Comm fComm;
        const int fRank;
        const int fSize;
    };

    struct Dummy final : std::monostate {
        auto PreLoopAction() -> void { muc::unreachable(); }
        auto PreTaskAction() -> void { muc::unreachable(); }
        auto PostTaskAction() -> void { muc::unreachable(); }
        auto PostLoopAction() -> void { muc::unreachable(); }
    };

    class Master final : public NonMoveableBase {
    public:
        Master(DynamicScheduler<T>* ds);

        auto PreLoopAction() -> void;
        auto PreTaskAction() -> void {}
        auto PostTaskAction() -> void;
        auto PostLoopAction() -> void {}

    private:
        class Supervisor final : public NonMoveableBase {
        public:
            Supervisor(DynamicScheduler<T>* ds);
            ~Supervisor();

            auto FetchAddTaskID() -> T;

            auto Start() -> void;

        private:
            DynamicScheduler<T>* const fDS;
            std::atomic<T> fMainTaskID;
            std::vector<MPI_Request> fRecv;
            std::vector<T> fTaskIDSend;
            std::vector<MPI_Request> fSend;
            std::jthread fSupervisorThread;
        };

    private:
        DynamicScheduler<T>* fDS;
        Supervisor fSupervisor;
        T fBatchCounter;
    };
    friend class Master;

    class Worker final : public NonMoveableBase {
    public:
        Worker(DynamicScheduler<T>* ds);
        ~Worker();

        auto PreLoopAction() -> void;
        auto PreTaskAction() -> void;
        auto PostTaskAction() -> void;
        auto PostLoopAction() -> void;

    private:
        DynamicScheduler<T>* fDS;
        T fTaskIDRecv;
        std::array<MPI_Request, 2> fRequest;
        T fBatchCounter;
    };
    friend class Worker;

private:
    Comm fComm;
    T fBatchSize;
    std::variant<Dummy, Master, Worker> fContext;

    static constexpr auto fgBalancingFactor{0.001};
};

} // namespace Mustard::inline Extension::MPIX::inline Execution

#include "Mustard/Extension/MPIX/Execution/DynamicScheduler.inl"
