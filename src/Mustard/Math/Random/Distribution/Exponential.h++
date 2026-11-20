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

#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/Distribution/impl/FastLogOn01.h++"
#include "Mustard/Math/Random/RandomNumberDistributionBase.h++"
#include "Mustard/Utility/ClassSemanticHelper.h++"

#include "CLHEP/Random/RandomEngine.h"

#include "TRandom.h"

#include "muc/concepts"

#include <cmath>
#include <concepts>
#include <iomanip>
#include <limits>
#include <random>

namespace Mustard::inline Math::Random::inline Distribution {

namespace impl {

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
    MUSTARD_UTILITY_DECLARE_NONVIRTUAL_PROTECTED_DEFAULT_DESTRUCTOR_IN_DEFAULT_MOVEABLE(ExponentialBase)

public:
    constexpr auto Reset() -> void {}

    constexpr auto Parameter() const -> auto { return fParameter; }
    constexpr auto Expectation() const -> auto { return fParameter.Expectation(); }

    constexpr auto Parameter(const typename Base::ParameterType& p) -> void { fParameter = p; }
    constexpr auto Expectation(T expectation) -> void { fParameter.Expectation(expectation); }

    constexpr auto Min() const -> auto { return std::numeric_limits<T>::min(); }
    constexpr auto Max() const -> auto { return std::numeric_limits<T>::max(); }

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const ExponentialBase& self) -> auto& { return os << self.fParameter; }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, ExponentialBase& self) -> auto& { return is >> self.fParameter; }

protected:
    typename Base::ParameterType fParameter;
};

} // namespace impl

/// @brief Generates random floating-points of exponential distribution.
/// This version is faster than ExponentialS and they are practically the same.
/// @tparam T The type of the result.
template<std::floating_point T = double>
class Exponential;

template<std::floating_point T>
using ExponentialParameter = impl::BasicExponentialParameter<T, Exponential>;

template<std::floating_point T>
class Exponential final : public impl::ExponentialBase<Exponential, T> {
public:
    using impl::ExponentialBase<Exponential, T>::ExponentialBase;

    template<std::uniform_random_bit_generator G>
    constexpr auto operator()(G& g) -> auto { return Impl(g, this->fParameter); }
    template<std::uniform_random_bit_generator G>
    constexpr auto operator()(G& g, const ExponentialParameter<T>& p) -> auto { return Impl(g, p); }

    auto operator()(CLHEP::HepRandomEngine& g) -> auto { return Impl(g, this->fParameter); }
    auto operator()(CLHEP::HepRandomEngine& g, const ExponentialParameter<T>& p) -> auto { return Impl(g, p); }

    auto operator()(TRandom& g) -> auto { return Impl(g, this->fParameter); }
    auto operator()(TRandom& g, const ExponentialParameter<T>& p) -> auto { return Impl(g, p); }

private:
    static constexpr auto Impl(auto& g, const ExponentialParameter<T>& p) -> T;
};

template<typename T>
Exponential(T) -> Exponential<T>;

/// @brief Generates random floating-points of exponential distribution.
/// This version is slower than Gaussian2D and mainly for testing purpose.
/// @tparam T The type of the result.
template<std::floating_point T = double>
class ExponentialS;

template<std::floating_point T>
using ExponentialSParameter = impl::BasicExponentialParameter<T, ExponentialS>;

template<std::floating_point T>
class ExponentialS final : public impl::ExponentialBase<ExponentialS, T> {
public:
    using impl::ExponentialBase<ExponentialS, T>::ExponentialBase;

    template<std::uniform_random_bit_generator G>
    auto operator()(G& g) -> auto { return Impl(g, this->fParameter); }
    template<std::uniform_random_bit_generator G>
    auto operator()(G& g, const ExponentialSParameter<T>& p) -> auto { return Impl(g, p); }

    auto operator()(CLHEP::HepRandomEngine& g) -> auto { return Impl(g, this->fParameter); }
    auto operator()(CLHEP::HepRandomEngine& g, const ExponentialSParameter<T>& p) -> auto { return Impl(g, p); }

    auto operator()(TRandom& g) -> auto { return Impl(g, this->fParameter); }
    auto operator()(TRandom& g, const ExponentialSParameter<T>& p) -> auto { return Impl(g, p); }

private:
    static auto Impl(auto& g, const ExponentialSParameter<T>& p) -> T;
};

template<typename T>
ExponentialS(T) -> ExponentialS<T>;

} // namespace Mustard::inline Math::Random::inline Distribution

#include "Mustard/Math/Random/Distribution/Exponential.inl"
