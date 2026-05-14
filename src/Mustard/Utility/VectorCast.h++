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
#include "Mustard/Utility/FunctionAttribute.h++"
#include "Mustard/Utility/VectorAssign.h++"
#include "Mustard/Utility/VectorDimension.h++"

#include <concepts>
#include <type_traits>
#include <utility>

namespace Mustard::inline Utility {

/// @brief Cast a value to a numeric vector type when directly convertible.
/// @details This overload is selected when @p src is implicitly convertible
/// to the target vector type @p T (e.g., identical type or a type with a
/// compatible implicit conversion). It simply forwards the source.
/// @tparam T The target numeric vector type.
/// @tparam S The source type (deduced).
/// @param src The source value to cast.
/// @return The cast value of type @p T.
template<Concept::NumericVectorAny T, typename S>
    requires std::convertible_to<S&&, T>
[[nodiscard]] MUSTARD_ALWAYS_INLINE constexpr auto VectorCast(S&& src) -> T {
    return std::forward<S>(src);
}

/// @brief Cast a value to a numeric vector type via element-by-element
/// assignment.
/// @details This overload handles the general case where @p src is not
/// directly convertible to the target vector type @p T. It default-
/// constructs a target vector, optionally resizes it to match the source
/// dimension (if resizing is supported), and then assigns elements via
/// `VectorAssign`.
/// @tparam T The target numeric vector type.
/// @tparam S The source type (deduced).
/// @param src The source value to cast.
/// @return A new vector of type @p T with data converted from @p src.
template<Concept::NumericVectorAny T, typename S>
[[nodiscard]] MUSTARD_ALWAYS_INLINE constexpr auto VectorCast(S&& src) -> T {
    T dst;
    if constexpr (requires { dst.resize(VectorDimension(src)); }) {
        dst.resize(VectorDimension(src));
    }
    VectorAssign(dst, std::forward<S>(src));
    return dst;
}

/// @concept VectorConvertibleTo
/// @brief Checks if a value of type @p T can be casted to a numeric vector
/// of type @p U via `VectorCast`.
/// @tparam T The source type (can be an lvalue or rvalue reference).
/// @tparam U The target numeric vector type.
/// @details Satisfied when the expression
/// `VectorCast<U>(std::forward<T>(src))` is well-formed and returns exactly
/// type @p U.
template<typename T, typename U>
concept VectorConvertibleTo = requires(T&& src) {
    { VectorCast<U>(std::forward<T>(src)) } -> std::same_as<U>;
};

} // namespace Mustard::inline Utility
