// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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
muc::spin_mutex Singleton<ADerived>::fgSpinMutex{};

template<typename ADerived>
Singleton<ADerived>::Singleton() :
    SingletonBase{} {
    static_assert(Singletonified<ADerived>);
}

template<typename ADerived>
Singleton<ADerived>::~Singleton() {
    LoadInstance();
    *fgInstance = nullptr;
}

template<typename ADerived>
MUSTARD_ALWAYS_INLINE auto Singleton<ADerived>::Instance() -> ADerived& {
    switch (Status()) {
    [[likely]] case Status::Available:
        return *static_cast<ADerived*>(*fgInstance);
    [[unlikely]] case Status::Expired:
        Throw<std::logic_error>(fmt::format("The instance of {} has been deleted",
                                            muc::try_demangle(typeid(ADerived).name())));
    }
    muc::unreachable();
}

template<typename ADerived>
MUSTARD_ALWAYS_INLINE auto Singleton<ADerived>::Status() -> enum Status {
    std::scoped_lock lock{fgSpinMutex};
    if (fgInstance and *fgInstance) [[likely]] {
        return Status::Available;
    }
    return LoadInstance();
}

template<typename ADerived>
MUSTARD_NOINLINE auto Singleton<ADerived>::LoadInstance() -> enum Status {
    std::scoped_lock lock{internal::SingletonPool::RecursiveMutex()};
    if (fgInstance == nullptr) {
        if (const auto sharedNode{internal::SingletonPool::Instance().Find<ADerived>()}) {
            fgInstance = sharedNode;
        } else {
            auto& pool{internal::SingletonPool::Instance()};
            if (pool.Contains<ADerived>()) {
                Throw<std::logic_error>(fmt::format("Trying to construct {} (environmental singleton) twice",
                                                    muc::try_demangle(typeid(ADerived).name())));
            }
            fgInstance = pool.Insert<ADerived>(SingletonInstantiator::New<ADerived>());
        }
    }
    if (*fgInstance) {
        return Status::Available;
    } else {
        return Status::Expired;
    }
}

} // namespace Mustard::Env::Memory
