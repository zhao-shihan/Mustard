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
