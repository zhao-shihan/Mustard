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

#include "Math/TRandomEngine.h"

#include "muc/random"
#include "muc/utility"

#include <typeinfo>

namespace Mustard::ROOTX::Math {

/// @brief Adapter that wraps Mustard RNGs as ROOT-compatible random engines
///
/// Implements the ROOT::Math::TRandomEngine interface for any standard-compliant
/// random number engine.
///
/// @tparam PRNG A standard-compliant random number engine type satisfying
/// C++ named requirements RandomNumberEngine (muc::random_number_engine).
///
/// @note Inherits from ROOT::Math::TRandomEngine to integrate with ROOT framework.
template<muc::random_number_engine PRNG>
class AsTRandomEngine : public ROOT::Math::TRandomEngine {
public:
    /// @brief Default constructor (uses PRNG's default seed)
    AsTRandomEngine() = default;
    /// @brief Construct with specific seed value
    /// @param seed Initialization seed for underlying PRNG
    explicit AsTRandomEngine(typename PRNG::SeedType seed);

    /// @brief Generate uniform double in [0,1) (ROOT interface requirement)
    /// @return Random double value in unit interval
    auto Rndm() -> double override { return Mustard::Random::Uniform<double>{}(fPRNG); }

    /// @brief Functor interface equivalent to Rndm()
    auto operator()() -> auto { return Rndm(); }
    /// @brief Generate native integer from underlying PRNG
    /// @return Raw random integer from PRNG's output range
    auto IntRndm() -> auto { return fPRNG(); }
    /// @brief Re-seed the underlying random engine
    /// @param seed New seed value for PRNG
    auto SetSeed(typename PRNG::SeedType seed) -> void { fPRNG.seed(seed); }

    /// @brief Maximum integer value from underlying PRNG
    static auto MaxInt() -> auto { return PRNG::Max(); }
    /// @brief Minimum integer value from underlying PRNG
    static auto MinInt() -> auto { return PRNG::Min(); }
    /// @brief Human-readable name of the adapted PRNG type
    /// @return Demangled type name of the PRNG template parameter
    static auto Name() -> std::string { return muc::try_demangle(typeid(PRNG).name()); }

private:
    PRNG fPRNG;
};

} // namespace Mustard::ROOTX::Math

#include "Mustard/ROOTX/Math/AsTRandomEngine.inl"
