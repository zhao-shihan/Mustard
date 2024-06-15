#pragma once

#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Utility/VectorAssign.h++"

#include <concepts>
#include <type_traits>
#include <utility>

namespace Mustard::inline Utility {

template<Concept::NumericVectorAny T>
[[nodiscard]] T VectorCast(std::convertible_to<T> auto&& src) {
    return src;
}

template<Concept::NumericVectorAny T>
[[nodiscard]] T VectorCast(auto&& src)
    requires(not std::convertible_to<decltype(src), T>)
{
    T dst;
    VectorAssign(dst, std::forward<decltype(src)>(src));
    return dst;
}

} // namespace Mustard::inline Utility
