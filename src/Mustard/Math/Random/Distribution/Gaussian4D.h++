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
#include "Mustard/Math/Random/Distribution/Gaussian2D.h++"
#include "Mustard/Math/Random/RandomNumberDistributionBase.h++"
#include "Mustard/Utility/ClassSemanticHelper.h++"
#include "Mustard/Utility/VectorValueType.h++"

#include "TRandom.h"

#include "muc/array"
#include "muc/concepts"

#include <array>
#include <concepts>
#include <iomanip>
#include <random>
#include <type_traits>
#include <utility>

namespace Mustard::inline Math::Random::inline Distribution {

namespace impl {

template<Concept::NumericVector4FloatingPoint T, template<typename> typename AGaussian4D>
class BasicGaussian4DSParameter final : public DistributionParameterBase<BasicGaussian4DSParameter<T, AGaussian4D>,
                                                                         AGaussian4D<T>> {
private:
    using Scalar = VectorValueType<T>;
    using Base = DistributionParameterBase<BasicGaussian4DSParameter<T, AGaussian4D>,
                                           AGaussian4D<T>>;

public:
    constexpr BasicGaussian4DSParameter();
    constexpr BasicGaussian4DSParameter(std::pair<Scalar, Scalar> pX, std::pair<Scalar, Scalar> pY, std::pair<Scalar, Scalar> pZ, std::pair<Scalar, Scalar> pW);

    constexpr auto MuX() const -> auto { return fMuX; }
    constexpr auto SigmaX() const -> auto { return fSigmaX; }
    constexpr auto MuY() const -> auto { return fMuY; }
    constexpr auto SigmaY() const -> auto { return fSigmaY; }
    constexpr auto MuZ() const -> auto { return fMuZ; }
    constexpr auto SigmaZ() const -> auto { return fSigmaZ; }
    constexpr auto MuW() const -> auto { return fMuW; }
    constexpr auto SigmaW() const -> auto { return fSigmaW; }

    constexpr auto MuX(Scalar muX) -> void { fMuX = muX; }
    constexpr auto SigmaX(Scalar sigmaX) -> void { fSigmaX = sigmaX; }
    constexpr auto MuY(Scalar muY) -> void { fMuY = muY; }
    constexpr auto SigmaY(Scalar sigmaY) -> void { fSigmaY = sigmaY; }
    constexpr auto MuZ(Scalar muZ) -> void { fMuZ = muZ; }
    constexpr auto SigmaZ(Scalar sigmaZ) -> void { fSigmaZ = sigmaZ; }
    constexpr auto MuW(Scalar muW) -> void { fMuW = muW; }
    constexpr auto SigmaW(Scalar sigmaW) -> void { fSigmaW = sigmaW; }

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const BasicGaussian4DSParameter& self) -> decltype(os) { return self.StreamOutput(os); }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, BasicGaussian4DSParameter& self) -> decltype(is) { return self.StreamInput(is); }

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
    Scalar fMuZ;
    Scalar fSigmaZ;
    Scalar fMuW;
    Scalar fSigmaW;
};

template<template<typename> typename ADerived, Concept::NumericVector4FloatingPoint T>
class Gaussian4DBase : public RandomNumberDistributionBase<ADerived<T>,
                                                           BasicGaussian4DSParameter<T, ADerived>,
                                                           T> {
protected:
    using Scalar = VectorValueType<T>;

private:
    using Base = RandomNumberDistributionBase<ADerived<T>,
                                              BasicGaussian4DSParameter<T, ADerived>,
                                              T>;

public:
    constexpr Gaussian4DBase() = default;
    constexpr Gaussian4DBase(std::pair<Scalar, Scalar> pX, std::pair<Scalar, Scalar> pY, std::pair<Scalar, Scalar> pZ, std::pair<Scalar, Scalar> pW);
    constexpr explicit Gaussian4DBase(const typename Base::ParameterType& p);

protected:
    MUSTARD_UTILITY_DECLARE_NONVIRTUAL_PROTECTED_DEFAULT_DESTRUCTOR_IN_DEFAULT_MOVEABLE(Gaussian4DBase)

public:
    constexpr auto Parameter() const -> auto { return fParameter; }
    constexpr auto MuX() const -> auto { return fParameter.MuX(); }
    constexpr auto SigmaX() const -> auto { return fParameter.SigmaX(); }
    constexpr auto MuY() const -> auto { return fParameter.MuY(); }
    constexpr auto SigmaY() const -> auto { return fParameter.SigmaY(); }
    constexpr auto MuZ() const -> auto { return fParameter.MuZ(); }
    constexpr auto SigmaZ() const -> auto { return fParameter.SigmaZ(); }
    constexpr auto MuW() const -> auto { return fParameter.MuW(); }
    constexpr auto SigmaW() const -> auto { return fParameter.SigmaW(); }

    constexpr auto Parameter(const typename Base::ParameterType& p) -> void { fParameter = p; }
    constexpr auto MuX(Scalar muX) -> void { fParameter.MuX(muX); }
    constexpr auto SigmaX(Scalar sigmaX) -> void { fParameter.SigmaX(sigmaX); }
    constexpr auto MuY(Scalar muY) -> void { fParameter.MuY(muY); }
    constexpr auto SigmaY(Scalar sigmaY) -> void { fParameter.SigmaY(sigmaY); }
    constexpr auto MuZ(Scalar muZ) -> void { fParameter.MuZ(muZ); }
    constexpr auto SigmaZ(Scalar sigmaZ) -> void { fParameter.SigmaZ(sigmaZ); }
    constexpr auto MuW(Scalar muW) -> void { fParameter.MuW(muW); }
    constexpr auto SigmaW(Scalar sigmaW) -> void { fParameter.SigmaW(sigmaW); }

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const Gaussian4DBase& self) -> auto& { return os << self.fParameter; }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, Gaussian4DBase& self) -> auto& { return is >> self.fParameter; }

protected:
    typename Base::ParameterType fParameter;
};

} // namespace impl

/// @brief Generates 4D normal random vectors with diagonal covariance using the Box-Muller transform.
/// This version uses optimized math functions to compute two log or four trigonometric values at once.
/// It is branchless and faster than Gaussian4DS by ~1.7x on x86-64 with FMA and AVX2 and they are practically the same.
/// @tparam T The result vector type. It must be 4-dimensional and has floating-point type.
template<Concept::NumericVector4FloatingPoint T = muc::array4d>
class Gaussian4D;

template<Concept::NumericVector4FloatingPoint T>
using Gaussian4DParameter = impl::BasicGaussian4DSParameter<T, Gaussian4D>;

template<Concept::NumericVector4FloatingPoint T>
class Gaussian4D final : public impl::Gaussian4DBase<Gaussian4D, T> {
private:
    using Base = impl::Gaussian4DBase<Gaussian4D, T>;
    using typename Base::Scalar;

public:
    using impl::Gaussian4DBase<Gaussian4D, T>::Gaussian4DBase;

    constexpr auto Reset() -> void {}

    template<std::uniform_random_bit_generator G>
    auto operator()(G& g) -> auto { return Impl(g, this->fParameter); }
    template<std::uniform_random_bit_generator G>
    auto operator()(G& g, const Gaussian4DParameter<T>& p) -> auto { return Impl(g, p); }

    auto operator()(CLHEP::HepRandomEngine& g) -> auto { return Impl(g, this->fParameter); }
    auto operator()(CLHEP::HepRandomEngine& g, const Gaussian4DParameter<T>& p) -> auto { return Impl(g, p); }

    auto operator()(TRandom& g) -> auto { return Impl(g, this->fParameter); }
    auto operator()(TRandom& g, const Gaussian4DParameter<T>& p) -> auto { return Impl(g, p); }

    constexpr auto Min() const -> T { return {std::numeric_limits<Scalar>::lowest(), std::numeric_limits<Scalar>::lowest(), std::numeric_limits<Scalar>::lowest(), std::numeric_limits<Scalar>::lowest()}; }
    constexpr auto Max() const -> T { return {std::numeric_limits<Scalar>::max(), std::numeric_limits<Scalar>::max(), std::numeric_limits<Scalar>::max(), std::numeric_limits<Scalar>::max()}; }

private:
    static auto Impl(auto& g, const Gaussian4DParameter<T>& p) -> T;
};

template<typename T, typename U, typename V, typename W>
Gaussian4D(std::initializer_list<T>, std::initializer_list<U>, std::initializer_list<V>, std::initializer_list<W>) -> Gaussian4D<std::array<std::common_type_t<T, U, V, W>, 4>>;

/// @brief Generates 4D normal random vectors with diagonal covariance using the Marsaglia method.
/// Delegates to Gaussian2DS twice. This version is mainly for testing purpose.
/// @tparam T The result vector type. It must be 4-dimensional and has floating-point type.
template<Concept::NumericVector4FloatingPoint T = muc::array4d>
class Gaussian4DS;

template<Concept::NumericVector4FloatingPoint T>
using Gaussian4DSParameter = impl::BasicGaussian4DSParameter<T, Gaussian4DS>;

template<Concept::NumericVector4FloatingPoint T>
class Gaussian4DS final : public impl::Gaussian4DBase<Gaussian4DS, T> {
private:
    using Base = impl::Gaussian4DBase<Gaussian4DS, T>;
    using typename Base::Scalar;

public:
    using impl::Gaussian4DBase<Gaussian4DS, T>::Gaussian4DBase;

    constexpr auto Reset() -> void {}

    template<std::uniform_random_bit_generator G>
    auto operator()(G& g) -> auto { return Impl(g, this->fParameter); }
    template<std::uniform_random_bit_generator G>
    auto operator()(G& g, const Gaussian4DSParameter<T>& p) -> auto { return Impl(g, p); }

    auto operator()(CLHEP::HepRandomEngine& g) -> auto { return Impl(g, this->fParameter); }
    auto operator()(CLHEP::HepRandomEngine& g, const Gaussian4DSParameter<T>& p) -> auto { return Impl(g, p); }

    auto operator()(TRandom& g) -> auto { return Impl(g, this->fParameter); }
    auto operator()(TRandom& g, const Gaussian4DSParameter<T>& p) -> auto { return Impl(g, p); }

    constexpr auto Min() const -> T { return {std::numeric_limits<Scalar>::lowest(), std::numeric_limits<Scalar>::lowest(), std::numeric_limits<Scalar>::lowest(), std::numeric_limits<Scalar>::lowest()}; }
    constexpr auto Max() const -> T { return {std::numeric_limits<Scalar>::max(), std::numeric_limits<Scalar>::max(), std::numeric_limits<Scalar>::max(), std::numeric_limits<Scalar>::max()}; }

private:
    static auto Impl(auto& g, const Gaussian4DSParameter<T>& p) -> T;
};

template<typename T, typename U, typename V, typename W>
Gaussian4DS(std::initializer_list<T>, std::initializer_list<U>, std::initializer_list<V>, std::initializer_list<W>) -> Gaussian4DS<std::array<std::common_type_t<T, U, V, W>, 4>>;

} // namespace Mustard::inline Math::Random::inline Distribution

#include "Mustard/Math/Random/Distribution/Gaussian4D.inl"
