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

#include "Mustard/Concept/MPIPredefined.h++"
#include "Mustard/Utility/NonMoveableBase.h++"

#include <concepts>
#include <utility>

namespace Mustard::inline Extension::MPIX::inline Execution {

template<std::integral T>
    requires(Concept::MPIPredefined<T> and sizeof(T) >= sizeof(short))
class Executor;

template<std::integral T>
    requires(Concept::MPIPredefined<T> and sizeof(T) >= sizeof(short))
class Scheduler : public NonMoveableBase {
    friend class Executor<T>;

public:
    virtual ~Scheduler() = default;

protected:
    auto NTask() const -> T { return fTask.last - fTask.first; }

private:
    auto Reset() -> void;

    virtual auto PreLoopAction() -> void = 0;
    virtual auto PreTaskAction() -> void = 0;
    virtual auto PostTaskAction() -> void = 0;
    virtual auto PostLoopAction() -> void = 0;

    virtual auto NExecutedTask() const -> std::pair<bool, T> = 0;

protected:
    struct Task {
        T first;
        T last;
    };

protected:
    Task fTask;
    T fExecutingTask;
    T fNLocalExecutedTask;
};

} // namespace Mustard::inline Extension::MPIX::inline Execution

#include "Mustard/Extension/MPIX/Execution/Scheduler.inl"
