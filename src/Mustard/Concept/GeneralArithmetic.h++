#pragma once

#include "Mustard/Concept/MathVector.h++"

#include <type_traits>

namespace Mustard::Concept {

template<typename T>
concept GeneralArithmetic =
    std::is_arithmetic_v<T> or
    MathVectorAny<T>;

} // namespace Mustard::Concept
