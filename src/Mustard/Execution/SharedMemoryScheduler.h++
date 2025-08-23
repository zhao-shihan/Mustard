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
#include "Mustard/Parallel/MPIDataType.h++"

#include "mpi.h"

#include "gsl/gsl"

#include <algorithm>
#include <cmath>
#include <concepts>
#include <stdexcept>
#include <string>
#include <utility>

namespace Mustard::inline Execution {

template<std::integral T>
class SharedMemoryScheduler : public Scheduler<T> {
public:
    SharedMemoryScheduler();
    ~SharedMemoryScheduler();

    virtual auto PreLoopAction() -> void override;
    virtual auto PreTaskAction() -> void override {}
    virtual auto PostTaskAction() -> void override;
    virtual auto PostLoopAction() -> void override {}

    virtual auto NExecutedTaskEstimation() const -> std::pair<bool, T> override;

private:
    volatile T* fMainTaskID;
    MPI_Win fMainTaskIDWindow;
    T fBatchSize;
    T fTaskCounter;

    static constexpr long double fgImbalancingFactor{1e-4};
};

} // namespace Mustard::inline Execution

#include "Mustard/Execution/SharedMemoryScheduler.inl"
