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

#include "Mustard/Extension/MPIX/DataType.h++"
#include "Mustard/Extension/MPIX/Execution/Scheduler.h++"

#include "mpl/mpl.hpp"

#include "mpi.h"

#include <algorithm>
#include <concepts>
#include <string>
#include <utility>

namespace Mustard::inline Extension::MPIX::inline Execution {

template<std::integral T>
class TaskQueueScheduler : public Scheduler<T> {
public:
    TaskQueueScheduler();
    ~TaskQueueScheduler();

private:
    virtual auto PreLoopAction() -> void override;
    virtual auto PreTaskAction() -> void override {}
    virtual auto PostTaskAction() -> void override;
    virtual auto PostLoopAction() -> void override;

    virtual auto NExecutedTaskEstimation() const -> std::pair<bool, T> override;

private:
    T fMainTaskID;
    MPI_Info fMainTaskIDWindowInfo;
    MPI_Win fMainTaskIDWindow;
    T fBatchSize;
    T fBatchCounter;

    static constexpr auto fgBalancingFactor{0.001};
};

} // namespace Mustard::inline Extension::MPIX::inline Execution

#include "Mustard/Extension/MPIX/Execution/TaskQueueScheduler.inl"
