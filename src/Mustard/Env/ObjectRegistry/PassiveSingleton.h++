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

#pragma once

#include "Mustard/Env/ObjectRegistry/PassiveSingletonified.h++"
#include "Mustard/Env/ObjectRegistry/WeakSingleton.h++"
#include "Mustard/Env/ObjectRegistry/internal/PassiveSingletonBase.h++"
#include "Mustard/Env/ObjectRegistry/internal/WeakSingletonPool.h++"
#include "Mustard/IO/PrettyLog.h++"

#include "muc/utility"

#include "fmt/format.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>

namespace Mustard::Env::inline ObjectRegistry {

/// @brief Base mixin for passive singletons.
/// @tparam ADerived Final derived type.
/// @note Registration uses a shared indirection node from the global pool, so
/// code in different dynamic libraries can observe the same object pointer when
/// they are running in the same Mustard environment.
template<typename ADerived>
class PassiveSingleton : public internal::PassiveSingletonBase,
                         public WeakSingleton<ADerived> {
private:
    using Base = WeakSingleton<ADerived>;

protected:
    /// @brief Registers @p self into ObjectRegistry.
    /// @param self Non-null pointer to the derived object under construction.
    PassiveSingleton(ADerived* self);
    /// @brief Defaulted destructor.
    ~PassiveSingleton() = default;

public:
    /// @brief Returns the registered instance.
    /// @return Reference to the managed instance.
    /// @throw std::runtime_error If the instance is not yet created or expired.
    MUSTARD_ALWAYS_INLINE static auto Instance() -> ADerived&;
};

} // namespace Mustard::Env::inline ObjectRegistry

#include "Mustard/Env/ObjectRegistry/PassiveSingleton.inl"
