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

#include "Mustard/Compatibility/CLHEPDefectFix/Hep2VectorDivisionAssignment.h++"
#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Utility/VectorValueType.h++"

#include <concepts>
#include <cstddef>
#include <limits>
#include <type_traits>

namespace Mustard::Concept {

namespace internal {

template<typename T>
concept ExtraRequirementsForMathVector =
    requires(T u, const T v, const T w, const VectorValueType<T> c) {
        { u += v } -> std::same_as<T&>;
        { u -= v } -> std::same_as<T&>;
        { u *= c } -> std::same_as<T&>;
        { u /= c } -> std::same_as<T&>;
        { -v } -> std::convertible_to<T>;
        { v + w } -> std::convertible_to<T>;
        { v - w } -> std::convertible_to<T>;
        { c * v } -> std::convertible_to<T>;
        { v * c } -> std::convertible_to<T>;
        { v / c } -> std::convertible_to<T>;
    };

} // namespace internal

template<typename T, typename F, std::size_t N = std::numeric_limits<std::size_t>::max()>
concept MathVector =
    requires {
        requires NumericVector<T, F, N>;
        requires internal::ExtraRequirementsForMathVector<T>;
    };

template<typename T, typename F>
concept MathVector2 = MathVector<T, F, 2>;
template<typename T, typename F>
concept MathVector3 = MathVector<T, F, 3>;
template<typename T, typename F>
concept MathVector4 = MathVector<T, F, 4>;
template<typename T>
concept MathVector2F = MathVector2<T, float>;
template<typename T>
concept MathVector3F = MathVector3<T, float>;
template<typename T>
concept MathVector4F = MathVector4<T, float>;
template<typename T>
concept MathVector2D = MathVector2<T, double>;
template<typename T>
concept MathVector3D = MathVector3<T, double>;
template<typename T>
concept MathVector4D = MathVector4<T, double>;

template<typename T, std::size_t N = std::numeric_limits<std::size_t>::max()>
concept MathVectorIntegral =
    requires {
        requires NumericVectorIntegral<T, N>;
        requires internal::ExtraRequirementsForMathVector<T>;
    };

template<typename T>
concept MathVector2Integral = MathVectorIntegral<T, 2>;
template<typename T>
concept MathVector3Integral = MathVectorIntegral<T, 3>;
template<typename T>
concept MathVector4Integral = MathVectorIntegral<T, 4>;

template<typename T, std::size_t N = std::numeric_limits<std::size_t>::max()>
concept MathVectorFloatingPoint =
    requires {
        requires NumericVectorFloatingPoint<T, N>;
        requires internal::ExtraRequirementsForMathVector<T>;
    };

template<typename T>
concept MathVector2FloatingPoint = MathVectorFloatingPoint<T, 2>;
template<typename T>
concept MathVector3FloatingPoint = MathVectorFloatingPoint<T, 3>;
template<typename T>
concept MathVector4FloatingPoint = MathVectorFloatingPoint<T, 4>;

template<typename T, std::size_t N = std::numeric_limits<std::size_t>::max()>
concept MathVectorAny =
    requires {
        requires NumericVectorAny<T, N>;
        requires internal::ExtraRequirementsForMathVector<T>;
    };

template<typename T>
concept MathVector2Any = MathVectorAny<T, 2>;
template<typename T>
concept MathVector3Any = MathVectorAny<T, 3>;
template<typename T>
concept MathVector4Any = MathVectorAny<T, 4>;

} // namespace Mustard::Concept
