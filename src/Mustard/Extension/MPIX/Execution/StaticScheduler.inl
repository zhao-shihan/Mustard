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

namespace Mustard::inline Extension::MPIX::inline Execution {

template<std::integral T>
auto StaticScheduler<T>::PreLoopAction() -> void {
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    this->fExecutingTask = this->fTask.first + (mpiEnv.CommWorldSize() - 1 - mpiEnv.CommWorldRank());
    if (this->fExecutingTask > this->fTask.last) [[unlikely]] {
        this->fExecutingTask = this->fTask.last;
    }
}

template<std::integral T>
auto StaticScheduler<T>::PostTaskAction() -> void {
    this->fExecutingTask += Env::MPIEnv::Instance().CommWorldSize();
    if (this->fExecutingTask > this->fTask.last) [[unlikely]] {
        this->fExecutingTask = this->fTask.last;
    }
}

template<std::integral T>
auto StaticScheduler<T>::NExecutedTask() const -> std::pair<bool, T> {
    return {this->fNLocalExecutedTask > 10,
            this->fExecutingTask - this->fTask.first};
}

} // namespace Mustard::inline Extension::MPIX::inline Execution
