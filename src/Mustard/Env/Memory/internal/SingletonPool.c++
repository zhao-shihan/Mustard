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

#include "Mustard/Env/Memory/internal/SingletonPool.h++"

#include "gsl/gsl"

#include <algorithm>

namespace Mustard::Env::Memory::internal {

std::mutex SingletonPool::fgMutex{};

SingletonPool::SingletonPool() :
    PassiveSingleton<SingletonPool>{this} {}

SingletonPool::~SingletonPool() {
    for (auto&& [_1, instanceInfo] : std::as_const(fInstanceMap)) {
        auto&& [instance, _2, _3]{instanceInfo};
        Ensures(not instance.expired());
        Ensures(*instance.lock() == nullptr);
    }
}

[[nodiscard]] auto SingletonPool::GetUndeletedInReverseInsertionOrder() const -> std::vector<gsl::owner<const SingletonBase*>> {
    std::vector<std::pair<gsl::index, gsl::owner<const SingletonBase*>>> undeletedListWithID;
    undeletedListWithID.reserve(fInstanceMap.size());
    for (auto&& [type, instanceInfo] : fInstanceMap) {
        auto&& [instance, index, base]{instanceInfo};
        if (instance.expired()) {
            Throw<std::logic_error>(fmt::format("Instance pointer of {} expired", muc::try_demangle(type.name())));
        }
        if (*instance.lock() != nullptr) {
            undeletedListWithID.emplace_back(index, base);
        }
    }

    std::ranges::sort(undeletedListWithID,
                      [](const auto& lhs, const auto& rhs) {
                          return lhs.first > rhs.first;
                      });

    std::vector<gsl::owner<const SingletonBase*>> undeletedList;
    undeletedList.reserve(undeletedListWithID.size());
    for (auto&& [id, base] : std::as_const(undeletedListWithID)) {
        undeletedList.emplace_back(base);
    }
    return undeletedList;
}

} // namespace Mustard::Env::Memory::internal
