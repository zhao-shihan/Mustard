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

#pragma once

#include "Mustard/Env/Memory/PassiveSingletonified.h++"
#include "Mustard/Env/Memory/WeakSingleton.h++"
#include "Mustard/Env/Memory/internal/PassiveSingletonBase.h++"
#include "Mustard/Env/Memory/internal/WeakSingletonPool.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "muc/utility"

#include "fmt/format.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>

namespace Mustard::Env::Memory {

template<typename ADerived>
class PassiveSingleton : public internal::PassiveSingletonBase,
                         public WeakSingleton<ADerived> {
private:
    using Base = WeakSingleton<ADerived>;

protected:
    [[deprecated]] PassiveSingleton() = default;
    PassiveSingleton(ADerived* self);
    ~PassiveSingleton() = default;

public:
    MUSTARD_ALWAYS_INLINE static auto Instance() -> ADerived&;
};

} // namespace Mustard::Env::Memory

#include "Mustard/Env/Memory/PassiveSingleton.inl"
