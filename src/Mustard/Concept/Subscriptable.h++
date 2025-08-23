// -*- C++ -*-
//
// Copyright 2020-2025  The Mustard development team
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

#include "gsl/gsl"

#include <concepts>
#include <cstddef>
#include <type_traits>

namespace Mustard::Concept {

template<typename T>
concept Subscriptable =
    requires(T v, gsl::index i) {
        v[i];
    };

template<typename T, typename U>
concept SubscriptableTo =
    requires(T v, gsl::index i) {
        { v[i] } -> std::same_as<U>;
    };

template<typename T, typename U>
concept SubscriptableToMaybeReferenced =
    SubscriptableTo<T, std::remove_reference_t<U>> or
    SubscriptableTo<T, U> or
    SubscriptableTo<T, U&>;

#include "Mustard/Concept/internal/AccessToMaybeCVConceptMacro.inl"

template<typename T, typename U>
concept SubscriptableToMaybeConst =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_CONST(SubscriptableTo);

template<typename T, typename U>
concept SubscriptableToMaybeConstReferenced =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_CONST(SubscriptableToMaybeReferenced);

#undef MUSTARD_CONCEPT_ACCESS_TO_MAYBE_CONST

template<typename T, typename U>
concept SubscriptableToMaybeVolatile =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_VOLATILE(SubscriptableTo);

template<typename T, typename U>
concept SubscriptableToMaybeVolatileReferenced =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_VOLATILE(SubscriptableToMaybeReferenced);

#undef MUSTARD_CONCEPT_ACCESS_TO_MAYBE_VOLATILE

template<typename T, typename U>
concept SubscriptableToMaybeQualified =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_QUALIFIED(SubscriptableTo);

template<typename T, typename U>
concept SubscriptableToMaybeQualifyReferenced =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_QUALIFIED(SubscriptableToMaybeReferenced);

#undef MUSTARD_CONCEPT_ACCESS_TO_MAYBE_QUALIFIED

} // namespace Mustard::Concept
