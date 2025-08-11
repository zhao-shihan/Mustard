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

#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Math/Random/Distribution/UniformRectangle.h++"
#include "Mustard/Math/Random/RandomNumberDistributionBase.h++"
#include "Mustard/Math/internal/FastLogOn01.h++"
#include "Mustard/Utility/InlineMacro.h++"
#include "Mustard/Utility/VectorValueType.h++"

#include "muc/array"
#include "muc/concepts"
#include "muc/math"

#include <array>
#include <concepts>
#include <iomanip>
#include <type_traits>
#include <utility>

namespace Mustard::Math::Random::inline Distribution {

namespace internal {

template<Concept::NumericVector2FloatingPoint T, template<typename> typename AGaussian2DDiagnoal>
class BasicGaussian2DDiagnoalParameter final : public DistributionParameterBase<BasicGaussian2DDiagnoalParameter<T, AGaussian2DDiagnoal>,
                                                                                AGaussian2DDiagnoal<T>> {
private:
    using VT = VectorValueType<T>;
    using Base = DistributionParameterBase<BasicGaussian2DDiagnoalParameter<T, AGaussian2DDiagnoal>,
                                           AGaussian2DDiagnoal<T>>;

public:
    constexpr BasicGaussian2DDiagnoalParameter();
    constexpr BasicGaussian2DDiagnoalParameter(std::pair<VT, VT> pX, std::pair<VT, VT> pY);

    constexpr auto MuX() const -> auto { return fMuX; }
    constexpr auto SigmaX() const -> auto { return fSigmaX; }
    constexpr auto MuY() const -> auto { return fMuY; }
    constexpr auto SigmaY() const -> auto { return fSigmaY; }

    constexpr auto MuX(VT muX) -> void { fMuX = muX; }
    constexpr auto SigmaX(VT sigmaX) -> void { fSigmaX = sigmaX; }
    constexpr auto MuY(VT muY) -> void { fMuY = muY; }
    constexpr auto SigmaY(VT sigmaY) -> void { fSigmaY = sigmaY; }

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const BasicGaussian2DDiagnoalParameter& self) -> decltype(os) { return self.StreamOutput(os); }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, BasicGaussian2DDiagnoalParameter& self) -> decltype(is) { return self.StreamInput(is); }

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
};

template<template<typename> typename ADerived, Concept::NumericVector2FloatingPoint T>
class Gaussian2DDiagnoalBase : public RandomNumberDistributionBase<ADerived<T>,
                                                                   BasicGaussian2DDiagnoalParameter<T, ADerived>,
                                                                   T> {
protected:
    using VT = VectorValueType<T>;

private:
    using Base = RandomNumberDistributionBase<ADerived<T>,
                                              BasicGaussian2DDiagnoalParameter<T, ADerived>,
                                              T>;

public:
    constexpr Gaussian2DDiagnoalBase() = default;
    constexpr Gaussian2DDiagnoalBase(std::pair<VT, VT> pX, std::pair<VT, VT> pY);
    constexpr explicit Gaussian2DDiagnoalBase(const typename Base::ParameterType& p);

protected:
    constexpr ~Gaussian2DDiagnoalBase() = default;

public:
    constexpr auto Parameter() const -> auto { return fParameter; }
    constexpr auto MuX() const -> auto { return fParameter.MuX(); }
    constexpr auto SigmaX() const -> auto { return fParameter.SigmaX(); }
    constexpr auto MuY() const -> auto { return fParameter.MuY(); }
    constexpr auto SigmaY() const -> auto { return fParameter.SigmaY(); }

    constexpr auto Parameter(const typename Base::ParameterType& p) -> void { fParameter = p; }
    constexpr auto MuX(VT muX) -> void { fParameter.MuX(muX); }
    constexpr auto SigmaX(VT sigmaX) -> void { fParameter.SigmaX(sigmaX); }
    constexpr auto MuY(VT muY) -> void { fParameter.MuY(muY); }
    constexpr auto SigmaY(VT sigmaY) -> void { fParameter.SigmaY(sigmaY); }

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const Gaussian2DDiagnoalBase& self) -> auto& { return os << self.fParameter; }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, Gaussian2DDiagnoalBase& self) -> auto& { return is >> self.fParameter; }

protected:
    typename Base::ParameterType fParameter;
};

} // namespace internal

/// @brief
/// @tparam T The result vector type. It must be 2-dimensional and has floating-point type.
template<Concept::NumericVector2FloatingPoint T = muc::array2d>
class Gaussian2DDiagnoal;

template<Concept::NumericVector2FloatingPoint T>
using Gaussian2DDiagnoalParameter = internal::BasicGaussian2DDiagnoalParameter<T, Gaussian2DDiagnoal>;

template<Concept::NumericVector2FloatingPoint T>
class Gaussian2DDiagnoal final : public internal::Gaussian2DDiagnoalBase<Gaussian2DDiagnoal, T> {
private:
    using Base = internal::Gaussian2DDiagnoalBase<Gaussian2DDiagnoal, T>;
    using VT = typename Base::VT;

public:
    using internal::Gaussian2DDiagnoalBase<Gaussian2DDiagnoal, T>::Gaussian2DDiagnoalBase;

    constexpr void Reset() {}

    MUSTARD_STRONG_INLINE auto operator()(UniformRandomBitGenerator auto& g) -> auto { return (*this)(g, this->fParameter); }
    MUSTARD_STRONG_INLINE auto operator()(UniformRandomBitGenerator auto& g, const Gaussian2DDiagnoalParameter<T>& p) -> T;

    constexpr auto Min() const -> T { return {std::numeric_limits<VT>::lowest(), std::numeric_limits<VT>::lowest()}; }
    constexpr auto Max() const -> T { return {std::numeric_limits<VT>::max(), std::numeric_limits<VT>::max()}; }

    static constexpr auto Stateless() { return true; }
};

template<typename T, typename U>
Gaussian2DDiagnoal(std::initializer_list<T>, std::initializer_list<U>) -> Gaussian2DDiagnoal<std::array<std::common_type_t<T, U>, 2>>;

/// @brief
/// @tparam T The result vector type. It must be 2-dimensional and has floating-point type.
template<Concept::NumericVector2FloatingPoint T = muc::array2d>
class Gaussian2DDiagnoalFast;

template<Concept::NumericVector2FloatingPoint T>
using Gaussian2DDiagnoalFastParameter = internal::BasicGaussian2DDiagnoalParameter<T, Gaussian2DDiagnoalFast>;

template<Concept::NumericVector2FloatingPoint T>
class Gaussian2DDiagnoalFast final : public internal::Gaussian2DDiagnoalBase<Gaussian2DDiagnoalFast, T> {
private:
    using Base = internal::Gaussian2DDiagnoalBase<Gaussian2DDiagnoalFast, T>;
    using VT = typename Base::VT;

public:
    using internal::Gaussian2DDiagnoalBase<Gaussian2DDiagnoalFast, T>::Gaussian2DDiagnoalBase;

    constexpr void Reset() {}

    auto operator()(UniformRandomBitGenerator auto& g) -> auto { return (*this)(g, this->fParameter); }
    auto operator()(UniformRandomBitGenerator auto& g, const Gaussian2DDiagnoalFastParameter<T>& p) -> T;

    MUSTARD_STRONG_INLINE constexpr auto Min() const -> T { return {std::numeric_limits<VT>::lowest(), std::numeric_limits<VT>::lowest()}; }
    MUSTARD_STRONG_INLINE constexpr auto Max() const -> T { return {std::numeric_limits<VT>::max(), std::numeric_limits<VT>::max()}; }

    static constexpr auto Stateless() { return true; }
};

template<typename T, typename U>
Gaussian2DDiagnoalFast(std::initializer_list<T>, std::initializer_list<U>) -> Gaussian2DDiagnoalFast<std::array<std::common_type_t<T, U>, 2>>;

} // namespace Mustard::Math::Random::inline Distribution

#include "Mustard/Math/Random/Distribution/Gaussian2DDiagnoal.inl"
