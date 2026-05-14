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
#include "Mustard/Utility/VectorValueType.h++"

#include <ranges>

namespace Mustard::inline Utility {

/// @brief Get the number of elements in a numeric vector.
/// @details For types that model `std::ranges::sized_range`, returns
/// `std::ranges::size(vec)`. For fixed-size vector types (e.g., CLHEP's
/// `CLHEP::Hep3Vector`) that do not satisfy sized_range, falls back
/// to computing `sizeof(T) / sizeof(ValueType)`.
/// @tparam T The numeric vector type (deduced).
/// @param vec The numeric vector to query.
/// @return The number of elements in @p vec.
template<Concept::NumericVectorAny T>
constexpr auto VectorDimension(const T& vec) -> std::size_t {
    if constexpr (std::ranges::sized_range<T>) {
        return std::ranges::size(vec);
    } else {
        return sizeof(T) / sizeof(VectorValueType<T>);
    }
}

} // namespace Mustard::inline Utility
