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

#include "Mustard/Math/Polynomial.h++"

#include <concepts>
#include <initializer_list>
#include <ranges>

namespace Mustard::Math {

template<std::floating_point T,
         std::ranges::range A = std::initializer_list<T>,
         std::ranges::range B = std::initializer_list<T>>
constexpr auto RationalFraction(const A& numerator,
                                const B& denominator,
                                T x) -> T {
    return Polynomial(numerator, x) / Polynomial(denominator, x);
}

template<std::floating_point T = double,
         std::ranges::range A = std::initializer_list<T>,
         std::ranges::range B = std::initializer_list<T>>
constexpr auto RationalFraction(const A& numerator,
                                const B& denominator,
                                std::integral auto x) -> T {
    return RationalFraction<T>(numerator, denominator, x);
}

} // namespace Mustard::Math
