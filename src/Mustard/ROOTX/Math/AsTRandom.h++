// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Math/Random/Distribution/Gaussian.h++"
#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/UniformPseudoRandomBitGenerator.h++"

#include "TRandom.h"

#include <algorithm>
#include <cstdio>

namespace Mustard::ROOTX::Math {

/// @brief Full-featured adapter for Mustard RNGs to ROOT's TRandom interface
///
/// Provides a complete implementation of ROOT's TRandom interface using any
/// Mustard UniformPseudoRandomBitGenerator. Supports all TRandom features
/// including Gaussian generation and bulk array generation.
///
/// @tparam PRBG Mustard UniformPseudoRandomBitGenerator type to adapt
///   - Must satisfy Mustard::Math::Random::UniformPseudoRandomBitGenerator
///   - Must provide SeedType and Seed() method
///
/// @note Inherits from ROOT's TRandom for full framework integration
/// @warning GetSeed() is intentionally non-functional (ROOT API limitation)
template<Mustard::Math::Random::UniformPseudoRandomBitGenerator PRBG>
class AsTRandom : public TRandom {
public:
    /// @brief Default constructor (uses PRBG's default seed)
    AsTRandom() = default;
    /// @brief Construct with specific seed value
    /// @param seed Initialization seed for underlying PRBG
    AsTRandom(typename PRBG::SeedType seed);

    virtual ~AsTRandom() override = default;

    /// @brief Generate Gaussian-distributed number
    /// @param mu Mean of the distribution (default: 0)
    /// @param sigma Standard deviation (default: 1)
    virtual auto Gaus(Double_t mu = 0, Double_t sigma = 1) -> Double_t override { return fGaussian(fPRBG, {mu, sigma}); }

    /// @brief Seed the underlying random engine
    /// @param seed New seed value (converted to PRBG's SeedType)
    virtual auto SetSeed(ULong_t seed) -> void override { fPRBG.Seed(seed); }
    /// @brief Generate uniform double in (0,1)
    virtual auto Rndm() -> Double_t override { return Mustard::Math::Random::Uniform<Double_t>{}(fPRBG); }
    /// @brief Fill array with uniform float values in (0,1)
    /// @param n Number of elements to generate
    /// @param array Pre-allocated output buffer
    virtual auto RndmArray(Int_t n, Float_t* array) -> void override;
    /// @brief Fill array with uniform double values in (0,1)
    /// @param n Number of elements to generate
    /// @param array Pre-allocated output buffer
    virtual auto RndmArray(Int_t n, Double_t* array) -> void override;

private:
    /// @brief [Disabled] TRandom requires GetSeed() but PRBGs are stateless
    /// @return Always 0
    /// @warning Prints error message when called (ROOT API limitation)
    virtual auto GetSeed() const -> UInt_t override;

private:
    PRBG fPRBG;
    Mustard::Math::Random::Gaussian<Double_t> fGaussian;
};

} // namespace Mustard::ROOTX::Math

#include "Mustard/ROOTX/Math/AsTRandom.inl"
