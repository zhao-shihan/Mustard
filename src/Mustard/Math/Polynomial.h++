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
#include <initializer_list>
#include <limits>
#include <ranges>

namespace Mustard::Math {

template<std::floating_point T,
         std::ranges::range C = std::initializer_list<T>>
constexpr auto Polynomial(const C& coefficientList,
                          T x) -> T {
    auto c{std::ranges::crbegin(coefficientList)};
    const auto end{std::ranges::crend(coefficientList)};
    if (c == end) {
        using nl = std::numeric_limits<T>;
        return nl::has_quiet_NaN ? nl::quiet_NaN() : 0;
    }
    T p{*c++};
    while (c != end) {
        p = p * x + *c++;
    }
    return p;
}

template<std::floating_point T = double,
         std::ranges::range C = std::initializer_list<T>>
constexpr auto Polynomial(const C& coefficientList,
                          std::integral auto x) -> T {
    return Polynomial<T>(coefficientList, x);
}

} // namespace Mustard::Math
