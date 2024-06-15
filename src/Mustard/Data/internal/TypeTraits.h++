#pragma once

#include "Mustard/Concept/ROOTFundamental.h++"

#include <type_traits>

namespace Mustard::Data::internal {

template<typename>
struct IsStdArray
    : std::false_type {};
template<typename T, std::size_t N>
struct IsStdArray<std::array<T, N>>
    : std::true_type {};

} // namespace Mustard::Data::internal
