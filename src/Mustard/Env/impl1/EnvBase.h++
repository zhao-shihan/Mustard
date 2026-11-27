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

#include "Mustard/Utility/ClassSemanticHelper.h++"

#include <memory>

namespace mp1 {
class environment;
} // namespace mp1

namespace Mustard::Env {

inline namespace ObjectRegistry {
namespace impl2 {
class WeakSingletonPool;
class SingletonPool;
} // namespace impl2
} // namespace ObjectRegistry

namespace impl1 {

/// @brief Implementation detail. Enables MPI in `EnvBase`.
/// @details Holds references to `argc`/`argv` only for the duration of the
/// `EnvBase` construction; do not store it beyond the constructor call.
struct EnableMPI {
    int& argc;
    char**& argv;
};

/// @brief Implementation detail. Non-copyable base for all environment classes.
/// @details Not API. Owns the `WeakSingletonPool` and `SingletonPool`
/// instances that back the ObjectRegistry subsystem, and optionally owns an
/// `mp1::environment` passed by an MPI-aware environment. Constructed first in
/// the virtual inheritance hierarchy, ensuring the MPI environment and the
/// pools are available before any derived environment logic executes.
/// @note `EnvBase` is a virtual base: a most-derived class that derives from an
/// MPI-aware environment must initialize it with `EnableMPI{argc, argv}`.
class EnvBase : public NoncopyableBase {
protected:
    /// @brief Constructs the base without an MPI environment.
    EnvBase();
    /// @brief Constructs the base and initializes an owned MPI environment.
    /// @param enable MPI enablement carrying the `argc`/`argv` references
    EnvBase(EnableMPI enable);
    /// @brief Destroys the base, tearing down the pools and then the MPI environment.
    ~EnvBase();

private:
    /// @brief Constructs the base, taking ownership of an optional MPI environment.
    /// @param mp1 MPI environment to own; null for a non-MPI environment
    explicit EnvBase(std::unique_ptr<mp1::environment> mp1);
    /// @brief Verifies consistency of fundamental type sizes across translation units.
    static auto CheckFundamentalType() -> void;

private:
    std::unique_ptr<mp1::environment> fMP1Env;                                    ///< Owned MPI environment, if any; destroyed last.
    std::unique_ptr<ObjectRegistry::impl2::WeakSingletonPool> fWeakSingletonPool; ///< Weak singleton registry pool.
    std::unique_ptr<ObjectRegistry::impl2::SingletonPool> fSingletonPool;         ///< Singleton registry pool.
};

} // namespace impl1

} // namespace Mustard::Env
