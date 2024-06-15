#pragma once

#include "Mustard/Math/RationalFraction.h++"
#include "Mustard/Utility/InlineMacro.h++"

#include "muc/bit"

#include <bit>
#include <cassert>
#include <climits>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <numbers>

namespace Mustard::Math::Random::inline Distribution::internal {

template<std::floating_point T>
MUSTARD_ALWAYS_INLINE constexpr auto FastLogForOpen01(T x) -> auto {
    assert(0 < x and x < 1);
    muc::assume(0 < x and x < 1);
    if constexpr (std::numeric_limits<T>::is_iec559) {
        using B =
            std::conditional_t<
                std::same_as<T, float>, std::uint32_t,
                std::conditional_t<
                    std::same_as<T, double>, std::uint64_t,
                    void>>;
        constexpr int n{std::numeric_limits<T>::digits - 1};
        constexpr int k{muc::bit_size<T> - 1 - n};
        const auto xBits{std::bit_cast<B>(x)};
        muc::assume(xBits > 0);
        muc::assume(xBits < ~(~static_cast<B>(0) >> 1));
        x = std::bit_cast<T>((xBits | ~static_cast<B>(0) << n) << 2 >> 2);
        const auto r{RationalFraction({1.00000000000000000000000000000000000L,
                                       0.583383967700472856709787286973478877L},
                                      {0.382629200453083135302003393260680748L,
                                       1.03060337901870614323216255936183184L,
                                       0.170537349167416694276401758784217301L},
                                      x)};
        return (x * r - r) +
               (static_cast<int>(xBits >> n) - ((1 << (k - 1)) - 1)) * std::numbers::ln2_v<T>;
    } else {
        return std::log(x);
    }
}

} // namespace Mustard::Math::Random::inline Distribution::internal
