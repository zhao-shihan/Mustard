#pragma once

#include <concepts>

namespace Mustard::Math {

constexpr bool IsOdd(const std::integral auto n) {
    return n & static_cast<decltype(n)>(1);
}

constexpr bool IsEven(const std::integral auto n) {
    return not IsOdd(n);
}

} // namespace Mustard::Math
