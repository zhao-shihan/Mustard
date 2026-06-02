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
/// @tparam T The target numeric vector type.
/// @tparam U The source type.
/// @param lhs The target numeric vector.
/// @param rhs The source value to assign.
/// @return lvalue reference to @p lhs.
template<Concept::NumericVectorAny T, typename U>
    requires std::assignable_from<T&, U&&>
MUSTARD_ALWAYS_INLINE constexpr auto VectorAssign(T& lhs, U&& rhs) -> T& {
    return lhs = std::forward<U>(rhs);
}

/// @brief Assign an input range to a numeric vector element by element.
/// @details This overload is selected when @p rhs satisfies
/// `std::ranges::input_range` but is not directly assignable to @p lhs as a
/// whole, and the range's value type is assignable to @p lhs's element type.
/// Each element of the range is forwarded into the corresponding element of
/// the vector in order.
/// @tparam T The target numeric vector type.
/// @tparam U The source range type.
/// @param lhs The target numeric vector.
/// @param rhs An input range whose elements are to be assigned.
/// @return lvalue reference to @p lhs.
template<Concept::NumericVectorAny T, std::ranges::input_range U>
    requires(not std::assignable_from<T&, U &&> and
             std::assignable_from<VectorValueType<T>&, std::ranges::range_value_t<U>>)
MUSTARD_ALWAYS_INLINE constexpr auto VectorAssign(T& lhs, U&& rhs) -> T& {
    for (gsl::index i{};
         auto&& value : std::forward<U>(rhs)) {
        lhs[i++] = muc::forward_like<U>(value);
    }
    return lhs;
}

/// @brief Assign an InputVector to a numeric vector element by element.
/// @details This overload is selected when @p rhs satisfies the InputVector
/// concept, but neither the whole vector is directly assignable to @p lhs
/// nor are the element types compatible for range-based assignment. Elements
/// are copied individually via `operator[]` using the vector's compile-time
/// dimension.
/// @tparam T The target numeric vector type.
/// @tparam U The source InputVector type.
/// @param lhs The target numeric vector.
/// @param rhs An InputVector whose elements are to be assigned.
/// @return lvalue reference to @p lhs.
template<Concept::NumericVectorAny T, typename U>
    requires(Concept::InputVectorAny<std::decay_t<U>> and
             not std::assignable_from<T&, U &&> and
             not std::ranges::input_range<U>)
MUSTARD_ALWAYS_INLINE constexpr auto VectorAssign(T& lhs, U&& rhs) -> T& {
    constexpr auto dim{muc::to_signed(VectorDimension<std::decay_t<decltype(lhs)>>::value)};
    for (gsl::index i{}; i < dim; ++i) {
        lhs[i] = rhs[i];
    }
    return lhs;
}

/// @concept VectorAssignableFrom
/// @brief Checks if a value of type @p U can be assigned to a numeric vector
/// of type @p T via `VectorAssign`.
/// @tparam T The numeric vector type.
/// @tparam U The source type.
/// @details Satisfied when the expression
/// `VectorAssign(lhs, std::forward<U>(rhs))` is well-formed for an lvalue
/// @p lhs of type @p T and a forwarded reference @p rhs of type @p U, and
/// the result is a reference to @p T.
template<typename T, typename U>
concept VectorAssignableFrom = requires(T& lhs, U&& rhs) {
    { VectorAssign(lhs, std::forward<U>(rhs)) } -> std::same_as<T&>;
};

inline namespace VectorAssignOperator {

/// @brief Shorthand operator for vector assignment.
/// @details Provides a convenient `lhs <<= rhs` syntax that delegates to
/// `VectorAssign`. Available whenever the types satisfy
/// `VectorAssignableFrom`.
/// @tparam T The target numeric vector type.
/// @tparam U The source type.
/// @param lhs The target numeric vector.
/// @param rhs The source value to assign.
/// @return lvalue reference to @p lhs.
template<Concept::NumericVectorAny T, typename U>
    requires VectorAssignableFrom<T&, U&&>
MUSTARD_ALWAYS_INLINE constexpr auto operator<<=(T& lhs, U&& rhs) -> T& {
    return VectorAssign(lhs, std::forward<U>(rhs));
}

} // namespace VectorAssignOperator

} // namespace Mustard::inline Utility
