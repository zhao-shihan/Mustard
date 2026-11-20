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

#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/RandomNumberEngineHolder.h++"
#include "Mustard/Utility/ClassSemanticHelper.h++"

#include "Math/TRandomEngine.h"

#include "muc/utility"

#include <concepts>
#include <string>
#include <type_traits>
#include <typeinfo>

namespace Mustard::ROOTX::Math {

/// @brief Adapter that wraps Mustard RNGs as ROOT-compatible random engines
///
/// Implements the ROOT::Math::TRandomEngine interface for any
/// standard-compliant random number engine, which is either owned by this
/// adapter or referred to by it.
///
/// @tparam R A random number engine type (this adapter owns an engine of that
/// type) or an lvalue reference to one (this adapter refers to an engine
/// belonging to someone else).
///
/// @note Inherits from ROOT::Math::TRandomEngine to integrate with ROOT framework.
/// @note Not copyable nor movable: an adapter is bound to the engine it owns or
/// refers to.
template<Mustard::Random::RandomNumberEngineOrReference R>
class AsTRandomEngine : public ROOT::Math::TRandomEngine,
                        public Mustard::Random::RandomNumberEngineHolder<R>,
                        public NoncopyableBase {
private:
    using Base = Mustard::Random::RandomNumberEngineHolder<R>;

public:
    using Base::Engine;
    using typename Base::EngineType;
    using typename Base::SeedType;

    /// @brief Construct an adapter owning a default-state engine.
    /// @note Only available when @c R is an engine type.
    AsTRandomEngine()
        requires(not std::is_reference_v<R>)
    = default;
    /// @brief Construct an adapter owning an engine seeded with @p seed.
    /// @param seed Initialization seed of the engine.
    /// @note Only available when @c R is an engine type.
    explicit AsTRandomEngine(SeedType seed)
        requires(not std::is_reference_v<R>);
    /// @brief Construct an adapter referring to @p engine.
    /// @param engine Engine to refer to.
    /// @pre @p engine outlives this adapter.
    /// @note Only available when @c R is an engine reference type.
    explicit AsTRandomEngine(EngineType& engine)
        requires(std::is_reference_v<R>);

    /// @brief Generate uniform double in (0, 1) (ROOT interface requirement)
    /// @return Random double value strictly inside the unit interval
    auto Rndm() -> double override { return DrawOpen01<double>(); }

    /// @brief Functor interface equivalent to Rndm()
    auto operator()() -> auto { return Rndm(); }
    /// @brief Generate native integer from underlying PRNG
    /// @return Raw random integer from PRNG's output range
    auto IntRndm() -> auto { return Engine()(); }
    /// @brief Re-seed the underlying random engine
    /// @param seed New seed value for PRNG
    auto SetSeed(SeedType seed) -> void { Engine().seed(seed); }

    /// @brief Maximum integer value from underlying PRNG
    static auto MaxInt() -> auto { return EngineType::Max(); }
    /// @brief Minimum integer value from underlying PRNG
    static auto MinInt() -> auto { return EngineType::Min(); }
    /// @brief Human-readable name of the adapted PRNG type
    /// @return Demangled type name of the engine template parameter
    static auto Name() -> std::string { return muc::try_demangle(typeid(EngineType).name()); }

private:
    /// @brief Draw a uniform value on the open interval (0, 1).
    /// @tparam T Floating-point type of the value.
    /// @return A value strictly between 0 and 1; an exact 0, which the wrapped
    /// engines can produce, is drawn again.
    template<std::floating_point T>
    auto DrawOpen01() -> T;
};

} // namespace Mustard::ROOTX::Math

#include "Mustard/ROOTX/Math/AsTRandomEngine.inl"
