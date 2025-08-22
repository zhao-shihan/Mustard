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
#include "Mustard/Utility/VectorCast.h++"
#include "Mustard/Utility/VectorDimension.h++"
#include "Mustard/Utility/VectorValueType.h++"
#include "Mustard/gslx/index_sequence.h++"

#include "muc/math"
#include "muc/utility"

#include "gsl/gsl"

#include <cmath>
#include <concepts>
#include <type_traits>

namespace Mustard::Math {

/// @brief Computes the squared Euclidean norm of a floating-point vector
///
/// Calculates the sum of squares of all components in a numeric vector.
/// Compile-time optimized using component unpacking.
///
/// @tparam V Numeric vector type (satisfies NumericVectorFloatingPoint concept)
/// @param x Vector input (must support index-based element access)
///
/// @return Sum of squares: x₀² + x₁² + ... + xₙ²
constexpr auto NormSq(const Concept::NumericVectorFloatingPoint auto& x) {
    return [&x]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        return muc::hypot_sq(x[Is]...);
    }(gslx::make_index_sequence<VectorDimension<std::decay_t<decltype(x)>>>());
}

/// @brief Computes the Euclidean norm of a floating-point vector
///
/// Calculates the magnitude √(Σxᵢ²) for floating-point vectors.
///
/// @tparam V Numeric vector type (satisfies NumericVectorFloatingPoint concept)
/// @param x Vector input
///
/// @return Euclidean norm: √(x₁² + x₂² + ... + xₙ²)
auto Norm(const Concept::NumericVectorFloatingPoint auto& x) {
    return std::sqrt(NormSq(x));
}

/// @brief Computes squared Euclidean norm for integral vectors
///
/// Specialized version for integral vectors to prevent overflow/truncation.
/// Converts components to specified floating-point type before computation.
///
/// @tparam T Floating-point result type (default=double)
/// @tparam V Integral vector type (satisfies NumericVectorIntegral concept)
/// @param x Vector input
///
/// @return Sum of squares as floating-point value: x₀² + x₁² + ... + xₙ²
template<std::floating_point T = double>
constexpr auto NormSq(const Concept::NumericVectorIntegral auto& x) {
    return [&x]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        return muc::hypot_sq<T>(x[Is]...);
    }(gslx::make_index_sequence<VectorDimension<std::decay_t<decltype(x)>>>());
}

/// @brief Computes Euclidean norm for integral vectors
///
/// Calculates √(Σxᵢ²) for integral vectors with floating-point precision.
///
/// @tparam T Floating-point result type (default=double)
/// @tparam V Integral vector type (satisfies NumericVectorIntegral concept)
/// @param x Vector input
///
/// @return Euclidean norm as floating-point value: √(x₁² + x₂² + ... + xₙ²)
template<std::floating_point T = double>
auto Norm(const Concept::NumericVectorIntegral auto& x) {
    return std::sqrt(NormSq<T>(x));
}

} // namespace Mustard::Math
