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
#include "Mustard/Utility/VectorDimension.h++"
#include "Mustard/Utility/VectorValueType.h++"

#include <array>
#include <tuple>

namespace Mustard::inline Utility {

/// @brief Create a zero-initialized numeric vector.
/// @details Creates an array of zero-valued elements and aggregates them into
/// the target vector type via @c std::apply. This ensures zero-initialization
/// regardless of the underlying vector type's default-construction behavior —
/// notably working around the fact that Eigen's @c T{} does not zero-initialize
/// its elements.
/// @tparam T The target numeric vector type, must satisfy
/// @c Concept::NumericVectorAny.
/// @return A zero-initialized vector of type @p T.
template<Concept::NumericVectorAny T>
[[nodiscard]] MUSTARD_ALWAYS_INLINE constexpr auto ZeroVector() -> T {
    return std::apply([](auto... zeros) { return T{zeros...}; },
                      std::array<VectorValueType<T>, VectorDimension<T>{}>{});
}

} // namespace Mustard::inline Utility
