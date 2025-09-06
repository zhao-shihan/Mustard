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

namespace Mustard::ROOTX {

template<ROOTX::Fundamental T>
constexpr auto LeafTypeCode() -> char {
    if constexpr (std::same_as<std::decay_t<T>, gsl::zstring>) {
        return 'C';
    } else if constexpr (std::same_as<T, Char_t>) {
        return 'B';
    } else if constexpr (std::same_as<T, UChar_t>) {
        return 'b';
    } else if constexpr (std::same_as<T, Short_t>) {
        return 'S';
    } else if constexpr (std::same_as<T, UShort_t>) {
        return 's';
    } else if constexpr (std::same_as<T, Int_t>) {
        return 'I';
    } else if constexpr (std::same_as<T, UInt_t>) {
        return 'i';
    } else if constexpr (std::same_as<T, Float_t>) {
        return 'F';
    } else if constexpr (std::same_as<T, Double_t>) {
        return 'D';
    } else if constexpr (std::same_as<T, Long64_t>) {
        return 'L';
    } else if constexpr (std::same_as<T, ULong64_t>) {
        return 'l';
    } else if constexpr (std::same_as<T, Long_t>) {
        return 'G';
    } else if constexpr (std::same_as<T, ULong_t>) {
        return 'g';
    } else if constexpr (std::same_as<T, Bool_t>) {
        return 'O';
    }
}

} // namespace Mustard::ROOTX
