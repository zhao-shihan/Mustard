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
#include "Mustard/Utility/VectorAssign.h++"

#include <concepts>
#include <type_traits>
#include <utility>

namespace Mustard::inline Utility {

template<Concept::NumericVectorAny T>
[[nodiscard]] T VectorCast(std::convertible_to<T> auto&& src) {
    return src;
}

template<Concept::NumericVectorAny T>
[[nodiscard]] T VectorCast(auto&& src)
    requires(not std::convertible_to<decltype(src), T>)
{
    T dst;
    VectorAssign(dst, std::forward<decltype(src)>(src));
    return dst;
}

} // namespace Mustard::inline Utility
