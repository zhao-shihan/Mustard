#pragma once

#include "gsl/gsl"

#include <utility>

namespace Mustard::inline Extension::gslx {

template<gsl::index... Is>
using index_sequence = std::integer_sequence<gsl::index, Is...>;

template<gsl::index N>
using make_index_sequence = std::make_integer_sequence<gsl::index, N>;

template<typename... T>
using index_sequence_for = make_index_sequence<sizeof...(T)>;

} // namespace Mustard::inline Extension::gslx
