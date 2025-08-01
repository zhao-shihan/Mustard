// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
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
#include "Mustard/Utility/PrettyLog.h++"

#include "CLHEP/Random/RandomEngine.h"

#include "muc/math"
#include "muc/utility"

#include "gsl/gsl"

#include "fmt/core.h"

#include <cstdio>
#include <fstream>
#include <limits>
#include <string>
#include <string_view>
#include <typeinfo>

namespace Mustard::CLHEPX::Random {

/// @brief Adapter for Mustard RNGs to CLHEP random engine interface
///
/// Provides full compatibility with CLHEP's HepRandomEngine API for any
/// UniformPseudoRandomBitGenerator.
///
/// @tparam PRBG Mustard UniformPseudoRandomBitGenerator type to wrap
///   - Must satisfy Mustard::Math::Random::UniformPseudoRandomBitGenerator
///
/// @note Implements all CLHEP engine virtual methods except getState()
/// @warning getState() is intentionally non-functional
template<Math::Random::UniformPseudoRandomBitGenerator PRBG>
class Wrap : public CLHEP::HepRandomEngine {
public:
    /// @brief Construct with default seed
    Wrap() = default;
    /// @brief Construct with specific seed
    /// @param seed Initialization value (converted to PRBG::SeedType)
    explicit Wrap(long seed);

    ~Wrap() = default;

    /// @brief Generate uniform double in 0--1
    virtual auto flat() -> double override { return Math::Random::Uniform<double>{}(fPRBG); }
    /// @brief Fill array with uniform doubles
    /// @param size Number of values to generate
    /// @param vect Pre-allocated output buffer
    virtual auto flatArray(const int size, double* vect) -> void override;

    /// @brief Seed the engine (single seed)
    /// @param seed New seed value
    /// @param (ignored) Stream index parameter (CLHEP compatibility)
    virtual auto setSeed(long seed, int = 0) -> void override;
    /// @brief Seed the engine (seed array)
    /// @param seeds Pointer to seed array (uses first element)
    /// @param (ignored) Stream index parameter
    virtual auto setSeeds(const long* seeds, int = 0) -> void override;

    /// @brief Save engine state to file
    /// @param filename Output file path
    virtual auto saveStatus(gsl::czstring filename) const -> void override;
    /// @brief Restore engine state from file
    /// @param filename Input file path
    virtual auto restoreStatus(gsl::czstring filename) -> void override;
    /// @brief Print engine status to stdout
    virtual auto showStatus() const -> void override;

    /// @brief Get engine type name
    /// @return Demangled type name of the wrapped PRBG
    virtual auto name() const -> std::string override { return muc::try_demangle(typeid(Wrap).name()); }

    /// @brief Serialize engine state to output stream
    virtual auto put(std::ostream& os) const -> decltype(os) override;
    /// @brief Deserialize engine state from input stream
    virtual auto get(std::istream& is) -> decltype(is) override;

    /// @name Random-number-generating conversion operators
    /// @{
    virtual operator double() override { return Math::Random::Uniform<double>()(fPRBG); }
    virtual operator float() override { return Math::Random::Uniform<float>()(fPRBG); }
    virtual operator unsigned int() override { return fPRBG(); }
    /// @}

private:
    /// @brief [Disabled] CLHEP state restoration method
    /// @return Input stream unchanged
    /// @warning DO NOT USE
    virtual auto getState(std::istream& is) -> decltype(is) override;

private:
    PRBG fPRBG;
};

} // namespace Mustard::CLHEPX::Random

#include "Mustard/CLHEPX/Random/Wrap.inl"
