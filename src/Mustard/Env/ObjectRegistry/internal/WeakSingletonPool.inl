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

namespace Mustard::Env::inline ObjectRegistry::internal {

template<WeakSingletonified AWeakSingleton>
[[nodiscard]] auto WeakSingletonPool::Find() const -> std::shared_ptr<void*> {
    const auto existed{fInstanceMap.find(typeid(AWeakSingleton))};
    if (existed == fInstanceMap.cend()) {
        return {};
    }
    const auto& [type, instancePtr]{*existed};
    if (instancePtr.expired()) {
        Throw<std::runtime_error>(fmt::format("Instance pointer of {} expired", muc::try_demangle(type.name())));
    }
    return instancePtr.lock();
}

template<WeakSingletonified AWeakSingleton>
[[nodiscard]] auto WeakSingletonPool::Insert(gsl::not_null<AWeakSingleton*> instance) -> std::shared_ptr<void*> {
    auto instancePtr{std::allocate_shared<void*>(Allocator<void*>{}, instance)};
    const auto [_, inserted]{fInstanceMap.try_emplace(typeid(AWeakSingleton), instancePtr)};
    if (not inserted) {
        Throw<std::runtime_error>(fmt::format("Instance of type {} already exists", muc::try_demangle(typeid(AWeakSingleton).name())));
    }
    return instancePtr;
}

} // namespace Mustard::Env::inline ObjectRegistry::internal
