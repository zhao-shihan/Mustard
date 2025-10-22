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

#include "mpi.h"

#include <complex>
#include <concepts>
#include <cstdint>

namespace Mustard::Parallel {

/// @concept MPIPredefined
/// @brief Checks if a type is a predefined MPI datatype
///
/// This concept verifies that a type matches one of the fundamental datatypes
/// directly supported by MPI standard. It ensures the type can be used with
/// MPI operations without requiring custom datatype definitions.
///
/// The concept checks for decayed types (removing cv-qualifiers and references)
/// and accepts:
///   - All fundamental integer types (signed/unsigned, all sizes including char)
///   - Floating-point types (float, double, long double)
///   - Complex number types (complex<float>, complex<double>, complex<long double>)
///   - Special MPI types (MPI_Aint, MPI_Count, MPI_Offset)
///   - Character types (char, wchar_t)
///   - Fixed-width integers (std::intX_t/std::uintX_t for X=8,16,32,64)
///   - bool and std::byte
///
/// @tparam T Type to check
template<typename T>
concept MPIPredefined =
    std::same_as<std::decay_t<T>, char> or
    std::same_as<std::decay_t<T>, signed short int> or
    std::same_as<std::decay_t<T>, signed int> or
    std::same_as<std::decay_t<T>, signed long> or
    std::same_as<std::decay_t<T>, signed long long> or
    std::same_as<std::decay_t<T>, signed char> or
    std::same_as<std::decay_t<T>, unsigned char> or
    std::same_as<std::decay_t<T>, unsigned short> or
    std::same_as<std::decay_t<T>, unsigned int> or
    std::same_as<std::decay_t<T>, unsigned long> or
    std::same_as<std::decay_t<T>, unsigned long long> or
    std::same_as<std::decay_t<T>, float> or
    std::same_as<std::decay_t<T>, double> or
    std::same_as<std::decay_t<T>, long double> or
    std::same_as<std::decay_t<T>, wchar_t> or
    std::same_as<std::decay_t<T>, std::int8_t> or
    std::same_as<std::decay_t<T>, std::int16_t> or
    std::same_as<std::decay_t<T>, std::int32_t> or
    std::same_as<std::decay_t<T>, std::int64_t> or
    std::same_as<std::decay_t<T>, std::uint8_t> or
    std::same_as<std::decay_t<T>, std::uint16_t> or
    std::same_as<std::decay_t<T>, std::uint32_t> or
    std::same_as<std::decay_t<T>, std::uint64_t> or
    std::same_as<std::decay_t<T>, MPI_Aint> or
    std::same_as<std::decay_t<T>, MPI_Count> or
    std::same_as<std::decay_t<T>, MPI_Offset> or
    std::same_as<std::decay_t<T>, bool> or
    std::same_as<std::decay_t<T>, std::complex<float>> or
    std::same_as<std::decay_t<T>, std::complex<double>> or
    std::same_as<std::decay_t<T>, std::complex<long double>> or
    std::same_as<std::decay_t<T>, std::byte>;

} // namespace Mustard::Parallel
