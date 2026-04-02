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
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Memory/Allocator.h++"
#include "Mustard/Utility/NonCopyableBase.h++"

#include "gtl/phmap.hpp"

#include "muc/utility"

#include "gsl/gsl"

#include "fmt/format.h"

#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <typeinfo>

namespace Mustard::Env::inline ObjectRegistry::internal {

/// @brief Implementation detail of Mustard::Env::WeakSingleton.
/// @details Not API. Stores weak singleton indirection nodes keyed by dynamic type.
/// @note This shared indirection makes weak singleton lookups converge across
/// dynamic library boundaries when all modules use the same environment.
class WeakSingletonPool final : public NonCopyableBase {
public:
    /// @brief Constructs an empty weak singleton pool.
    WeakSingletonPool();
    /// @brief Validates tracked entries and marks the pool expired.
    ~WeakSingletonPool();

    /// @brief Checks whether the pool has ever been constructed.
    static auto Instantiated() -> bool { return fgInstantiated; }
    /// @brief Checks whether the global pool pointer is currently valid.
    static auto Available() -> bool { return fgInstance != nullptr; }
    /// @brief Checks whether a previously constructed pool already expired.
    static auto Expired() -> bool { return fgExpired; }

    /// @brief Returns the active pool singleton.
    /// @return Reference to the active weak singleton pool.
    /// @throw std::runtime_error If no active pool exists.
    static auto Instance() -> WeakSingletonPool&;

    /// @brief Finds the shared indirection node for @p AWeakSingleton.
    /// @tparam AWeakSingleton Weak singleton type key.
    /// @return Shared node storing the tracked raw pointer; empty if absent.
    template<WeakSingletonified AWeakSingleton>
    auto Find() const -> std::shared_ptr<void*>;
    /// @brief Checks whether @p AWeakSingleton is registered.
    /// @tparam AWeakSingleton Weak singleton type key.
    template<WeakSingletonified AWeakSingleton>
    auto Contains() const -> auto { return fInstanceMap.contains(typeid(AWeakSingleton)); }
    /// @brief Inserts a new tracked weak singleton pointer.
    /// @tparam AWeakSingleton Weak singleton type key.
    /// @param instance Non-null pointer to register.
    /// @return Shared node storing @p instance.
    template<WeakSingletonified AWeakSingleton>
    [[nodiscard]] auto Insert(gsl::not_null<AWeakSingleton*> instance) -> std::shared_ptr<void*>;

    /// @brief Returns the global recursive mutex guarding pool operations.
    static auto RecursiveMutex() -> auto& { return fgRecursiveMutex; }

private:
    /// @brief Type-indexed weak singleton registry.
    gtl::flat_hash_map<std::type_index, std::weak_ptr<void*>> fInstanceMap;

    /// @brief Raw pointer to the active pool instance.
    static WeakSingletonPool* fgInstance;
    /// @brief Whether the pool has been constructed at least once.
    static bool fgInstantiated;
    /// @brief Whether the pool has already been destroyed.
    static bool fgExpired;
    /// @brief Global lock to protect pool from concurrent access.
    static std::recursive_mutex fgRecursiveMutex;
};

} // namespace Mustard::Env::inline ObjectRegistry::internal

#include "Mustard/Env/ObjectRegistry/internal/WeakSingletonPool.inl"
