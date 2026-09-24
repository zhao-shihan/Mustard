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

#include "Mustard/Math/Random/Distribution/Exponential.h++"
#include "Mustard/Math/Random/Distribution/Gaussian.h++"
#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/Generator/RandomDevice.h++"
#include "Mustard/Math/Random/RandomNumberEngineHolder.h++"
#include "Mustard/Utility/ClassSemanticHelper.h++"

#include "TRandom.h"

#include <algorithm>
#include <concepts>
#include <type_traits>

namespace Mustard::ROOTX::Math {

/// @brief Full-featured adapter for Mustard RNGs to ROOT's TRandom interface
///
/// Provides a complete implementation of ROOT's TRandom interface using any
/// standard-compliant random number engine, which is either owned by this
/// adapter or referred to by it. Supports all TRandom features including
/// Gaussian generation and bulk array generation.
///
/// @tparam R A random number engine type (this adapter owns an engine of that
/// type) or an lvalue reference to one (this adapter refers to an engine
/// belonging to someone else).
///
/// @note Inherits from ROOT's TRandom for full framework integration.
/// @note Not copyable nor movable: an adapter is bound to the engine it owns or
/// refers to, and its seed record cannot be transferred with it.
/// @note The seed record, i.e. GetSeed(), reports the seed this adapter was
/// constructed or SetSeed()-ed with, and 0 as long as it has not been told any
/// seed. A wrapper referring to an external engine cannot know how that engine
/// was seeded, and the engine may be reseeded behind its back at any time.
/// ROOT truncates it to 32 bits in any case.
template<Mustard::Random::RandomNumberEngineOrReference R>
class AsTRandom : public TRandom,
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
    AsTRandom()
        requires(not std::is_reference_v<R>);
    /// @brief Construct an adapter owning an engine seeded with @p seed.
    /// @param seed Initialization seed of the engine; 0 draws a non-deterministic
    /// one, exactly as TRandom(0) does.
    /// @note Only available when @c R is an engine type.
    explicit AsTRandom(SeedType seed)
        requires(not std::is_reference_v<R>);
    /// @brief Construct an adapter referring to @p engine.
    /// @param engine Engine to refer to.
    /// @pre @p engine outlives this adapter.
    /// @note Only available when @c R is an engine reference type.
    explicit AsTRandom(EngineType& engine)
        requires(std::is_reference_v<R>);

    /// @brief Generate exponential-distributed number
    /// @param tau Mean of the distribution
    virtual auto Exp(double tau) -> double override { return Random::Exponential{tau}(Engine()); }
    /// @brief Generate Gaussian-distributed number
    /// @param mu Mean of the distribution (default: 0)
    /// @param sigma Standard deviation (default: 1)
    /// @note The cached variate pair is dropped by SetSeed(), but not when the
    /// engine is reseeded through another handle referring to it, in which case
    /// one variate may still come from the previous stream.
    virtual auto Gaus(double mu = 0, double sigma = 1) -> double override { return fGaus(Engine(), {mu, sigma}); }

    /// @brief Seed the underlying random engine
    /// @param seed New seed value (converted to the engine's seed type); 0 draws
    /// a non-deterministic one, exactly as TRandom::SetSeed(0) does
    virtual auto SetSeed(unsigned long seed) -> void override;
    /// @brief Generate uniform double in (0, 1)
    virtual auto Rndm() -> double override { return DrawOpen01<double>(); }
    /// @brief Fill array with uniform float values in (0, 1)
    /// @param n Number of elements to generate
    /// @param array Pre-allocated output buffer
    virtual auto RndmArray(int n, float* array) -> void override;
    /// @brief Fill array with uniform double values in (0, 1)
    /// @param n Number of elements to generate
    /// @param array Pre-allocated output buffer
    virtual auto RndmArray(int n, double* array) -> void override;

private:
    /// @brief Draw a uniform value on the open interval (0, 1).
    /// @tparam T Floating-point type of the value.
    /// @return A value strictly between 0 and 1, as ROOT's Rndm() gives: an
    /// exact 0, which the wrapped engines can produce, is drawn again.
    template<std::floating_point T>
    auto DrawOpen01() -> T;

private:
    Random::Gaussian<> fGaus;
};

} // namespace Mustard::ROOTX::Math

#include "Mustard/ROOTX/Math/AsTRandom.inl"
