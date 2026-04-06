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

#include "gsl/gsl"

#include <concepts>
#include <cstdint>

namespace Mustard::ROOTX {

/// @brief Concept defining fundamental types persistable in ROOT TTree branches.
///
/// This concept checks whether a type `T` is one of the core fundamental types that ROOT can
/// persist in TTree. It is primarily used for template constraints where only basic data types are
/// accepted (e.g., serialization, type checking).
///
/// @tparam T Type to check against
/// @see https://root.cern/doc/master/classTTree.html for ROOT fundamental types persistable in TTree
template<typename T>
concept TTreePersistableFundamental =
    std::same_as<std::decay_t<T>, gsl::zstring> or
    std::same_as<T, char> or
    std::same_as<T, std::uint8_t> or
    std::same_as<T, std::int16_t> or
    std::same_as<T, std::uint16_t> or
    std::same_as<T, std::int32_t> or
    std::same_as<T, std::uint32_t> or
    std::same_as<T, float> or
    std::same_as<T, double> or
    std::same_as<T, std::int64_t> or
    std::same_as<T, std::uint64_t> or
    std::same_as<T, long> or
    std::same_as<T, unsigned long> or
    std::same_as<T, bool>;

} // namespace Mustard::ROOTX
