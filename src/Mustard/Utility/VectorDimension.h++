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

#include <concepts>
#include <cstddef>
#include <type_traits>

namespace Mustard::inline Utility {

namespace impl {

template<typename T>
concept HasFixedSizeAtCompileTime = requires {
    { T::SizeAtCompileTime } -> std::convertible_to<std::size_t>;
    requires(T::SizeAtCompileTime > 0);
};

template<typename T, bool = HasFixedSizeAtCompileTime<T>>
struct VectorDimensionImpl;

template<typename T>
struct VectorDimensionImpl<T, true>
    : std::integral_constant<std::size_t, T::SizeAtCompileTime> {};

template<typename T>
    requires(alignof(T) == sizeof(VectorValueType<T>))
struct VectorDimensionImpl<T, false>
    : std::integral_constant<std::size_t, sizeof(T) / sizeof(VectorValueType<T>)> {};

} // namespace impl

/// @brief Compile-time trait to obtain the number of elements in a numeric vector type.
/// @details Computes the dimension (element count) of a fixed-size numeric vector
/// type @p T. If @p T provides a static @c SizeAtCompileTime member with a positive
/// value (e.g. Eigen fixed-size vectors), that value is used directly. Otherwise,
/// the dimension is determined by dividing the total size of the vector object by
/// the size of its underlying value type. The result is available as the static
/// member `value` (inherited from `std::integral_constant`).
///
/// This trait works with any type satisfying @ref Concept::NumericVectorAny,
/// such as ROOT::Math::XYZVector, CLHEP::Hep3Vector, Eigen::Vector, or similar
/// fixed-size numeric vector types from other libraries.
///
/// @tparam T A type satisfying @ref Concept::NumericVectorAny.
template<Concept::NumericVectorAny T>
struct VectorDimension : impl::VectorDimensionImpl<T> {};

} // namespace Mustard::inline Utility
