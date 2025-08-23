// -*- C++ -*-
//
// Copyright 2020-2025  The Mustard development team
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
#include "Mustard/Math/Random/UniformPseudoRandomBitGenerator.h++"

#include "Math/TRandomEngine.h"

#include "muc/utility"

#include <typeinfo>

namespace Mustard::ROOTX::Math {

/// @brief Adapter that wraps Mustard RNGs as ROOT-compatible random engines
///
/// Implements the ROOT::Math::TRandomEngine interface for any Mustard library
/// UniformPseudoRandomBitGenerator type. Allows using modern Mustard RNGs
/// with ROOT's statistical functions and classes.
///
/// @tparam PRBG Mustard UniformPseudoRandomBitGenerator type to adapt
///   - Must satisfy Mustard::Math::Random::UniformPseudoRandomBitGenerator concept
///   - Must provide SeedType, Seed(), Min(), Max(), and operator()
///
/// @note Inherits from ROOT::Math::TRandomEngine to integrate with ROOT framework
template<Mustard::Math::Random::UniformPseudoRandomBitGenerator PRBG>
class AsTRandomEngine : public ROOT::Math::TRandomEngine {
public:
    /// @brief Default constructor (uses PRBG's default seed)
    AsTRandomEngine() = default;
    /// @brief Construct with specific seed value
    /// @param seed Initialization seed for underlying PRBG
    explicit AsTRandomEngine(typename PRBG::SeedType seed);

    /// @brief Virtual destructor (default implementation)
    virtual ~AsTRandomEngine() override = default;

    /// @brief Generate uniform double in [0,1) (ROOT interface requirement)
    /// @return Random double value in unit interval
    auto Rndm() -> double override { Mustard::Math::Random::Uniform<double>{}(fPRBG); }

    /// @brief Functor interface equivalent to Rndm()
    auto operator()() -> auto { return Rndm(); }
    /// @brief Generate native integer from underlying PRBG
    /// @return Raw random integer from PRBG's output range
    auto IntRndm() -> auto { return fPRBG(); }
    /// @brief Re-seed the underlying random engine
    /// @param seed New seed value for PRBG
    auto SetSeed(typename PRBG::SeedType seed) -> void { fPRBG.Seed(seed); }

    /// @brief Maximum integer value from underlying PRBG
    static auto MaxInt() -> auto { return PRBG::Max(); }
    /// @brief Minimum integer value from underlying PRBG
    static auto MinInt() -> auto { return PRBG::Min(); }
    /// @brief Human-readable name of the adapted PRBG type
    /// @return Demangled type name of the PRBG template parameter
    static auto Name() -> std::string { return muc::try_demangle(typeid(PRBG).name()); }

private:
    PRBG fPRBG;
};

} // namespace Mustard::ROOTX::Math

#include "Mustard/ROOTX/Math/AsTRandomEngine.inl"
