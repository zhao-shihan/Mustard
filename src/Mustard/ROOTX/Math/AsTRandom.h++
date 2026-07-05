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

#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Math/Random/Distribution/Exponential.h++"
#include "Mustard/Math/Random/Distribution/Gaussian.h++"
#include "Mustard/Math/Random/Distribution/Uniform.h++"

#include "TRandom.h"

#include "muc/random"

#include <algorithm>
#include <cstdio>

namespace Mustard::ROOTX::Math {

/// @brief Full-featured adapter for Mustard RNGs to ROOT's TRandom interface
///
/// Provides a complete implementation of ROOT's TRandom interface using any
/// standard-compliant random number engine. Supports all TRandom features
/// including Gaussian generation and bulk array generation.
///
/// @tparam PRNG A standard-compliant random number engine type satisfying
/// C++ named requirements RandomNumberEngine (muc::random_number_engine).
///
/// @note Inherits from ROOT's TRandom for full framework integration.
/// @warning GetSeed() is intentionally non-functional (ROOT API limitation)
template<muc::random_number_engine PRNG>
class AsTRandom : public TRandom {
public:
    /// @brief Default constructor (uses PRNG's default seed)
    AsTRandom() = default;
    /// @brief Construct with specific seed value
    /// @param seed Initialization seed for underlying PRNG
    AsTRandom(typename PRNG::SeedType seed);

    /// @brief Generate exponential-distributed number
    /// @param tau Mean of the distribution
    virtual auto Exp(double tau) -> double override { return Random::ExponentialFast{tau}(fPRNG); }
    /// @brief Generate Gaussian-distributed number
    /// @param mu Mean of the distribution (default: 0)
    /// @param sigma Standard deviation (default: 1)
    virtual auto Gaus(double mu = 0, double sigma = 1) -> double override { return fGaus(fPRNG, {mu, sigma}); }

    /// @brief Seed the underlying random engine
    /// @param seed New seed value (converted to PRNG's SeedType)
    virtual auto SetSeed(unsigned long seed) -> void override { fPRNG.seed(seed); }
    /// @brief Generate uniform double in (0,1)
    virtual auto Rndm() -> double override { return Random::Uniform<double>{}(fPRNG); }
    /// @brief Fill array with uniform float values in (0,1)
    /// @param n Number of elements to generate
    /// @param array Pre-allocated output buffer
    virtual auto RndmArray(int n, float* array) -> void override;
    /// @brief Fill array with uniform double values in (0,1)
    /// @param n Number of elements to generate
    /// @param array Pre-allocated output buffer
    virtual auto RndmArray(int n, double* array) -> void override;

    /// @brief Access underlying PRNG
    /// @return Reference to the adapted Mustard PRNG
    auto UnderlyingPRNG() -> PRNG& { return fPRNG; }
    /// @brief Access underlying PRNG
    /// @return Reference to the adapted Mustard PRNG
    auto UnderlyingPRNG() const -> const PRNG& { return fPRNG; }

private:
    /// @brief [Disabled] TRandom requires GetSeed() but PRNGs are stateless
    /// @return Always 0
    /// @warning Prints error message when called (ROOT API limitation)
    virtual auto GetSeed() const -> unsigned override;

private:
    PRNG fPRNG;
    Random::Gaussian<> fGaus;
};

} // namespace Mustard::ROOTX::Math

#include "Mustard/ROOTX/Math/AsTRandom.inl"
