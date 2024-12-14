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

namespace Mustard::Env::Memory::internal {

template<Singletonified ASingleton>
[[nodiscard]] auto SingletonPool::Find() -> std::shared_ptr<void*> {
    if (const auto existed{std::as_const(fInstanceMap).find(typeid(ASingleton))};
        existed == fInstanceMap.cend()) {
        return {};
    } else {
        auto&& [type, instanceInfo]{*existed};
        auto&& instance{get<0>(instanceInfo)};
        if (instance.expired()) {
            Throw<std::logic_error>(fmt::format("Instance pointer of {} expired", type.name()));
        }
        return instance.lock();
    }
}

template<Singletonified ASingleton>
[[nodiscard]] auto SingletonPool::Insert(gsl::not_null<ASingleton*> instance) -> std::shared_ptr<void*> {
    const auto sharedInstance{std::make_shared<void*>(instance)};
    const auto [_, inserted]{fInstanceMap.try_emplace(typeid(ASingleton),
                                                      std::tuple{sharedInstance,
                                                                 fInstanceMap.size(),
                                                                 static_cast<SingletonBase*>(instance)})};
    if (not inserted) {
        Throw<std::logic_error>(fmt::format("Instance of type {} already exists", typeid(ASingleton).name()));
    }
    return sharedInstance;
}

} // namespace Mustard::Env::Memory::internal
