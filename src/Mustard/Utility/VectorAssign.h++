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

#include "Mustard/Concept/InputVector.h++"
#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Utility/FunctionAttribute.h++"
#include "Mustard/Utility/VectorDimension.h++"
#include "Mustard/Utility/VectorValueType.h++"

#include "muc/utility"

#include "gsl/gsl"

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace Mustard::inline Utility {

/// @brief Assign to a numeric vector using its native `operator=`.
/// @details This overload is selected when @p rhs is directly assignable to
/// @p lhs (e.g., same vector type or implicitly convertible value). It
/// simply delegates to the vector's built-in assignment operator.
/// @param lhs The target numeric vector.
/// @param rhs The source value to assign.
/// @return lvalue reference to @p lhs.
MUSTARD_ALWAYS_INLINE constexpr auto VectorAssign(Concept::NumericVectorAny auto& lhs, auto&& rhs) -> auto&
    requires std::assignable_from<decltype(lhs), decltype(rhs)> {
    return lhs = std::forward<decltype(rhs)>(rhs);
}

/// @brief Assign an input range to a numeric vector element by element.
/// @details This overload is selected when @p rhs is an input range whose
/// value type is assignable to @p lhs's element type, but @p rhs itself is
/// not directly assignable to @p lhs. Each element of the range is
/// forwarded into the corresponding element of the vector in order.
/// @param lhs The target numeric vector.
/// @param rhs An input range whose elements are to be assigned.
/// @return lvalue reference to @p lhs.
MUSTARD_ALWAYS_INLINE constexpr auto VectorAssign(Concept::NumericVectorAny auto& lhs, std::ranges::input_range auto&& rhs) -> auto&
    requires(not std::assignable_from<decltype(lhs), decltype(rhs)> and
             std::assignable_from<VectorValueType<std::decay_t<decltype(lhs)>>&, std::ranges::range_value_t<decltype(rhs)>>) {
    for (gsl::index i{};
         auto&& value : std::forward<decltype(rhs)>(rhs)) {
        lhs[i++] = std::forward<decltype(value)>(value);
    }
    return lhs;
}

/// @brief Assign an InputVector to a numeric vector element by element.
/// @details This overload is selected when @p rhs satisfies the InputVector
/// concept but is neither directly assignable to @p lhs nor satisfies the
/// input_range concept. Elements are copied individually via `operator[]`.
/// @param lhs The target numeric vector.
/// @param rhs An InputVector whose elements are to be assigned.
/// @return lvalue reference to @p lhs.
MUSTARD_ALWAYS_INLINE constexpr auto VectorAssign(Concept::NumericVectorAny auto& lhs, auto&& rhs) -> auto&
    requires(Concept::InputVectorAny<std::decay_t<decltype(rhs)>> and
             not std::assignable_from<decltype(lhs), decltype(rhs)> and
             not std::ranges::input_range<decltype(rhs)>) {
    const auto dim{muc::to_signed(VectorDimension(lhs))};
    for (gsl::index i{}; i < dim; ++i) {
        lhs[i] = std::forward<decltype(rhs)>(rhs)[i];
    }
    return lhs;
}

/// @concept VectorAssignableFrom
/// @brief Checks if a value of type @p U can be assigned to a numeric vector
/// of type @p T via `VectorAssign`.
/// @tparam T The numeric vector type (can be an lvalue or rvalue reference).
/// @tparam U The source type (can be an lvalue or rvalue reference).
/// @details Satisfied when the expression
/// `VectorAssign(lhs, std::forward<U>(rhs))` is well-formed and returns a
/// reference to @p T.
template<typename T, typename U>
concept VectorAssignableFrom = requires(T&& lhs, U&& rhs) {
    { VectorAssign(lhs, std::forward<U>(rhs)) } -> std::same_as<T&>;
};

inline namespace VectorAssignOperator {

/// @brief Shorthand operator for vector assignment.
/// @details Provides a convenient `lhs <<= rhs` syntax that delegates to
/// `VectorAssign`. Available whenever the types satisfy
/// `VectorAssignableFrom`.
/// @param lhs The target numeric vector.
/// @param rhs The source value to assign.
/// @return lvalue reference to @p lhs.
template<typename T, typename U>
    requires VectorAssignableFrom<T&&, U&&>
MUSTARD_ALWAYS_INLINE constexpr auto operator<<=(Concept::NumericVectorAny auto& lhs, auto&& rhs) -> auto& {
    return VectorAssign(lhs, std::forward<decltype(rhs)>(rhs));
}

} // namespace VectorAssignOperator

} // namespace Mustard::inline Utility
