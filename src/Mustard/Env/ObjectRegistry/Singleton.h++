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

#include "Mustard/Env/ObjectRegistry/Singletonified.h++"
#include "Mustard/Env/ObjectRegistry/internal/SingletonBase.h++"
#include "Mustard/Env/ObjectRegistry/internal/SingletonPool.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Utility/FunctionAttribute.h++"
#include "Mustard/Utility/NonConstructibleBase.h++"

#include "muc/mutex"
#include "muc/utility"

#include "fmt/core.h"

#include <memory>
#include <mutex>
#include <stdexcept>
#include <typeinfo>

namespace Mustard::Env::inline ObjectRegistry {

/// @brief Base mixin for lazily created environment singletons.
/// @tparam ADerived Final derived singleton type.
/// @details The first call to Instance() resolves the instance through
/// `internal::SingletonPool`, creating it on demand when needed.
/// @note Derived types must declare Mustard::Env::SingletonFactory as a friend
/// to allow construction and destruction.
/// Registration uses a shared indirection node from the global pool, so
/// code in different dynamic libraries can observe the same object pointer when
/// they are running in the same Mustard environment.
/// @warning Singleton uses a special allocator to allocate instances,
/// and this base class inherits matching ClassSpecificAllocation to
/// allow usual delete expression on singleton instances. Do not implement
/// any allocation/deallocation function in derived classes, otherwise
/// memory could corrupt.
template<typename ADerived>
class Singleton : public internal::SingletonBase {
protected:
    /// @brief Protected constructor for CRTP inheritance.
    Singleton();
    /// @brief Marks the shared instance as expired.
    ~Singleton();

public:
    /// @brief Returns the singleton instance, constructing it on first use.
    /// @return Reference to the singleton instance.
    /// @throw std::runtime_error If the singleton has been destroyed.
    MUSTARD_ALWAYS_INLINE static auto Instance() -> ADerived&;
    /// @brief Ensures the singleton is constructed.
    MUSTARD_ALWAYS_INLINE static auto EnsureInstantiation() -> void { Instance(); }

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

#include "Mustard/Env/ObjectRegistry/Singleton.inl"
