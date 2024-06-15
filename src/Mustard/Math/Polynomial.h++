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
