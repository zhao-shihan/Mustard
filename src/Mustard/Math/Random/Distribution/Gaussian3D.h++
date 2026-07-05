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
#include "Mustard/Math/Random/Distribution/Gaussian.h++"
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

template<Concept::NumericVector3FloatingPoint T, template<typename> typename AGaussian3D>
class BasicGaussian3DSParameter final : public DistributionParameterBase<BasicGaussian3DSParameter<T, AGaussian3D>,
                                                                         AGaussian3D<T>> {
private:
    using Scalar = VectorValueType<T>;
    using Base = DistributionParameterBase<BasicGaussian3DSParameter<T, AGaussian3D>,
                                           AGaussian3D<T>>;

public:
    constexpr BasicGaussian3DSParameter();
    constexpr BasicGaussian3DSParameter(std::pair<Scalar, Scalar> pX, std::pair<Scalar, Scalar> pY, std::pair<Scalar, Scalar> pZ);

    constexpr auto MuX() const -> auto { return fMuX; }
    constexpr auto SigmaX() const -> auto { return fSigmaX; }
    constexpr auto MuY() const -> auto { return fMuY; }
    constexpr auto SigmaY() const -> auto { return fSigmaY; }
    constexpr auto MuZ() const -> auto { return fMuZ; }
    constexpr auto SigmaZ() const -> auto { return fSigmaZ; }

    constexpr auto MuX(Scalar muX) -> void { fMuX = muX; }
    constexpr auto SigmaX(Scalar sigmaX) -> void { fSigmaX = sigmaX; }
    constexpr auto MuY(Scalar muY) -> void { fMuY = muY; }
    constexpr auto SigmaY(Scalar sigmaY) -> void { fSigmaY = sigmaY; }
    constexpr auto MuZ(Scalar muZ) -> void { fMuZ = muZ; }
    constexpr auto SigmaZ(Scalar sigmaZ) -> void { fSigmaZ = sigmaZ; }

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const BasicGaussian3DSParameter& self) -> decltype(os) { return self.StreamOutput(os); }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, BasicGaussian3DSParameter& self) -> decltype(is) { return self.StreamInput(is); }

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
};

template<template<typename> typename ADerived, Concept::NumericVector3FloatingPoint T>
class Gaussian3DBase : public RandomNumberDistributionBase<ADerived<T>,
                                                           BasicGaussian3DSParameter<T, ADerived>,
                                                           T> {
protected:
    using Scalar = VectorValueType<T>;

private:
    using Base = RandomNumberDistributionBase<ADerived<T>,
                                              BasicGaussian3DSParameter<T, ADerived>,
                                              T>;

public:
    constexpr Gaussian3DBase() = default;
    constexpr Gaussian3DBase(std::pair<Scalar, Scalar> pX, std::pair<Scalar, Scalar> pY, std::pair<Scalar, Scalar> pZ);
    constexpr explicit Gaussian3DBase(const typename Base::ParameterType& p);

protected:
    MUSTARD_UTILITY_DECLARE_NONVIRTUAL_PROTECTED_DEFAULT_DESTRUCTOR_IN_DEFAULT_MOVEABLE(Gaussian3DBase)

public:
    constexpr auto Parameter() const -> auto { return fParameter; }
    constexpr auto MuX() const -> auto { return fParameter.MuX(); }
    constexpr auto SigmaX() const -> auto { return fParameter.SigmaX(); }
    constexpr auto MuY() const -> auto { return fParameter.MuY(); }
    constexpr auto SigmaY() const -> auto { return fParameter.SigmaY(); }
    constexpr auto MuZ() const -> auto { return fParameter.MuZ(); }
    constexpr auto SigmaZ() const -> auto { return fParameter.SigmaZ(); }

    constexpr auto Parameter(const typename Base::ParameterType& p) -> void { fParameter = p; }
    constexpr auto MuX(Scalar muX) -> void { fParameter.MuX(muX); }
    constexpr auto SigmaX(Scalar sigmaX) -> void { fParameter.SigmaX(sigmaX); }
    constexpr auto MuY(Scalar muY) -> void { fParameter.MuY(muY); }
    constexpr auto SigmaY(Scalar sigmaY) -> void { fParameter.SigmaY(sigmaY); }
    constexpr auto MuZ(Scalar muZ) -> void { fParameter.MuZ(muZ); }
    constexpr auto SigmaZ(Scalar sigmaZ) -> void { fParameter.SigmaZ(sigmaZ); }

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const Gaussian3DBase& self) -> auto& { return os << self.fParameter; }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, Gaussian3DBase& self) -> auto& { return is >> self.fParameter; }

protected:
    typename Base::ParameterType fParameter;
};

} // namespace impl

/// @brief Generates 3D normal random vectors with diagonal covariance.
/// Delegates to Gaussian2D and caches one extra random number for the next call.
/// @tparam T The result vector type. It must be 3-dimensional and has floating-point type.
template<Concept::NumericVector3FloatingPoint T = muc::array3d>
class Gaussian3D;

template<Concept::NumericVector3FloatingPoint T>
using Gaussian3DParameter = impl::BasicGaussian3DSParameter<T, Gaussian3D>;

template<Concept::NumericVector3FloatingPoint T>
class Gaussian3D final : public impl::Gaussian3DBase<Gaussian3D, T> {
private:
    using Base = impl::Gaussian3DBase<Gaussian3D, T>;
    using typename Base::Scalar;

public:
    using impl::Gaussian3DBase<Gaussian3D, T>::Gaussian3DBase;

    constexpr auto Reset() -> void { fCached = false; }

    template<std::uniform_random_bit_generator G>
    auto operator()(G& g) -> auto { return Impl(g, this->fParameter); }
    template<std::uniform_random_bit_generator G>
    auto operator()(G& g, const Gaussian3DParameter<T>& p) -> auto { return Impl(g, p); }

    auto operator()(CLHEP::HepRandomEngine& g) -> auto { return Impl(g, this->fParameter); }
    auto operator()(CLHEP::HepRandomEngine& g, const Gaussian3DParameter<T>& p) -> auto { return Impl(g, p); }

    auto operator()(TRandom& g) -> auto { return Impl(g, this->fParameter); }
    auto operator()(TRandom& g, const Gaussian3DParameter<T>& p) -> auto { return Impl(g, p); }

    constexpr auto Min() const -> T { return {std::numeric_limits<Scalar>::lowest(), std::numeric_limits<Scalar>::lowest()}; }
    constexpr auto Max() const -> T { return {std::numeric_limits<Scalar>::max(), std::numeric_limits<Scalar>::max()}; }

private:
    auto Impl(auto& g, const Gaussian3DParameter<T>& p) -> T;

private:
    bool fCached{};
    Scalar fCachedValue{};
};

template<typename T, typename U, typename V>
Gaussian3D(std::initializer_list<T>, std::initializer_list<U>, std::initializer_list<V>) -> Gaussian3D<std::array<std::common_type_t<T, U, V>, 3>>;

/// @brief Generates 3D normal random vectors with diagonal covariance.
/// Delegates to Gaussian2DS and caches one extra random number for the next call.
/// @tparam T The result vector type. It must be 3-dimensional and has floating-point type.
template<Concept::NumericVector3FloatingPoint T = muc::array3d>
class Gaussian3DS;

template<Concept::NumericVector3FloatingPoint T>
using Gaussian3DSParameter = impl::BasicGaussian3DSParameter<T, Gaussian3DS>;

template<Concept::NumericVector3FloatingPoint T>
class Gaussian3DS final : public impl::Gaussian3DBase<Gaussian3DS, T> {
private:
    using Base = impl::Gaussian3DBase<Gaussian3DS, T>;
    using typename Base::Scalar;

public:
    using impl::Gaussian3DBase<Gaussian3DS, T>::Gaussian3DBase;

    constexpr auto Reset() -> void { fCached = false; }

    template<std::uniform_random_bit_generator G>
    auto operator()(G& g) -> auto { return Impl(g, this->fParameter); }
    template<std::uniform_random_bit_generator G>
    auto operator()(G& g, const Gaussian3DSParameter<T>& p) -> auto { return Impl(g, p); }

    auto operator()(CLHEP::HepRandomEngine& g) -> auto { return Impl(g, this->fParameter); }
    auto operator()(CLHEP::HepRandomEngine& g, const Gaussian3DSParameter<T>& p) -> auto { return Impl(g, p); }

    auto operator()(TRandom& g) -> auto { return Impl(g, this->fParameter); }
    auto operator()(TRandom& g, const Gaussian3DSParameter<T>& p) -> auto { return Impl(g, p); }

    constexpr auto Min() const -> T { return {std::numeric_limits<Scalar>::lowest(), std::numeric_limits<Scalar>::lowest()}; }
    constexpr auto Max() const -> T { return {std::numeric_limits<Scalar>::max(), std::numeric_limits<Scalar>::max()}; }

private:
    auto Impl(auto& g, const Gaussian3DSParameter<T>& p) -> T;

private:
    bool fCached{};
    Scalar fCachedValue{};
};

template<typename T, typename U, typename V>
Gaussian3DS(std::initializer_list<T>, std::initializer_list<U>, std::initializer_list<V>) -> Gaussian3DS<std::array<std::common_type_t<T, U, V>, 3>>;

} // namespace Mustard::inline Math::Random::inline Distribution

#include "Mustard/Math/Random/Distribution/Gaussian3D.inl"
