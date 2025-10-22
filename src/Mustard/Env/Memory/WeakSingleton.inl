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

namespace Mustard::Env::Memory {

template<typename ADerived>
std::shared_ptr<void*> WeakSingleton<ADerived>::fgInstance{};
template<typename ADerived>
muc::spin_mutex WeakSingleton<ADerived>::fgSpinMutex{};

template<typename ADerived>
WeakSingleton<ADerived>::WeakSingleton(ADerived* self) :
    WeakSingletonBase{} {
    static_assert(WeakSingletonified<ADerived>);
    std::scoped_lock lock{internal::WeakSingletonPool::RecursiveMutex()};
    if (auto& weakSingletonPool{internal::WeakSingletonPool::Instance()};
        not weakSingletonPool.Contains<ADerived>()) {
        fgInstance = weakSingletonPool.Insert<ADerived>(self);
    } else {
        Throw<std::logic_error>(fmt::format("Trying to construct {} (weak singleton in environment) twice",
                                            muc::try_demangle(typeid(ADerived).name())));
    }
}

template<typename ADerived>
WeakSingleton<ADerived>::~WeakSingleton() {
    LoadInstance();
    *fgInstance = nullptr;
}

template<typename ADerived>
MUSTARD_ALWAYS_INLINE auto WeakSingleton<ADerived>::Status() -> enum Status {
    std::scoped_lock lock{fgSpinMutex};
    if (fgInstance and *fgInstance) [[likely]] {
        return Status::Available;
    }
    return LoadInstance();
}

template<typename ADerived>
MUSTARD_NOINLINE auto WeakSingleton<ADerived>::LoadInstance() -> enum Status {
    std::scoped_lock lock{internal::WeakSingletonPool::RecursiveMutex()};
    if (fgInstance == nullptr) {
        if (const auto sharedNode{internal::WeakSingletonPool::Instance().Find<ADerived>()}) {
            fgInstance = sharedNode;
        } else {
            return Status::NotInstantiated;
        }
    }
    if (*fgInstance == nullptr) {
        return Status::Expired;
    } else {
        return Status::Available;
    }
}

} // namespace Mustard::Env::Memory
