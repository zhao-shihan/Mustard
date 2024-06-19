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

#include <concepts>
#include <type_traits>

namespace Mustard::Concept {

template<typename T>
concept Fundamental =
    std::is_fundamental_v<T>;

template<typename T>
concept Character =
    std::same_as<T, signed char> or
    std::same_as<T, unsigned char> or
    std::same_as<T, char> or
    std::same_as<T, char8_t> or
    std::same_as<T, char16_t> or
    std::same_as<T, char32_t> or
    std::same_as<T, wchar_t>;

template<typename T>
concept Arithmetic =
    std::is_arithmetic_v<T>;

} // namespace Mustard::Concept
