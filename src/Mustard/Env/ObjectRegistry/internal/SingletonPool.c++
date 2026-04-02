// -*- C++ -*-
//
// Copyright (C) 2020-2025  Mustard developers
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

#include "Mustard/Env/ObjectRegistry/internal/SingletonPool.h++"

#include <queue>
#include <ranges>

namespace Mustard::Env::inline ObjectRegistry::internal {

std::recursive_mutex SingletonPool::fgRecursiveMutex{};

SingletonPool::SingletonPool() :
    PassiveSingleton<SingletonPool>{this},
    fInstanceMap{} {}

SingletonPool::~SingletonPool() {
    std::priority_queue<std::pair<gsl::index, std::function<auto()->void>*>> executionList;
    for (auto&& [type, instanceInfo] : fInstanceMap) {
        auto& [instance, order, dispose]{instanceInfo};
        if (instance.expired()) {
            Throw<std::runtime_error>(fmt::format("Instance pointer of {} expired", muc::try_demangle(type.name())));
        }
        if (*instance.lock()) { // if the instance was not deleted externally
            executionList.emplace(order, &dispose);
        }
    }
    // Delete instances in the reverse order of their creation
    for (; not executionList.empty(); executionList.pop()) {
        std::invoke(*executionList.top().second);
    }
}

} // namespace Mustard::Env::inline ObjectRegistry::internal
