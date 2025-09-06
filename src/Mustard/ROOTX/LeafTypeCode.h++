// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

#include "Mustard/ROOTX/Fundamental.h++"

#include "RtypesCore.h"

#include "gsl/gsl"

#include <concepts>
#include <type_traits>

namespace Mustard::ROOTX {

/// @brief Returns ROOT TLeaf type character code for fundamental types
///
/// Maps ROOT's fundamental types to their corresponding TLeaf type codes.
/// This is a compile-time function used for ROOT I/O operations.
///
/// @tparam T Type to get the leaf code for (must satisfy ROOTX::Fundamental)
///
/// @return Single-character code used by ROOT's TLeaf system:
///   - 'C': gsl::zstring (C-style string)
///   - 'B': Char_t (signed character)
///   - 'b': UChar_t (unsigned character)
///   - 'S': Short_t (signed short)
///   - 's': UShort_t (unsigned short)
///   - 'I': Int_t (signed int)
///   - 'i': UInt_t (unsigned int)
///   - 'F': Float_t (single-precision float)
///   - 'D': Double_t (double-precision float)
///   - 'L': Long64_t (signed 64-bit int)
///   - 'l': ULong64_t (unsigned 64-bit int)
///   - 'G': Long_t (signed long - architecture dependent)
///   - 'g': ULong_t (unsigned long - architecture dependent)
///   - 'O': Bool_t (boolean type)
///
/// @note The function is constexpr and evaluated at compile-time
/// @warning Type must satisfy the ROOTFundamental concept
/// @see TLeaf, ROOT data types documentation
template<ROOTX::Fundamental T>
constexpr auto LeafTypeCode() -> char;

} // namespace Mustard::ROOTX

#include "Mustard/ROOTX/LeafTypeCode.inl"
