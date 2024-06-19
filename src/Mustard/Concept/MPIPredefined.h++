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

#include "mpi.h"

#include <complex>
#include <concepts>
#include <cstdint>

namespace Mustard::Concept {

template<typename T>
concept MPIPredefined =
    std::same_as<T, char> or
    std::same_as<T, signed short int> or
    std::same_as<T, signed int> or
    std::same_as<T, signed long> or
    std::same_as<T, signed long long> or
    std::same_as<T, signed long long> or
    std::same_as<T, signed char> or
    std::same_as<T, unsigned char> or
    std::same_as<T, unsigned short> or
    std::same_as<T, unsigned int> or
    std::same_as<T, unsigned long> or
    std::same_as<T, unsigned long long> or
    std::same_as<T, float> or
    std::same_as<T, double> or
    std::same_as<T, long double> or
    std::same_as<T, wchar_t> or
    std::same_as<T, std::int8_t> or
    std::same_as<T, std::int16_t> or
    std::same_as<T, std::int32_t> or
    std::same_as<T, std::int64_t> or
    std::same_as<T, std::uint8_t> or
    std::same_as<T, std::uint16_t> or
    std::same_as<T, std::uint32_t> or
    std::same_as<T, std::uint64_t> or
    std::same_as<T, MPI_Aint> or
    std::same_as<T, MPI_Count> or
    std::same_as<T, MPI_Offset> or
    std::same_as<T, bool> or
    std::same_as<T, std::complex<float>> or
    std::same_as<T, std::complex<double>> or
    std::same_as<T, std::complex<long double>>;

} // namespace Mustard::Concept
