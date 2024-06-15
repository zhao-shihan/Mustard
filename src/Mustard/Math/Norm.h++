#pragma once

#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Extension/gslx/index_sequence.h++"
#include "Mustard/Utility/VectorCast.h++"
#include "Mustard/Utility/VectorDimension.h++"
#include "Mustard/Utility/VectorValueType.h++"

#include "muc/math"
#include "muc/utility"

#include "gsl/gsl"

#include <cmath>
#include <concepts>
#include <type_traits>

namespace Mustard::Math {

constexpr auto Norm2(const Concept::NumericVectorFloatingPoint auto& x) {
    return ([&x]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        return muc::hypot2(x[Is]...);
    })(gslx::make_index_sequence<VectorDimension<std::decay_t<decltype(x)>>>());
}

auto Norm(const Concept::NumericVectorFloatingPoint auto& x) {
    return std::sqrt(Norm2(x));
}

template<std::floating_point T = double>
constexpr auto Norm2(const Concept::NumericVectorIntegral auto& x) {
    return ([&x]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        return muc::hypot2<T>(x[Is]...);
    })(gslx::make_index_sequence<VectorDimension<std::decay_t<decltype(x)>>>());
}

template<std::floating_point T = double>
auto Norm(const Concept::NumericVectorIntegral auto& x) {
    return std::sqrt(Norm2<T>(x));
}

} // namespace Mustard::Math
