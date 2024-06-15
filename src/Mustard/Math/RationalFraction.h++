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
