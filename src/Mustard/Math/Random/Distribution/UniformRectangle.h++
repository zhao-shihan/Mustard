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
#include "Mustard/Math/Random/Distribution/Joint.h++"
#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/RandomNumberDistributionBase.h++"
#include "Mustard/Utility/ClassSemanticHelper.h++"
#include "Mustard/Utility/VectorValueType.h++"

#include "muc/array"

#include <array>
#include <concepts>
#include <iomanip>
#include <random>
#include <type_traits>

namespace Mustard::inline Math::Random::inline Distribution {

namespace impl {

template<Concept::NumericVector2Any T, template<typename> typename AUniformRectangle, template<typename> typename AUniform>
class BasicUniformRectangleParameter final : public JointParameterInterface<BasicUniformRectangleParameter<T, AUniformRectangle, AUniform>,
                                                                            AUniformRectangle<T>,
                                                                            AUniform<VectorValueType<T>>,
                                                                            AUniform<VectorValueType<T>>> {
private:
    using Scalar = VectorValueType<T>;
    using Base = JointParameterInterface<BasicUniformRectangleParameter<T, AUniformRectangle, AUniform>,
                                         AUniformRectangle<T>,
                                         AUniform<Scalar>,
                                         AUniform<Scalar>>;

public:
    using Base::Base;

    constexpr auto ParameterX() const -> auto { return this->template Parameter<0>(); }
    constexpr auto InfimumX() const -> auto { return ParameterX().Infimum(); }
    constexpr auto SupremumX() const -> auto { return ParameterX().Supremum(); }
    constexpr auto ParameterY() const -> auto { return this->template Parameter<1>(); }
    constexpr auto InfimumY() const -> auto { return ParameterY().Infimum(); }
    constexpr auto SupremumY() const -> auto { return ParameterY().Supremum(); }

    constexpr auto ParameterX(const typename AUniform<Scalar>::ParameterType x) -> void { Parameter<0>(x); }
    constexpr auto InfimumX(Scalar infX) -> void { ParameterX({infX, SupremumX()}); }
    constexpr auto SupremumX(Scalar supX) -> void { ParameterX({InfimumX(), supX}); }
    constexpr auto ParameterY(const typename AUniform<Scalar>::ParameterType y) -> void { Parameter<1>(y); }
    constexpr auto InfimumY(Scalar infY) -> void { ParameterY({infY, SupremumY()}); }
    constexpr auto SupremumY(Scalar supY) -> void { ParameterY({InfimumY(), supY}); }
};

template<template<typename> typename ADerived, Concept::NumericVector2Any T, template<typename> typename AUniform>
class UniformRectangleBase : public JointInterface<ADerived<T>,
                                                   BasicUniformRectangleParameter<T, ADerived, AUniform>,
                                                   T,
                                                   AUniform<VectorValueType<T>>,
                                                   AUniform<VectorValueType<T>>> {
private:
    using Scalar = VectorValueType<T>;
    using Base = JointInterface<ADerived<T>,
                                BasicUniformRectangleParameter<T, ADerived, AUniform>,
                                T,
                                AUniform<Scalar>,
                                AUniform<Scalar>>;

public:
    using Base::Base;

protected:
    MUSTARD_UTILITY_DECLARE_NONVIRTUAL_PROTECTED_DEFAULT_DESTRUCTOR_IN_DEFAULT_MOVEABLE(UniformRectangleBase)

public:
    constexpr auto ParameterX() const -> auto { return Base::template Parameter<0>(); }
    constexpr auto InfimumX() const -> auto { return ParameterX().Infimum(); }
    constexpr auto SupremumX() const -> auto { return ParameterX().Supremum(); }
    constexpr auto ParameterY() const -> auto { return Base::template Parameter<1>(); }
    constexpr auto InfimumY() const -> auto { return ParameterY().Infimum(); }
    constexpr auto SupremumY() const -> auto { return ParameterY().Supremum(); }

    constexpr auto ParameterX(const typename AUniform<Scalar>::ParameterType& x) -> void { Base::template Parameter<0>(x); }
    constexpr auto InfimumX(Scalar infX) -> void { ParameterX({infX, SupremumX()}); }
    constexpr auto SupremumX(Scalar supX) -> void { ParameterX({InfimumX(), supX}); }
    constexpr auto ParameterY(const typename AUniform<Scalar>::ParameterType& y) -> void { Base::template Parameter<1>(y); }
    constexpr auto InfimumY(Scalar infY) -> void { ParameterY({infY, SupremumY()}); }
    constexpr auto SupremumY(Scalar supY) -> void { ParameterY({InfimumY(), supY}); }
};

} // namespace impl

/// @brief Generates 2D uniform random vector on [a, b) x [c, d) rectangular region.
/// @tparam T The result vector type. It must be 2-dimensional and has floating-point type.
template<Concept::NumericVector2FloatingPoint T = muc::array2d>
class UniformRealRectangle;

/// @brief Generates 2D uniform random integral vector on a rectangular region.
/// @tparam T The result vector type. It must be 2-dimensional and has integral type.
template<Concept::NumericVector2Integral T = muc::array2i>
class UniformIntegerRectangle;

/// @brief Generates 2D uniform random vector on a rectangular region.
/// @tparam T The result vector type.
template<Concept::NumericVector2Any T>
using UniformRectangle = std::conditional_t<std::floating_point<VectorValueType<T>>,
                                            UniformRealRectangle<std::conditional_t<std::floating_point<VectorValueType<T>>, T, muc::array2d>>,
                                            UniformIntegerRectangle<std::conditional_t<std::integral<VectorValueType<T>>, T, muc::array2i>>>;

template<Concept::NumericVector2Any T>
using UniformRectangleParameter = impl::BasicUniformRectangleParameter<T, UniformRectangle, Uniform>;

template<Concept::NumericVector2FloatingPoint T>
class UniformRealRectangle final : public impl::UniformRectangleBase<UniformRealRectangle, T, Uniform> {
public:
    using impl::UniformRectangleBase<UniformRealRectangle, T, Uniform>::UniformRectangleBase;
};

template<typename T, typename U>
UniformRealRectangle(std::initializer_list<T>, std::initializer_list<U>) -> UniformRealRectangle<std::array<std::common_type_t<T, U>, 2>>;

template<Concept::NumericVector2Integral T>
class UniformIntegerRectangle final : public impl::UniformRectangleBase<UniformIntegerRectangle, T, Uniform> {
public:
    using impl::UniformRectangleBase<UniformIntegerRectangle, T, Uniform>::UniformRectangleBase;
};

template<typename T, typename U>
UniformIntegerRectangle(std::initializer_list<T>, std::initializer_list<U>) -> UniformIntegerRectangle<std::array<std::common_type_t<T, U>, 2>>;

} // namespace Mustard::inline Math::Random::inline Distribution
