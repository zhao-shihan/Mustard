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

#include "Mustard/Math/Random/RandomNumberDistributionBase.h++"
#include "Mustard/Utility/FunctionAttribute.h++"

#include "muc/concepts"
#include "muc/utility"

#include <concepts>
#include <iomanip>
#include <random>
#include <type_traits>

namespace Mustard::Math::Random::inline Distribution {

namespace internal {

template<muc::arithmetic T, template<typename> typename AUniform>
class BasicUniformParameter final : public DistributionParameterBase<BasicUniformParameter<T, AUniform>, AUniform<T>> {
public:
    constexpr BasicUniformParameter();
    constexpr BasicUniformParameter(T inf, T sup);

    constexpr auto Infimum() const -> auto { return fInfimum; }
    constexpr auto Supremum() const -> auto { return fSupremum; }

    constexpr auto Infimum(T inf) -> void { fInfimum = inf; }
    constexpr auto Supremum(T sup) -> void { fSupremum = sup; }

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const BasicUniformParameter& self) -> decltype(os) { return self.StreamOutput(os); }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, BasicUniformParameter& self) -> decltype(is) { return self.StreamInput(is); }

private:
    template<muc::character AChar>
    auto StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os);
    template<muc::character AChar>
    auto StreamInput(std::basic_istream<AChar>& is) & -> decltype(is);

private:
    T fInfimum;
    T fSupremum;
};

template<template<typename> typename ADerived, muc::arithmetic T>
class UniformBase : public RandomNumberDistributionBase<ADerived<T>,
                                                        BasicUniformParameter<T, ADerived>,
                                                        T> {
private:
    using Base = RandomNumberDistributionBase<ADerived<T>,
                                              BasicUniformParameter<T, ADerived>,
                                              T>;

public:
    constexpr UniformBase() = default;
    constexpr UniformBase(T inf, T sup);
    constexpr explicit UniformBase(const typename Base::ParameterType& p);

protected:
    constexpr ~UniformBase() = default;

public:
    constexpr auto Reset() -> void {}

    constexpr auto Parameter() const -> auto { return fParameter; }
    constexpr auto Infimum() const -> auto { return fParameter.Infimum(); }
    constexpr auto Supremum() const -> auto { return fParameter.Supremum(); }

    constexpr auto Parameter(const typename Base::ParameterType& p) -> void { fParameter = p; }
    constexpr auto Infimum(T inf) -> void { fParameter.Infimum(inf); }
    constexpr auto Supremum(T sup) -> void { fParameter.Supremum(sup); }

    constexpr auto Min() const -> auto { return Infimum(); }
    constexpr auto Max() const -> auto { return Supremum(); }

    static constexpr auto Stateless() -> bool { return true; }

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const UniformBase& self) -> auto& { return os << self.fParameter; }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, UniformBase& self) -> auto& { return is >> self.fParameter; }

protected:
    typename Base::ParameterType fParameter;
};

} // namespace internal

/// @brief Generates uniform random floating-point value on a compact (including end-point) interval.
/// @tparam T The value type.
template<std::floating_point T = double>
class UniformCompact;

template<std::floating_point T>
using UniformCompactParameter = internal::BasicUniformParameter<T, UniformCompact>;

template<std::floating_point T>
class UniformCompact final : public internal::UniformBase<UniformCompact, T> {
public:
    using internal::UniformBase<UniformCompact, T>::UniformBase;

    MUSTARD_OPTIMIZE_FAST MUSTARD_ALWAYS_INLINE constexpr auto operator()(UniformRandomBitGenerator auto& g) -> auto { return (*this)(g, this->fParameter); }
    MUSTARD_OPTIMIZE_FAST MUSTARD_ALWAYS_INLINE constexpr auto operator()(UniformRandomBitGenerator auto& g, const UniformCompactParameter<T>& p) -> T;
};

template<typename T, typename U>
UniformCompact(T, U) -> UniformCompact<std::common_type_t<T, U>>;

/// @brief Generates uniform random floating-point value on an open (excluding end-point) interval.
/// @tparam T The value type.
template<std::floating_point T = double>
class UniformReal;

/// @brief Generates uniform random integral value on a interval.
/// @tparam T The value type.
template<std::integral T = int>
class UniformInteger;

/// @brief Generates uniform random value on a interval.
/// @tparam T The value type.
template<muc::arithmetic T>
using Uniform = std::conditional_t<std::floating_point<T>,
                                   UniformReal<std::conditional_t<std::floating_point<T>, T, double>>,
                                   UniformInteger<std::conditional_t<std::integral<T>, T, int>>>;

template<muc::arithmetic T>
using UniformParameter = internal::BasicUniformParameter<T, Uniform>;

/// @brief Generates uniform random floating-point value on an open (excluding end-point) interval.
/// @tparam T The value type.
template<std::floating_point T>
class UniformReal final : public internal::UniformBase<Uniform, T> {
public:
    using internal::UniformBase<Uniform, T>::UniformBase;

    MUSTARD_OPTIMIZE_FAST MUSTARD_ALWAYS_INLINE constexpr auto operator()(UniformRandomBitGenerator auto& g) -> auto { return (*this)(g, this->fParameter); }
    MUSTARD_OPTIMIZE_FAST MUSTARD_ALWAYS_INLINE constexpr auto operator()(UniformRandomBitGenerator auto& g, const UniformParameter<T>& p) -> T;
};

template<typename T, typename U>
UniformReal(T, U) -> UniformReal<std::common_type_t<T, U>>;

/// @brief Generates uniform random integral value on a interval.
/// @tparam T The value type.
template<std::integral T>
class UniformInteger final : public internal::UniformBase<Uniform, T> {
public:
    using internal::UniformBase<Uniform, T>::UniformBase;

    MUSTARD_ALWAYS_INLINE constexpr auto operator()(UniformRandomBitGenerator auto& g) -> auto { return (*this)(g, this->fParameter); }
    MUSTARD_ALWAYS_INLINE constexpr auto operator()(UniformRandomBitGenerator auto& g, const UniformParameter<T>& p) -> T;
};

template<typename T, typename U>
UniformInteger(T, U) -> UniformInteger<std::common_type_t<T, U>>;

} // namespace Mustard::Math::Random::inline Distribution

#include "Mustard/Math/Random/Distribution/Uniform.inl"
