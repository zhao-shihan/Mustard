// -*- C++ -*-
//
// Copyright (C) 2020-2026  Shihan Zhao and contributors
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

#include "Mustard/Utility/FunctionAttribute.h++"

#include "muc/utility"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <concepts>
#include <numbers>

namespace Mustard::inline Math::Random::inline Distribution::impl {

// In auto-vectorization we trust
// This function have been optimized to be easily vectorized by the compiler,
// do not change the code unless you know what you are doing.
template<int N, std::floating_point T>
MUSTARD_ALWAYS_INLINE auto FastCosSinOnPiPi(const std::array<T, N>& x) -> std::array<std::array<T, N>, 2> {
    constexpr auto pi{std::numbers::pi_v<T>};
    assert(std::ranges::all_of(x, [pi](auto xi) { return -pi <= xi and xi <= pi; }));
    muc::assume(std::ranges::all_of(x, [pi](auto xi) { return -pi <= xi and xi <= pi; }));
    constexpr auto halfPi{pi / 2};
    constexpr T c[]{0.99999906095491026, -0.166655545141904635,
                    0.00831190424199405764, -0.000184882511720804211};
    std::array<std::array<T, N>, 2> r;
    for (int i{}; i < N; ++i) {
        const auto y{halfPi - std::abs(x[i])};
        const auto z{halfPi - std::abs(y)};
        const auto y2{y * y};
        const auto z2{z * z};
        const auto cx{y * (c[0] + y2 * (c[1] + y2 * (c[2] + c[3] * y2)))};
        const auto sx{z * (c[0] + z2 * (c[1] + z2 * (c[2] + c[3] * z2)))};
        r[0][i] = std::copysign(cx, y);
        r[1][i] = std::copysign(sx, x[i]);
    }
    return r;
}

template<std::floating_point T>
MUSTARD_ALWAYS_INLINE auto FastCosSinOnPiPi(T x) -> std::array<T, 2> {
    const auto [cosx, sinx]{FastCosSinOnPiPi<1, T>({x})};
    return {cosx[0], sinx[0]};
}

} // namespace Mustard::inline Math::Random::inline Distribution::impl
