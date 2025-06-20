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
#include "Mustard/Utility/PrettyLog.h++"

#include "mpl/mpl.hpp"

#include "mpi.h"

#include "gsl/gsl"

#include <algorithm>
#include <concepts>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace Mustard::inline Extension::MPIX::inline Execution {

template<std::integral T>
class SharedMemoryScheduler : public Scheduler<T> {
public:
    SharedMemoryScheduler();

private:
    virtual auto PreLoopAction() -> void override;
    virtual auto PreTaskAction() -> void override {}
    virtual auto PostTaskAction() -> void override;
    virtual auto PostLoopAction() -> void override;

    virtual auto NExecutedTaskEstimation() const -> std::pair<bool, T> override;

private:
    struct MPIWinDeleter;

private:
    T* fMainTaskID;
    std::unique_ptr<MPI_Win, MPIWinDeleter> fMainTaskIDWindow;
    T fBatchSize;
    T fBatchCounter;

    static constexpr auto fgImbalancingFactor{1e-4};
};

} // namespace Mustard::inline Extension::MPIX::inline Execution

#include "Mustard/Extension/MPIX/Execution/SharedMemoryScheduler.inl"
