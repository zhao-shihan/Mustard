#pragma once

#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Utility/VectorValueType.h++"

namespace Mustard::inline Utility {

template<Concept::NumericVectorAny T>
constexpr auto VectorDimension = sizeof(T) / sizeof(VectorValueType<T>);

} // namespace Mustard::inline Utility
