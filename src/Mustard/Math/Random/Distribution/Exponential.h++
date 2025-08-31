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

#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/RandomNumberDistributionBase.h++"
#include "Mustard/Math/internal/FastLogOn01.h++"
#include "Mustard/Utility/FunctionAttribute.h++"

#include "muc/concepts"

#include <cmath>
#include <concepts>
#include <iomanip>
#include <limits>

namespace Mustard::Math::Random::inline Distribution {

namespace internal {

template<std::floating_point T, template<typename> typename AExponential>
class BasicExponentialParameter final : public DistributionParameterBase<BasicExponentialParameter<T, AExponential>,
                                                                         AExponential<T>> {
private:
    using Base = DistributionParameterBase<BasicExponentialParameter<T, AExponential>,
                                           AExponential<T>>;

public:
    constexpr BasicExponentialParameter();
    constexpr BasicExponentialParameter(T expectation);

    constexpr auto Expectation() const -> auto { return fExpectation; }

    constexpr auto Expectation(T expectation) -> void { fExpectation = expectation; }

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const BasicExponentialParameter& self) -> decltype(os) { return self.StreamOutput(os); }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, BasicExponentialParameter& self) -> decltype(is) { return self.StreamInput(is); }

private:
    template<muc::character AChar>
    auto StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os);
    template<muc::character AChar>
    auto StreamInput(std::basic_istream<AChar>& is) & -> decltype(is);

private:
    T fExpectation;
};

template<template<typename> typename ADerived, std::floating_point T>
class ExponentialBase : public RandomNumberDistributionBase<ADerived<T>,
                                                            BasicExponentialParameter<T, ADerived>,
                                                            T> {
private:
    using Base = RandomNumberDistributionBase<ADerived<T>,
                                              BasicExponentialParameter<T, ADerived>,
                                              T>;

public:
    constexpr ExponentialBase() = default;
    constexpr explicit ExponentialBase(T expectation);
    constexpr explicit ExponentialBase(const typename Base::ParameterType& p);

protected:
    constexpr ~ExponentialBase() = default;

public:
    constexpr auto Reset() -> void {}

    constexpr auto Parameter() const -> auto { return fParameter; }
    constexpr auto Expectation() const -> auto { return fParameter.Expectation(); }

    constexpr auto Parameter(const typename Base::ParameterType& p) -> void { fParameter = p; }
    constexpr auto Expectation(T expectation) -> void { fParameter.Expectation(expectation); }

    constexpr auto Min() const -> auto { return std::numeric_limits<T>::min(); }
    constexpr auto Max() const -> auto { return std::numeric_limits<T>::max(); }

    static constexpr auto Stateless() -> bool { return true; }

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const ExponentialBase& self) -> auto& { return os << self.fParameter; }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, ExponentialBase& self) -> auto& { return is >> self.fParameter; }

protected:
    typename Base::ParameterType fParameter;
};

} // namespace internal

/// @brief Generates random floating-points of exponential distribution.
/// @tparam T The type of the result.
template<std::floating_point T = double>
class Exponential;

template<std::floating_point T>
using ExponentialParameter = internal::BasicExponentialParameter<T, Exponential>;

template<std::floating_point T>
class Exponential final : public internal::ExponentialBase<Exponential, T> {
public:
    using internal::ExponentialBase<Exponential, T>::ExponentialBase;

    MUSTARD_OPTIMIZE_FAST MUSTARD_ALWAYS_INLINE constexpr auto operator()(UniformRandomBitGenerator auto& g) -> auto { return (*this)(g, this->fParameter); }
    MUSTARD_OPTIMIZE_FAST MUSTARD_ALWAYS_INLINE constexpr auto operator()(UniformRandomBitGenerator auto& g, const ExponentialParameter<T>& p) -> T;
};

template<typename T>
Exponential(T) -> Exponential<T>;

/// @brief Generates random floating-points of exponential distribution. This
/// version is faster than Exponential and less accurate for a tiny bit. See
/// note.
/// @note This version uses RA2Log (P2/Q2 rational) instead of full-precision
/// Log. The average truncation error of RA2Log is O(10^-6), and the max
/// truncation error is less than 10^-5. This error will propagate to the
/// generated random numbers.
/// @tparam T The type of the result.
template<std::floating_point T = double>
class ExponentialFast;

template<std::floating_point T>
using ExponentialFastParameter = internal::BasicExponentialParameter<T, ExponentialFast>;

template<std::floating_point T>
class ExponentialFast final : public internal::ExponentialBase<ExponentialFast, T> {
public:
    using internal::ExponentialBase<ExponentialFast, T>::ExponentialBase;

    MUSTARD_OPTIMIZE_FAST MUSTARD_ALWAYS_INLINE constexpr auto operator()(UniformRandomBitGenerator auto& g) -> auto { return (*this)(g, this->fParameter); }
    MUSTARD_OPTIMIZE_FAST MUSTARD_ALWAYS_INLINE constexpr auto operator()(UniformRandomBitGenerator auto& g, const ExponentialFastParameter<T>& p) -> T;
};

template<typename T>
ExponentialFast(T) -> ExponentialFast<T>;

} // namespace Mustard::Math::Random::inline Distribution

#include "Mustard/Math/Random/Distribution/Exponential.inl"
