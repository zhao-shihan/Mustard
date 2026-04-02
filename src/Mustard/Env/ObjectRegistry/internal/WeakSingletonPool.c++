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

#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/Env/ObjectRegistry/internal/WeakSingletonPool.h++"

#include "fmt/core.h"

#include <utility>

namespace Mustard::Env::inline ObjectRegistry::internal {

WeakSingletonPool* WeakSingletonPool::fgInstance{};
bool WeakSingletonPool::fgInstantiated{};
bool WeakSingletonPool::fgExpired{};
std::recursive_mutex WeakSingletonPool::fgRecursiveMutex{};

WeakSingletonPool::WeakSingletonPool() :
    NonCopyableBase{},
    fInstanceMap{} {
    if (fgInstantiated) {
        Throw<std::runtime_error>("Trying to instantiate the pool twice");
    }
    fgInstance = this;
    fgInstantiated = true;
}

WeakSingletonPool::~WeakSingletonPool() {
    for (auto&& [type, instance] : std::as_const(fInstanceMap)) {
        if (instance.expired()) {
            PrintError(fmt::format("Instance pointer of {} expired", muc::try_demangle(type.name())));
        }
        if (*instance.lock() != nullptr) [[unlikely]] {
            PrintError(fmt::format("Instance of {} survives, implies memory leak or following undefined behavior",
                                   muc::try_demangle(type.name())));
        }
    }
    fgInstance = nullptr;
    fgExpired = true;
}

auto WeakSingletonPool::Instance() -> WeakSingletonPool& {
    if (fgInstance == nullptr) {
        Throw<std::runtime_error>("The pool has not been instantiated or has been destructed "
                                  "(maybe you forgot to instantiate an environment?)");
    }
    return *fgInstance;
}

} // namespace Mustard::Env::inline ObjectRegistry::internal
