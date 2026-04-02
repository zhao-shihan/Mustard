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

namespace Mustard::Env::inline ObjectRegistry {

namespace internal {

template<Singletonified ASingleton>
[[nodiscard]] auto SingletonPool::Find() const -> std::shared_ptr<void*> {
    const auto existed{fInstanceMap.find(typeid(ASingleton))};
    if (existed == fInstanceMap.cend()) {
        return {};
    }
    const auto& [type, info]{*existed};
    if (info.instancePtr.expired()) {
        Throw<std::runtime_error>(fmt::format("Instance pointer of {} expired", muc::try_demangle(type.name())));
    }
    return info.instancePtr.lock();
}

template<Singletonified ASingleton>
[[nodiscard]] auto SingletonPool::Make() -> std::shared_ptr<void*> {
    auto [instance, instancePtr]{SingletonFactory::Make<ASingleton>()};
    auto dispose{[instance] { SingletonFactory::Dispose(instance); }};
    const auto [_, inserted]{fInstanceMap.try_emplace(
        typeid(ASingleton), InstanceInfo{instancePtr, ssize(fInstanceMap), std::move(dispose)})};
    if (not inserted) {
        Throw<std::runtime_error>(fmt::format("Instance of type {} already exists", muc::try_demangle(typeid(ASingleton).name())));
    }
    return instancePtr;
}

} // namespace internal

template<Singletonified ASingleton>
auto SingletonFactory::Make() -> std::pair<gsl::owner<ASingleton*>, std::shared_ptr<void*>> {
    // Do not swap allocation/placement order -- allocate instance and
    // instance pointer closely for better cache locality
    void* instance{Allocator<ASingleton>{}.allocate(1)};
    auto instancePtr{std::allocate_shared<void*>(Allocator<void*>{}, instance)};
    return {::new (instance) ASingleton, instancePtr};
}

template<Singletonified ASingleton>
auto SingletonFactory::Dispose(gsl::owner<ASingleton*> instance) noexcept -> void {
    instance->~ASingleton();
    Allocator<ASingleton>{}.deallocate(instance, 1);
}

} // namespace Mustard::Env::inline ObjectRegistry
