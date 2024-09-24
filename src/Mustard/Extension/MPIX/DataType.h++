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

#include "Mustard/Concept/MPIPredefined.h++"

#include "mpi.h"

#include "muc/utility"

#include <type_traits>

namespace Mustard::inline Extension::MPIX {

template<typename T>
    requires Concept::MPIPredefined<std::decay_t<std::remove_pointer_t<std::decay_t<T>>>>
auto DataType() -> MPI_Datatype {
    using U = std::decay_t<std::remove_pointer_t<std::decay_t<T>>>;
    if constexpr (std::same_as<U, char>) {
        return MPI_CHAR;
    } else if constexpr (std::same_as<U, signed short int>) {
        return MPI_SHORT;
    } else if constexpr (std::same_as<U, signed int>) {
        return MPI_INT;
    } else if constexpr (std::same_as<U, signed long>) {
        return MPI_LONG;
    } else if constexpr (std::same_as<U, signed long long>) {
        return MPI_LONG_LONG_INT;
        /* } else if constexpr (std::same_as<U, signed long long>) {
            return MPI_LONG_LONG; */
    } else if constexpr (std::same_as<U, signed char>) {
        return MPI_SIGNED_CHAR;
    } else if constexpr (std::same_as<U, unsigned char>) {
        return MPI_UNSIGNED_CHAR;
    } else if constexpr (std::same_as<U, unsigned short>) {
        return MPI_UNSIGNED_SHORT;
    } else if constexpr (std::same_as<U, unsigned int>) {
        return MPI_UNSIGNED;
    } else if constexpr (std::same_as<U, unsigned long>) {
        return MPI_UNSIGNED_LONG;
    } else if constexpr (std::same_as<U, unsigned long long>) {
        return MPI_UNSIGNED_LONG_LONG;
    } else if constexpr (std::same_as<U, float>) {
        return MPI_FLOAT;
    } else if constexpr (std::same_as<U, double>) {
        return MPI_DOUBLE;
    } else if constexpr (std::same_as<U, long double>) {
        return MPI_LONG_DOUBLE;
    } else if constexpr (std::same_as<U, wchar_t>) {
        return MPI_WCHAR;
    } else if constexpr (std::same_as<U, std::int8_t>) {
        return MPI_INT8_T;
    } else if constexpr (std::same_as<U, std::int16_t>) {
        return MPI_INT16_T;
    } else if constexpr (std::same_as<U, std::int32_t>) {
        return MPI_INT32_T;
    } else if constexpr (std::same_as<U, std::int64_t>) {
        return MPI_INT64_T;
    } else if constexpr (std::same_as<U, std::uint8_t>) {
        return MPI_UINT8_T;
    } else if constexpr (std::same_as<U, std::uint16_t>) {
        return MPI_UINT16_T;
    } else if constexpr (std::same_as<U, std::uint32_t>) {
        return MPI_UINT32_T;
    } else if constexpr (std::same_as<U, std::uint64_t>) {
        return MPI_UINT64_T;
    } else if constexpr (std::same_as<U, MPI_Aint>) {
        return MPI_AINT;
    } else if constexpr (std::same_as<U, MPI_Count>) {
        return MPI_COUNT;
    } else if constexpr (std::same_as<U, MPI_Offset>) {
        return MPI_OFFSET;
    } else if constexpr (std::same_as<U, bool>) {
        return MPI_CXX_BOOL;
    } else if constexpr (std::same_as<U, std::complex<float>>) {
        return MPI_CXX_FLOAT_COMPLEX;
    } else if constexpr (std::same_as<U, std::complex<double>>) {
        return MPI_CXX_DOUBLE_COMPLEX;
    } else if constexpr (std::same_as<U, std::complex<long double>>) {
        return MPI_CXX_LONG_DOUBLE_COMPLEX;
    } else {
        static_assert(muc::dependent_false<U>, "No such data type in MPI");
    }
}

template<typename T>
auto DataType(T&&) -> MPI_Datatype {
    return DataType<T>();
}

} // namespace Mustard::inline Extension::MPIX
