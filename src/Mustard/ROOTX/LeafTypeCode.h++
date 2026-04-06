// -*- C++ -*-
//
// Copyright (C) 2020-2025  Mustard developers
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

#include "Mustard/ROOTX/TTreePersistableFundamental.h++"

#include "gsl/gsl"

#include <concepts>
#include <cstdint>
#include <type_traits>

namespace Mustard::ROOTX {

/// @brief Returns ROOT TLeaf type character code for fundamental types
///
/// Maps ROOT's fundamental types to their corresponding TLeaf type codes.
/// This is a compile-time function used for ROOT I/O operations.
///
/// @tparam T Type to get the leaf code for (must satisfy ROOTX::TTreePersistableFundamental)
///
/// @return Single-character code used by ROOT's TLeaf system:
///   - 'C': gsl::zstring (C-style string)
///   - 'B': char (character - just char, not always signed 8-bit integer)
///   - 'b': std::uint8_t (unsigned 8-bit integer)
///   - 'S': std::int16_t (signed 16-bit integer)
///   - 's': std::uint16_t (unsigned 16-bit integer)
///   - 'I': std::int32_t (signed 32-bit integer)
///   - 'i': std::uint32_t (unsigned 32-bit integer)
///   - 'F': float (single-precision float)
///   - 'D': double (double-precision float)
///   - 'L': std::int64_t (signed 64-bit integer)
///   - 'l': std::uint64_t (unsigned 64-bit integer)
///   - 'G': long (signed long - architecture dependent)
///   - 'g': unsigned long (unsigned long - architecture dependent)
///   - 'O': bool (boolean type)
///
/// @note The function is constexpr and evaluated at compile-time
/// @see TLeaf, ROOT data types documentation
template<ROOTX::TTreePersistableFundamental T>
constexpr auto LeafTypeCode() -> char;

} // namespace Mustard::ROOTX

#include "Mustard/ROOTX/LeafTypeCode.inl"
