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

#include "Mustard/Env/Memory/internal/SingletonPool.h++"

#include <algorithm>
#include <cassert>

namespace Mustard::Env::Memory::internal {

SingletonPool::~SingletonPool() {
    for ([[maybe_unused]] auto&& [_, instanceInfo] : std::as_const(fInstanceMap)) {
        [[maybe_unused]] auto&& [instance, __, ___]{instanceInfo};
        assert(not instance.expired());
        assert(*instance.lock() == nullptr);
    }
}

[[nodiscard]] auto SingletonPool::GetUndeletedInReverseInsertionOrder() const -> std::vector<gsl::owner<const SingletonBase*>> {
    std::vector<std::pair<gsl::index, gsl::owner<const SingletonBase*>>> undeletedListWithID;
    undeletedListWithID.reserve(fInstanceMap.size());
    for (auto&& [type, instanceInfo] : fInstanceMap) {
        auto&& [instance, index, base]{instanceInfo};
        if (instance.expired()) {
            Throw<std::logic_error>(fmt::format("Instance pointer of {} expired", type.name()));
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
