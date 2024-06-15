#pragma once

#include <type_traits>

namespace Mustard::Concept {

/// @brief Not moveable, implies non-copyable
template<typename T>
concept NonMoveable =
    requires {
        requires not std::is_constructible_v<T, const T>;
        requires not std::is_convertible_v<const T, T>;
        requires not std::is_constructible_v<T, T>;
        requires not std::is_convertible_v<T, T>;
        requires not std::is_assignable_v<T&, const T>;
        requires not std::is_assignable_v<T&, T>;
        requires not std::is_swappable_v<T>;
    };

} // namespace Mustard::Concept
