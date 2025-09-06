// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

#include "Mustard/Parallel/MPIPredefined.h++"
#include "Mustard/Utility/NonCopyableBase.h++"

#include <concepts>
#include <utility>

namespace Mustard::inline Execution {

template<std::integral T>
    requires(Parallel::MPIPredefined<T> and sizeof(T) >= sizeof(short))
class Scheduler : public NonCopyableBase {
public:
    struct Task {
        T first;
        T last;
    };

public:
    virtual ~Scheduler() = default;

    auto Task() const -> auto { return fTask; }
    auto NTask() const -> auto { return fTask.last - fTask.first; }
    auto ExecutingTask() const -> auto { return fExecutingTask; }
    auto NLocalExecutedTask() const -> auto { return fNLocalExecutedTask; }

    auto Task(struct Task task) -> void { fTask = task; }
    auto Reset() -> void;
    auto IncrementNLocalExecutedTask() -> void { ++fNLocalExecutedTask; }

    virtual auto PreLoopAction() -> void = 0;
    virtual auto PreTaskAction() -> void = 0;
    virtual auto PostTaskAction() -> void = 0;
    virtual auto PostLoopAction() -> void = 0;

    virtual auto NExecutedTaskEstimation() const -> std::pair<bool, T> = 0;

protected:
    struct Task fTask;
    T fExecutingTask;
    T fNLocalExecutedTask;
};

} // namespace Mustard::inline Execution

#include "Mustard/Execution/Scheduler.inl"
