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

#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/Distribution/UniformRectangle.h++"
#include "Mustard/Math/Random/Distribution/impl/FastLogOn01.h++"
#include "Mustard/Math/Random/Distribution/impl/FastSinCosOnPiPi.h++"
#include "Mustard/Math/Random/RandomNumberDistributionBase.h++"
#include "Mustard/Utility/ClassSemanticHelper.h++"
#include "Mustard/Utility/VectorValueType.h++"

#include "TRandom.h"

#include "muc/array"
#include "muc/concepts"
#include "muc/math"

#include <array>
#include <concepts>
#include <iomanip>
#include <numbers>
#include <random>
#include <type_traits>
#include <utility>

namespace Mustard::inline Math::Random::inline Distribution {

namespace impl {

template<Concept::NumericVector2FloatingPoint T, template<typename> typename AGaussian2D>
class BasicGaussian2DSParameter final : public DistributionParameterBase<BasicGaussian2DSParameter<T, AGaussian2D>,
                                                                         AGaussian2D<T>> {
private:
    using Scalar = VectorValueType<T>;
    using Base = DistributionParameterBase<BasicGaussian2DSParameter<T, AGaussian2D>,
                                           AGaussian2D<T>>;

public:
    constexpr BasicGaussian2DSParameter();
    constexpr BasicGaussian2DSParameter(std::pair<Scalar, Scalar> pX, std::pair<Scalar, Scalar> pY);

    constexpr auto MuX() const -> auto { return fMuX; }
    constexpr auto SigmaX() const -> auto { return fSigmaX; }
    constexpr auto MuY() const -> auto { return fMuY; }
    constexpr auto SigmaY() const -> auto { return fSigmaY; }

    constexpr auto MuX(Scalar muX) -> void { fMuX = muX; }
    constexpr auto SigmaX(Scalar sigmaX) -> void { fSigmaX = sigmaX; }
    constexpr auto MuY(Scalar muY) -> void { fMuY = muY; }
    constexpr auto SigmaY(Scalar sigmaY) -> void { fSigmaY = sigmaY; }

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const BasicGaussian2DSParameter& self) -> decltype(os) { return self.StreamOutput(os); }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, BasicGaussian2DSParameter& self) -> decltype(is) { return self.StreamInput(is); }

private:
    template<muc::character AChar>
    auto StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os);
    template<muc::character AChar>
    auto StreamInput(std::basic_istream<AChar>& is) & -> decltype(is);

private:
    Scalar fMuX;
    Scalar fSigmaX;
    Scalar fMuY;
    Scalar fSigmaY;
};

template<template<typename> typename ADerived, Concept::NumericVector2FloatingPoint T>
class Gaussian2DBase : public RandomNumberDistributionBase<ADerived<T>,
                                                           BasicGaussian2DSParameter<T, ADerived>,
                                                           T> {
protected:
    using Scalar = VectorValueType<T>;

private:
    using Base = RandomNumberDistributionBase<ADerived<T>,
                                              BasicGaussian2DSParameter<T, ADerived>,
                                              T>;

public:
    constexpr Gaussian2DBase() = default;
    constexpr Gaussian2DBase(std::pair<Scalar, Scalar> pX, std::pair<Scalar, Scalar> pY);
    constexpr explicit Gaussian2DBase(const typename Base::ParameterType& p);

protected:
    MUSTARD_UTILITY_DECLARE_NONVIRTUAL_PROTECTED_DEFAULT_DESTRUCTOR_IN_DEFAULT_MOVEABLE(Gaussian2DBase)

public:
    constexpr auto Parameter() const -> auto { return fParameter; }
    constexpr auto MuX() const -> auto { return fParameter.MuX(); }
    constexpr auto SigmaX() const -> auto { return fParameter.SigmaX(); }
    constexpr auto MuY() const -> auto { return fParameter.MuY(); }
    constexpr auto SigmaY() const -> auto { return fParameter.SigmaY(); }

    constexpr auto Parameter(const typename Base::ParameterType& p) -> void { fParameter = p; }
    constexpr auto MuX(Scalar muX) -> void { fParameter.MuX(muX); }
    constexpr auto SigmaX(Scalar sigmaX) -> void { fParameter.SigmaX(sigmaX); }
    constexpr auto MuY(Scalar muY) -> void { fParameter.MuY(muY); }
    constexpr auto SigmaY(Scalar sigmaY) -> void { fParameter.SigmaY(sigmaY); }

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const Gaussian2DBase& self) -> auto& { return os << self.fParameter; }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, Gaussian2DBase& self) -> auto& { return is >> self.fParameter; }

protected:
    typename Base::ParameterType fParameter;
};

} // namespace impl

/// @brief Generates 2D normal random vectors with diagonal covariance using the Box-Muller transform.
/// This version is branchless and faster than Gaussian2DS by ~1.3x to 1.5x,
/// and they are practically the same.
///
/// See also: Luc Devroye, Non-Uniform Random Variates Generation,
/// Chapter V, Section 4.4 (See http://luc.devroye.org/rnbookindex.html).
///
/// @tparam T The result vector type. It must be 2-dimensional and has floating-point type.
template<Concept::NumericVector2FloatingPoint T = muc::array2d>
class Gaussian2D;

template<Concept::NumericVector2FloatingPoint T>
using Gaussian2DParameter = impl::BasicGaussian2DSParameter<T, Gaussian2D>;

template<Concept::NumericVector2FloatingPoint T>
class Gaussian2D final : public impl::Gaussian2DBase<Gaussian2D, T> {
private:
    using Base = impl::Gaussian2DBase<Gaussian2D, T>;
    using typename Base::Scalar;

public:
    using impl::Gaussian2DBase<Gaussian2D, T>::Gaussian2DBase;

    constexpr auto Reset() -> void {}

    template<std::uniform_random_bit_generator G>
    auto operator()(G& g) -> auto { return Impl(g, this->fParameter); }
    template<std::uniform_random_bit_generator G>
    auto operator()(G& g, const Gaussian2DParameter<T>& p) -> auto { return Impl(g, p); }

    auto operator()(CLHEP::HepRandomEngine& g) -> auto { return Impl(g, this->fParameter); }
    auto operator()(CLHEP::HepRandomEngine& g, const Gaussian2DParameter<T>& p) -> auto { return Impl(g, p); }

    auto operator()(TRandom& g) -> auto { return Impl(g, this->fParameter); }
    auto operator()(TRandom& g, const Gaussian2DParameter<T>& p) -> auto { return Impl(g, p); }

    constexpr auto Min() const -> T { return {std::numeric_limits<Scalar>::lowest(), std::numeric_limits<Scalar>::lowest()}; }
    constexpr auto Max() const -> T { return {std::numeric_limits<Scalar>::max(), std::numeric_limits<Scalar>::max()}; }

private:
    static auto Impl(auto& g, const Gaussian2DParameter<T>& p) -> T;
};

template<typename T, typename U>
Gaussian2D(std::initializer_list<T>, std::initializer_list<U>) -> Gaussian2D<std::array<std::common_type_t<T, U>, 2>>;

/// @brief Generates 2D normal random vectors with diagonal covariance using the Marsaglia method.
/// This version is slower than Gaussian2D, but it has a better statistical quality
/// when you need more than 10^12 random vectors and test them all at once.
/// @tparam T The result vector type. It must be 2-dimensional and has floating-point type.
template<Concept::NumericVector2FloatingPoint T = muc::array2d>
class Gaussian2DS;

template<Concept::NumericVector2FloatingPoint T>
using Gaussian2DSParameter = impl::BasicGaussian2DSParameter<T, Gaussian2DS>;

template<Concept::NumericVector2FloatingPoint T>
class Gaussian2DS final : public impl::Gaussian2DBase<Gaussian2DS, T> {
private:
    using Base = impl::Gaussian2DBase<Gaussian2DS, T>;
    using typename Base::Scalar;

public:
    using impl::Gaussian2DBase<Gaussian2DS, T>::Gaussian2DBase;

    constexpr auto Reset() -> void {}

    template<std::uniform_random_bit_generator G>
    auto operator()(G& g) -> auto { return Impl(g, this->fParameter); }
    template<std::uniform_random_bit_generator G>
    auto operator()(G& g, const Gaussian2DSParameter<T>& p) -> auto { return Impl(g, p); }

    auto operator()(CLHEP::HepRandomEngine& g) -> auto { return Impl(g, this->fParameter); }
    auto operator()(CLHEP::HepRandomEngine& g, const Gaussian2DSParameter<T>& p) -> auto { return Impl(g, p); }

    auto operator()(TRandom& g) -> auto { return Impl(g, this->fParameter); }
    auto operator()(TRandom& g, const Gaussian2DSParameter<T>& p) -> auto { return Impl(g, p); }

    constexpr auto Min() const -> T { return {std::numeric_limits<Scalar>::lowest(), std::numeric_limits<Scalar>::lowest()}; }
    constexpr auto Max() const -> T { return {std::numeric_limits<Scalar>::max(), std::numeric_limits<Scalar>::max()}; }

private:
    static auto Impl(auto& g, const Gaussian2DSParameter<T>& p) -> T;
};

template<typename T, typename U>
Gaussian2DS(std::initializer_list<T>, std::initializer_list<U>) -> Gaussian2DS<std::array<std::common_type_t<T, U>, 2>>;

} // namespace Mustard::inline Math::Random::inline Distribution

#include "Mustard/Math/Random/Distribution/Gaussian2D.inl"
