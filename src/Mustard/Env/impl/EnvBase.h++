// -*- C++ -*-
//
// Copyright (C) 2020-2026  Shihan Zhao and contributors
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

#include "Mustard/Utility/NonCopyableBase.h++"

#include <memory>

namespace Mustard::Env {

inline namespace ObjectRegistry {
namespace impl {
class WeakSingletonPool;
class SingletonPool;
} // namespace impl
} // namespace ObjectRegistry

namespace impl {

/// @brief Implementation detail. Non-copyable base for all environment classes.
/// @details Not API. Owns the `WeakSingletonPool` and `SingletonPool`
/// instances that back the ObjectRegistry subsystem. Constructed first in
/// the virtual inheritance hierarchy, ensuring the pools are available
/// before any derived environment logic executes.
class EnvBase : public NonCopyableBase {
protected:
    /// @brief Constructs the base, instantiating the singleton and weak-singleton pools.
    EnvBase();
    /// @brief Destroys the base, tearing down the pools in reverse order.
    ~EnvBase();

private:
    /// @brief Verifies consistency of fundamental type sizes across translation units.
    static auto CheckFundamentalType() -> void;

private:
    std::unique_ptr<ObjectRegistry::impl::WeakSingletonPool> fWeakSingletonPool; ///< Weak singleton registry pool.
    std::unique_ptr<ObjectRegistry::impl::SingletonPool> fSingletonPool;         ///< Singleton registry pool.
};

} // namespace impl

} // namespace Mustard::Env
