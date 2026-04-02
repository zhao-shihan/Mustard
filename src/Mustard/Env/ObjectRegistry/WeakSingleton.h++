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

#include "Mustard/Env/ObjectRegistry/WeakSingletonified.h++"
#include "Mustard/Env/ObjectRegistry/internal/WeakSingletonBase.h++"
#include "Mustard/Env/ObjectRegistry/internal/WeakSingletonPool.h++"
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

namespace Mustard::Env::inline ObjectRegistry {

template<typename ADerived>
class PassiveSingleton;

/// @brief Base mixin for externally owned singleton-like objects.
/// @tparam ADerived Final derived type.
/// @details The object registers itself into `internal::WeakSingletonPool` on
/// construction. Access is status-driven and does not force object creation.
/// @note Registration uses a shared indirection node from the global pool, so
/// code in different dynamic libraries can observe the same object pointer when
/// they are running in the same Mustard environment.
template<typename ADerived>
class WeakSingleton : public internal::WeakSingletonBase {
    friend class PassiveSingleton<ADerived>;

protected:
    /// @brief Registers @p self as the current weak singleton instance.
    /// @param self Non-null pointer to the derived object under construction.
    WeakSingleton(ADerived* self);
    /// @brief Marks the shared instance slot as expired.
    ~WeakSingleton();

public:
    /// @brief Checks whether the singleton has ever been instantiated.
    MUSTARD_ALWAYS_INLINE static auto Instantiated() -> bool { return not NotInstantiated(); }
    /// @brief Checks whether the singleton has ever been instantiated.
    MUSTARD_ALWAYS_INLINE static auto NotInstantiated() -> bool { return Status() == Status::NotInstantiated; }
    /// @brief Checks whether the singleton instance is currently alive.
    MUSTARD_ALWAYS_INLINE static auto Available() -> bool { return Status() == Status::Available; }
    /// @brief Checks whether the singleton was instantiated but already destroyed.
    MUSTARD_ALWAYS_INLINE static auto Expired() -> bool { return Status() == Status::Expired; }

private:
    enum struct Status {
        NotInstantiated,
        Available,
        Expired
    };

private:
    MUSTARD_ALWAYS_INLINE static auto Status() -> enum Status;
    MUSTARD_NOINLINE static auto LoadInstance() -> enum Status;

private:
    static std::shared_ptr<void*> fgInstancePtr;
    static muc::spin_mutex fgSpinMutex;
};

} // namespace Mustard::Env::inline ObjectRegistry

#include "Mustard/Env/ObjectRegistry/WeakSingleton.inl"
