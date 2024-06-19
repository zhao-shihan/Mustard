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

#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/Env/Memory/internal/WeakSingletonPool.h++"
#include "Mustard/Env/Print.h++"

#include <utility>

namespace Mustard::Env::Memory::internal {

WeakSingletonPool* WeakSingletonPool::fgInstance = nullptr;

WeakSingletonPool::WeakSingletonPool() :
    NonMoveableBase{},
    fInstanceMap{} {
    if (fgInstance == nullptr) {
        fgInstance = this;
    } else {
        throw std::logic_error{"Mustard::Env::Memory::internal::WeakSingletonPool::WeakSingletonPool(): "
                               "Trying to instantiate the pool twice"};
    }
}

WeakSingletonPool::~WeakSingletonPool() {
    for (auto&& [type, instance] : std::as_const(fInstanceMap)) {
        if (instance.expired()) {
            PrintLnError("Mustard::Env::Memory::internal::WeakSingletonPool::~WeakSingletonPool(): "
                         "Instance pointer of {} expired",
                         type.name());
        }
        if (*instance.lock() != nullptr) [[unlikely]] {
            PrintLnError("Mustard::Env::Memory::internal::WeakSingletonPool::~WeakSingletonPool(): "
                         "Instance of {} survives, "
                         "implies memory leak or following undefined behavior",
                         type.name());
        }
    }
    fgInstance = nullptr;
}

auto WeakSingletonPool::Instance() -> WeakSingletonPool& {
    if (fgInstance != nullptr) {
        return *fgInstance;
    } else {
        throw std::logic_error{"Mustard::Env::Memory::internal::WeakSingletonPool::Instance(): "
                               "The pool has not been instantiated or has been destructed "
                               "(maybe you forgot to instantiate an environment?)"};
    }
}

} // namespace Mustard::Env::Memory::internal
