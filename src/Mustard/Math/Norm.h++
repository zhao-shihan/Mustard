// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
//
// This file is part of Mustard, an offline software framework for HEP experiments.
//
// Mustard is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// Mustard is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// Mustard. If not, see <https://www.gnu.org/licenses/>.

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
