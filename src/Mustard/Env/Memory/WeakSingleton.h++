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

#pragma once

#include "Mustard/Env/Memory/WeakSingletonified.h++"
#include "Mustard/Env/Memory/internal/WeakSingletonBase.h++"
#include "Mustard/Env/Memory/internal/WeakSingletonPool.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Utility/FunctionAttribute.h++"

#include "muc/mutex"
#include "muc/utility"

#include "fmt/core.h"

#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <typeinfo>

namespace Mustard::Env::Memory {

template<typename ADerived>
class PassiveSingleton;

template<typename ADerived>
class WeakSingleton : public internal::WeakSingletonBase {
    friend class PassiveSingleton<ADerived>;

protected:
    [[deprecated]] WeakSingleton();
    WeakSingleton(ADerived* self);
    ~WeakSingleton();

public:
    MUSTARD_ALWAYS_INLINE static auto NotInstantiated() -> bool { return Status() == Status::NotInstantiated; }
    MUSTARD_ALWAYS_INLINE static auto Available() -> bool { return Status() == Status::Available; }
    MUSTARD_ALWAYS_INLINE static auto Expired() -> bool { return Status() == Status::Expired; }
    MUSTARD_ALWAYS_INLINE static auto Instantiated() -> bool { return not NotInstantiated(); }

private:
    enum struct Status {
        NotInstantiated,
        Available,
        Expired
    };

    MUSTARD_ALWAYS_INLINE static auto Status() -> enum Status;
    MUSTARD_NOINLINE static auto LoadInstance() -> enum Status;

private:
    static std::shared_ptr<void*> fgInstance;
    static muc::spin_mutex fgSpinMutex;
};

} // namespace Mustard::Env::Memory

#include "Mustard/Env/Memory/WeakSingleton.inl"
