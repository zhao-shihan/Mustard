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

#include "Mustard/Env/ObjectRegistry/PassiveSingleton.h++"
#include "Mustard/Env/ObjectRegistry/Singletonified.h++"
#include "Mustard/Env/ObjectRegistry/internal/SingletonBase.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Memory/Allocator.h++"
#include "Mustard/Utility/NonConstructibleBase.h++"

#include "gtl/phmap.hpp"

#include "muc/utility"

#include "gsl/gsl"

#include "fmt/format.h"

#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <tuple>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <vector>

namespace Mustard::Env::inline ObjectRegistry {

namespace internal {

/// @brief Implementation detail of Mustard::Env::Singleton.
/// @details Not API. Stores singleton indirection nodes keyed by dynamic type.
/// @note This central pool is the anchor that allows Singleton<T> lookups to
/// converge to one instance across dynamic library boundaries within the same
/// running environment.
class SingletonPool final : public PassiveSingleton<SingletonPool> {
public:
    /// @brief Constructs an empty singleton pool.
    SingletonPool();
    /// @brief Disposes alive singletons in reverse construction order.
    ~SingletonPool();

    /// @brief Finds the shared indirection node for @p ASingleton.
    /// @tparam ASingleton Singleton type key.
    /// @return Shared node holding the singleton raw pointer; empty if missing.
    template<Singletonified ASingleton>
    auto Find() const -> std::shared_ptr<void*>;
    /// @brief Checks whether @p ASingleton has been registered.
    /// @tparam ASingleton Singleton type key.
    template<Singletonified ASingleton>
    auto Contains() const -> auto { return fInstanceMap.contains(typeid(ASingleton)); }
    /// @brief Creates and registers @p ASingleton.
    /// @tparam ASingleton Singleton type to construct.
    /// @return Shared node holding the newly created raw pointer.
    template<Singletonified ASingleton>
    [[nodiscard]] auto Make() -> std::shared_ptr<void*>;

    /// @brief Returns the global recursive mutex guarding pool operations.
    static auto RecursiveMutex() -> auto& { return fgRecursiveMutex; }

private:
    /// @brief Metadata tracked for one singleton entry.
    struct InstanceInfo {
        /// @brief Indirection node shared by all Singleton<T> accessors.
        std::weak_ptr<void*> instancePtr;
        /// @brief Creation order used for deterministic reverse destruction.
        gsl::index insertionOrder;
        /// @brief Type-erased disposer for the singleton object.
        std::function<auto()->void> dispose;
    };

private:
    /// @brief Type-indexed singleton registry.
    gtl::flat_hash_map<std::type_index, InstanceInfo> fInstanceMap;

    /// @brief Global lock to protect pool from concurrent access.
    static std::recursive_mutex fgRecursiveMutex;
};

} // namespace internal

/// @brief Internal type-erased construction/disposal helper for singletons.
/// @note Singletons should declare SingletonFactory as a friend
/// to allow SingletonFactory to construct and destruct them.
class SingletonFactory : public NonConstructibleBase {
    friend class internal::SingletonPool;

    /// @brief Allocates and constructs one singleton instance.
    /// @tparam ASingleton Singleton type.
    /// @return Pair of raw object pointer and shared indirection node.
    template<Singletonified ASingleton>
    [[nodiscard]] static auto Make() -> std::pair<gsl::owner<ASingleton*>, std::shared_ptr<void*>>;
    /// @brief Destroys and deallocates one singleton instance.
    /// @tparam ASingleton Singleton type.
    /// @param instance Raw singleton pointer previously returned by Make().
    template<Singletonified ASingleton>
    static auto Dispose(gsl::owner<ASingleton*> instance) noexcept -> void;
};

} // namespace Mustard::Env::inline ObjectRegistry

#include "Mustard/Env/ObjectRegistry/internal/SingletonPool.inl"
