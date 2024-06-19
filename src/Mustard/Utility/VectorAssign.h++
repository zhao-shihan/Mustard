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

#include "Mustard/Concept/FundamentalType.h++"
#include "Mustard/Concept/Indirectable.h++"
#include "Mustard/Concept/InputVector.h++"
#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Utility/VectorDimension.h++"
#include "Mustard/Utility/VectorValueType.h++"

#include "muc/utility"

#include "gsl/gsl"

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace Mustard::inline Utility {

/// @brief Assign something to a vector. If lhs = rhs is well-formed, assign
/// with operator=, else assign element by element.
/// @param lhs The vector at left-hand side.
/// @param rhs Something at right hand side.
/// @return If lhs = rhs is well-formed, returns lhs = rhs, else returns the
/// lvalue reference to lhs.
auto VectorAssign(Concept::NumericVectorAny auto& lhs, auto&& rhs) -> auto&
    requires std::assignable_from<decltype(lhs), decltype(rhs)>
{
    return lhs = std::forward<decltype(rhs)>(rhs);
}

/// @brief Assign something to a vector. If lhs = rhs is well-formed, assign
/// with operator=, else assign element by element.
/// @param lhs The vector at left-hand side.
/// @param rhs Something at right hand side.
/// @return If lhs = rhs is well-formed, returns lhs = rhs, else returns the
/// lvalue reference to lhs.
auto VectorAssign(Concept::NumericVectorAny auto& lhs, std::ranges::input_range auto&& rhs) -> auto& //
    requires(not std::assignable_from<decltype(lhs), decltype(rhs)> and
             std::assignable_from<VectorValueType<std::decay_t<decltype(lhs)>>&, std::ranges::range_value_t<decltype(rhs)>>) //
{
    for (gsl::index i = 0; auto&& value : std::forward<decltype(rhs)>(rhs)) {
        lhs[i++] = std::forward<decltype(value)>(value);
    }
    return lhs;
}

/// @brief Assign something to a vector. If lhs = rhs is well-formed, assign
/// with operator=, else assign element by element.
/// @param lhs The vector at left-hand side.
/// @param rhs Something at right hand side.
/// @return If lhs = rhs is well-formed, returns lhs = rhs, else returns the
/// lvalue reference to lhs.
auto VectorAssign(Concept::NumericVectorAny auto& lhs, auto&& rhs) -> auto& //
    requires(Concept::InputVectorAny<std::decay_t<decltype(rhs)>> and
             not std::assignable_from<decltype(lhs), decltype(rhs)> and
             not std::ranges::input_range<decltype(rhs)>) //
{
    for (gsl::index i = 0; i < muc::to_signed(VectorDimension<std::decay_t<decltype(lhs)>>); ++i) {
        lhs[i] = std::forward<decltype(rhs)>(rhs)[i];
    }
    return lhs;
}

inline namespace VectorAssignOperator {
    // clang-format off
auto operator<<=(Concept::NumericVectorAny auto& lhs, auto&& rhs) -> auto& {
    return VectorAssign(lhs, std::forward<decltype(rhs)>(rhs));
}
    // clang-format on
}

} // namespace Mustard::inline Utility
