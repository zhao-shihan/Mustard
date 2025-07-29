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

#include "gsl/gsl"

#include <utility>

namespace Mustard::gslx {

/// @brief Sequence of `gsl::index` integers
/// @tparam Is... Compile-time integer sequence values
template<gsl::index... Is>
using index_sequence = std::integer_sequence<gsl::index, Is...>;

/// @brief Generate 0-based index sequence of length N
/// @tparam N Length of sequence to generate
template<gsl::index N>
using make_index_sequence = std::make_integer_sequence<gsl::index, N>;

/// @brief Generate index sequence for a parameter pack
/// @tparam T... Types to generate sequence for (length = sizeof...(T))
template<typename... T>
using index_sequence_for = make_index_sequence<sizeof...(T)>;

} // namespace Mustard::gslx
