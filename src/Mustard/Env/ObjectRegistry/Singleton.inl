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

template<typename ADerived>
std::shared_ptr<void*> Singleton<ADerived>::fgInstancePtr{};
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
    *fgInstancePtr = nullptr;
}

template<typename ADerived>
MUSTARD_ALWAYS_INLINE auto Singleton<ADerived>::Instance() -> ADerived& {
    switch (Status()) {
    [[likely]] case Status::Available:
        return *static_cast<ADerived*>(*fgInstancePtr);
    [[unlikely]] case Status::Expired:
        Throw<std::runtime_error>(fmt::format("The instance of {} has been deleted",
                                              muc::try_demangle(typeid(ADerived).name())));
    }
    muc::unreachable();
}

template<typename ADerived>
MUSTARD_ALWAYS_INLINE auto Singleton<ADerived>::Status() -> enum Status {
    std::scoped_lock lock{fgSpinMutex};
    if (fgInstancePtr and *fgInstancePtr) [[likely]] {
        return Status::Available;
    }
    return LoadInstance();
}

template<typename ADerived>
MUSTARD_NOINLINE auto Singleton<ADerived>::LoadInstance() -> enum Status {
    std::scoped_lock lock{internal::SingletonPool::RecursiveMutex()};
    if (fgInstancePtr == nullptr) {
        auto& pool{internal::SingletonPool::Instance()};
        if (auto instancePtr{pool.Find<ADerived>()}) {
            fgInstancePtr = std::move(instancePtr);
        } else {
            fgInstancePtr = pool.Make<ADerived>();
        }
    }
    if (*fgInstancePtr) {
        return Status::Available;
    } else {
        return Status::Expired;
    }
}

} // namespace Mustard::Env::inline ObjectRegistry
