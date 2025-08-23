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

namespace Mustard::inline Execution {

template<std::integral T>
auto StaticScheduler<T>::PreLoopAction() -> void {
    const auto worldComm{mplr::comm_world()};
    this->fExecutingTask = this->fTask.first + (worldComm.size() - 1 - worldComm.rank());
    if (this->fExecutingTask > this->fTask.last) [[unlikely]] {
        this->fExecutingTask = this->fTask.last;
    }
}

template<std::integral T>
auto StaticScheduler<T>::PostTaskAction() -> void {
    this->fExecutingTask += mplr::comm_world().size();
    if (this->fExecutingTask > this->fTask.last) [[unlikely]] {
        this->fExecutingTask = this->fTask.last;
    }
}

template<std::integral T>
auto StaticScheduler<T>::NExecutedTaskEstimation() const -> std::pair<bool, T> {
    return {this->fNLocalExecutedTask > 10,
            this->fExecutingTask - this->fTask.first};
}

} // namespace Mustard::inline Execution
