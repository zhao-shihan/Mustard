#pragma once

#include "Mustard/Concept/InputVector.h++"

namespace Mustard::inline Utility {

template<Concept::InputVectorAny T>
using VectorValueType = std::remove_reference_t<decltype(std::declval<T&>()[std::declval<gsl::index>()])>;

} // namespace Mustard::inline Utility
