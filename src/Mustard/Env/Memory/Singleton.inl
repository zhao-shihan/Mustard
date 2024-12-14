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

namespace Mustard::Env::Memory {

template<typename ADerived>
std::shared_ptr<void*> Singleton<ADerived>::fgInstance{};

template<typename ADerived>
Singleton<ADerived>::Singleton() :
    SingletonBase{} {
    static_assert(Singletonified<ADerived>);
}

template<typename ADerived>
Singleton<ADerived>::~Singleton() {
    UpdateInstance();
    *fgInstance = nullptr;
}

template<typename ADerived>
MUSTARD_ALWAYS_INLINE auto Singleton<ADerived>::Instance() -> ADerived& {
    switch (UpdateInstance()) {
    [[unlikely]] case Status::NotInstantiated: {
        auto& pool{internal::SingletonPool::Instance()};
        if (pool.Contains<ADerived>()) {
            Throw<std::logic_error>(fmt::format("Trying to construct {} (environmental singleton) twice",
                                                typeid(ADerived).name()));
        }
        fgInstance = pool.Insert<ADerived>(SingletonInstantiator::New<ADerived>());
        [[fallthrough]];
    }
    [[likely]] case Status::Available:
        return *static_cast<ADerived*>(*fgInstance);
    [[unlikely]] case Status::Expired:
        Throw<std::logic_error>(fmt::format("The instance of {} has been deleted",
                                            typeid(ADerived).name()));
    }
    muc::unreachable();
}

template<typename ADerived>
MUSTARD_ALWAYS_INLINE auto Singleton<ADerived>::UpdateInstance() -> Status {
    if (fgInstance == nullptr) [[unlikely]] {
        if (const auto sharedNode{internal::SingletonPool::Instance().Find<ADerived>()};
            sharedNode == nullptr) {
            return Status::NotInstantiated;
        } else {
            fgInstance = sharedNode;
        }
    }
    if (*fgInstance == nullptr) {
        return Status::Expired;
    } else {
        return Status::Available;
    }
}

} // namespace Mustard::Env::Memory
