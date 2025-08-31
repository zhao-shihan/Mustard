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

#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Math/Random/Distribution/Gaussian.h++"
#include "Mustard/Math/Random/RandomNumberDistributionBase.h++"
#include "Mustard/Utility/VectorValueType.h++"

#include "muc/array"
#include "muc/concepts"

#include <array>
#include <concepts>
#include <iomanip>
#include <type_traits>
#include <utility>

namespace Mustard::Math::Random::inline Distribution {

namespace internal {

template<Concept::NumericVector3FloatingPoint T, template<typename> typename AGaussian3DDiagnoal>
class BasicGaussian3DDiagnoalParameter final : public DistributionParameterBase<BasicGaussian3DDiagnoalParameter<T, AGaussian3DDiagnoal>,
                                                                                AGaussian3DDiagnoal<T>> {
private:
    using VT = VectorValueType<T>;
    using Base = DistributionParameterBase<BasicGaussian3DDiagnoalParameter<T, AGaussian3DDiagnoal>,
                                           AGaussian3DDiagnoal<T>>;

public:
    constexpr BasicGaussian3DDiagnoalParameter();
    constexpr BasicGaussian3DDiagnoalParameter(std::pair<VT, VT> pX, std::pair<VT, VT> pY, std::pair<VT, VT> pZ);

    constexpr auto MuX() const -> auto { return fMuX; }
    constexpr auto SigmaX() const -> auto { return fSigmaX; }
    constexpr auto MuY() const -> auto { return fMuY; }
    constexpr auto SigmaY() const -> auto { return fSigmaY; }
    constexpr auto MuZ() const -> auto { return fMuZ; }
    constexpr auto SigmaZ() const -> auto { return fSigmaZ; }

    constexpr auto MuX(VT muX) -> void { fMuX = muX; }
    constexpr auto SigmaX(VT sigmaX) -> void { fSigmaX = sigmaX; }
    constexpr auto MuY(VT muY) -> void { fMuY = muY; }
    constexpr auto SigmaY(VT sigmaY) -> void { fSigmaY = sigmaY; }
    constexpr auto MuZ(VT muZ) -> void { fMuZ = muZ; }
    constexpr auto SigmaZ(VT sigmaZ) -> void { fSigmaZ = sigmaZ; }

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const BasicGaussian3DDiagnoalParameter& self) -> decltype(os) { return self.StreamOutput(os); }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, BasicGaussian3DDiagnoalParameter& self) -> decltype(is) { return self.StreamInput(is); }

private:
    template<muc::character AChar>
    auto StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os);
    template<muc::character AChar>
    auto StreamInput(std::basic_istream<AChar>& is) & -> decltype(is);

private:
    VT fMuX;
    VT fSigmaX;
    VT fMuY;
    VT fSigmaY;
    VT fMuZ;
    VT fSigmaZ;
};

template<template<typename> typename ADerived, Concept::NumericVector3FloatingPoint T>
class Gaussian3DDiagnoalBase : public RandomNumberDistributionBase<ADerived<T>,
                                                                   BasicGaussian3DDiagnoalParameter<T, ADerived>,
                                                                   T> {
protected:
    using VT = VectorValueType<T>;

private:
    using Base = RandomNumberDistributionBase<ADerived<T>,
                                              BasicGaussian3DDiagnoalParameter<T, ADerived>,
                                              T>;

public:
    constexpr Gaussian3DDiagnoalBase() = default;
    constexpr Gaussian3DDiagnoalBase(std::pair<VT, VT> pX, std::pair<VT, VT> pY, std::pair<VT, VT> pZ);
    constexpr explicit Gaussian3DDiagnoalBase(const typename Base::ParameterType& p);

protected:
    constexpr ~Gaussian3DDiagnoalBase() = default;

public:
    constexpr auto Parameter() const -> auto { return fParameter; }
    constexpr auto MuX() const -> auto { return fParameter.MuX(); }
    constexpr auto SigmaX() const -> auto { return fParameter.SigmaX(); }
    constexpr auto MuY() const -> auto { return fParameter.MuY(); }
    constexpr auto SigmaY() const -> auto { return fParameter.SigmaY(); }
    constexpr auto MuZ() const -> auto { return fParameter.MuZ(); }
    constexpr auto SigmaZ() const -> auto { return fParameter.SigmaZ(); }

    constexpr auto Parameter(const typename Base::ParameterType& p) -> void { fParameter = p; }
    constexpr auto MuX(VT muX) -> void { fParameter.MuX(muX); }
    constexpr auto SigmaX(VT sigmaX) -> void { fParameter.SigmaX(sigmaX); }
    constexpr auto MuY(VT muY) -> void { fParameter.MuY(muY); }
    constexpr auto SigmaY(VT sigmaY) -> void { fParameter.SigmaY(sigmaY); }
    constexpr auto MuZ(VT muZ) -> void { fParameter.MuZ(muZ); }
    constexpr auto SigmaZ(VT sigmaZ) -> void { fParameter.SigmaZ(sigmaZ); }

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const Gaussian3DDiagnoalBase& self) -> auto& { return os << self.fParameter; }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, Gaussian3DDiagnoalBase& self) -> auto& { return is >> self.fParameter; }

protected:
    typename Base::ParameterType fParameter;
};

} // namespace internal

/// @brief
/// @tparam T The result vector type. It must be 2-dimensional and has floating-point type.
template<Concept::NumericVector3FloatingPoint T = muc::array3d>
class Gaussian3DDiagnoal;

template<Concept::NumericVector3FloatingPoint T>
using Gaussian3DDiagnoalParameter = internal::BasicGaussian3DDiagnoalParameter<T, Gaussian3DDiagnoal>;

template<Concept::NumericVector3FloatingPoint T>
class Gaussian3DDiagnoal final : public internal::Gaussian3DDiagnoalBase<Gaussian3DDiagnoal, T> {
private:
    using Base = internal::Gaussian3DDiagnoalBase<Gaussian3DDiagnoal, T>;
    using VT = typename Base::VT;

public:
    using internal::Gaussian3DDiagnoalBase<Gaussian3DDiagnoal, T>::Gaussian3DDiagnoalBase;

    constexpr void Reset() { fSaved = false; }

    MUSTARD_OPTIMIZE_FAST MUSTARD_ALWAYS_INLINE auto operator()(UniformRandomBitGenerator auto& g) -> auto { return (*this)(g, this->fParameter); }
    MUSTARD_OPTIMIZE_FAST MUSTARD_ALWAYS_INLINE auto operator()(UniformRandomBitGenerator auto& g, const Gaussian3DDiagnoalParameter<T>& p) -> T;

    constexpr auto Min() const -> T { return {std::numeric_limits<VT>::lowest(), std::numeric_limits<VT>::lowest()}; }
    constexpr auto Max() const -> T { return {std::numeric_limits<VT>::max(), std::numeric_limits<VT>::max()}; }

    static constexpr auto Stateless() { return false; }

private:
    bool fSaved{};
    VT fSavedValue{};
};

template<typename T, typename U, typename V>
Gaussian3DDiagnoal(std::initializer_list<T>, std::initializer_list<U>, std::initializer_list<V>) -> Gaussian3DDiagnoal<std::array<std::common_type_t<T, U, V>, 3>>;

/// @brief
/// @tparam T The result vector type. It must be 2-dimensional and has floating-point type.
template<Concept::NumericVector3FloatingPoint T = muc::array3d>
class Gaussian3DDiagnoalFast;

template<Concept::NumericVector3FloatingPoint T>
using Gaussian3DDiagnoalFastParameter = internal::BasicGaussian3DDiagnoalParameter<T, Gaussian3DDiagnoalFast>;

template<Concept::NumericVector3FloatingPoint T>
class Gaussian3DDiagnoalFast final : public internal::Gaussian3DDiagnoalBase<Gaussian3DDiagnoalFast, T> {
private:
    using Base = internal::Gaussian3DDiagnoalBase<Gaussian3DDiagnoalFast, T>;
    using VT = typename Base::VT;

public:
    using internal::Gaussian3DDiagnoalBase<Gaussian3DDiagnoalFast, T>::Gaussian3DDiagnoalBase;

    constexpr void Reset() { fSaved = false; }

    MUSTARD_OPTIMIZE_FAST MUSTARD_ALWAYS_INLINE auto operator()(UniformRandomBitGenerator auto& g) -> auto { return (*this)(g, this->fParameter); }
    MUSTARD_OPTIMIZE_FAST MUSTARD_ALWAYS_INLINE auto operator()(UniformRandomBitGenerator auto& g, const Gaussian3DDiagnoalFastParameter<T>& p) -> T;

    constexpr auto Min() const -> T { return {std::numeric_limits<VT>::lowest(), std::numeric_limits<VT>::lowest()}; }
    constexpr auto Max() const -> T { return {std::numeric_limits<VT>::max(), std::numeric_limits<VT>::max()}; }

    static constexpr auto Stateless() { return false; }

private:
    bool fSaved{};
    VT fSavedValue{};
};

template<typename T, typename U, typename V>
Gaussian3DDiagnoalFast(std::initializer_list<T>, std::initializer_list<U>, std::initializer_list<V>) -> Gaussian3DDiagnoalFast<std::array<std::common_type_t<T, U, V>, 3>>;

} // namespace Mustard::Math::Random::inline Distribution

#include "Mustard/Math/Random/Distribution/Gaussian3DDiagnoal.inl"
