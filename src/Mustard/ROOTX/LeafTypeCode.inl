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

namespace Mustard::ROOTX {

template<ROOTX::TTreePersistableFundamental T>
constexpr auto LeafTypeCode() -> char {
    if constexpr (std::same_as<std::decay_t<T>, gsl::zstring>) {
        return 'C';
    } else if constexpr (std::same_as<T, char>) {
        return 'B';
    } else if constexpr (std::same_as<T, std::uint8_t>) {
        return 'b';
    } else if constexpr (std::same_as<T, std::int16_t>) {
        return 'S';
    } else if constexpr (std::same_as<T, std::uint16_t>) {
        return 's';
    } else if constexpr (std::same_as<T, std::int32_t>) {
        return 'I';
    } else if constexpr (std::same_as<T, std::uint32_t>) {
        return 'i';
    } else if constexpr (std::same_as<T, float>) {
        return 'F';
    } else if constexpr (std::same_as<T, double>) {
        return 'D';
    } else if constexpr (std::same_as<T, std::int64_t>) {
        return 'L';
    } else if constexpr (std::same_as<T, std::uint64_t>) {
        return 'l';
    } else if constexpr (std::same_as<T, long>) {
        return 'G';
    } else if constexpr (std::same_as<T, unsigned long>) {
        return 'g';
    } else if constexpr (std::same_as<T, bool>) {
        return 'O';
    }
}

} // namespace Mustard::ROOTX
