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

#include "muc/bit"
#include "muc/utility"

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <climits>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <limits>
#include <numbers>

namespace Mustard::inline Math::Random::inline Distribution::impl {

// In auto-vectorization we trust
// This function have been optimized to be easily vectorized by the compiler,
// do not change the code unless you know what you are doing.
template<int N, std::floating_point T>
MUSTARD_ALWAYS_INLINE auto FastLogOn01(std::array<T, N> x) -> std::array<T, N> {
    assert(std::ranges::all_of(x, [](auto xi) { return 0 < xi and xi <= 1; }));
    muc::assume(std::ranges::all_of(x, [](auto xi) { return 0 < xi and xi <= 1; }));
    constexpr auto iec559{std::numeric_limits<T>::is_iec559};
    constexpr auto isF32{std::same_as<T, float>};
    constexpr auto isF64{std::same_as<T, double>};
    if constexpr (iec559 and (isF32 or isF64)) {
        using B = std::conditional_t<isF32, std::uint32_t, std::uint64_t>;
        constexpr int n{std::numeric_limits<T>::digits - 1};
        constexpr int k{muc::bit_size<T> - 1 - n};
        constexpr T c[]{9.2651671377046504, 2.73948787606301709,
                        18.5305156812078555, 10.1158584065498774};
        for (int i{}; i < N; ++i) {
            auto& xx{x[i]};
            const auto b{std::bit_cast<B>(xx)};
            muc::assume(b > 0);
            muc::assume(b < ~(~static_cast<B>(0) >> 1));
            const auto e{static_cast<int>(b >> n) - (1 << (k - 1)) + 2};        // = e+1
            xx = std::bit_cast<T>((b | ~static_cast<B>(0) << n) << 2 >> 2) - 2; // = m-2
            xx = xx * (c[0] + c[1] * xx) / (c[2] + xx * (c[3] + xx));           // = log((x+2)/2) = log(m/2)
            xx += e * std::numbers::ln2_v<T>;                                   // = log(m/2) + (e+1)*log(2)
        }
    } else {
        for (int i{}; i < N; ++i) {
            x[i] = std::log(x[i]);
        }
    }
    return x;
}

template<std::floating_point T>
MUSTARD_ALWAYS_INLINE auto FastLogOn01(T x) -> T {
    return FastLogOn01<1, T>({x})[0];
}

} // namespace Mustard::inline Math::Random::inline Distribution::impl
