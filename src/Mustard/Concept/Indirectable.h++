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

#include <concepts>
#include <type_traits>

namespace Mustard::Concept {

template<typename T>
concept Indirectable =
    requires(T p) {
        *p;
    };

template<typename T, typename U>
concept IndirectableTo =
    requires(T p) {
        { *p } -> std::same_as<U>;
    };

template<typename T, typename U>
concept IndirectableToMaybeReferenced =
    IndirectableTo<T, std::remove_reference_t<U>> or
    IndirectableTo<T, U> or
    IndirectableTo<T, U&>;

#include "Mustard/Concept/internal/AccessToMaybeCVConceptMacro.inl"

template<typename T, typename U>
concept IndirectableToMaybeConst =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_CONST(IndirectableTo);

template<typename T, typename U>
concept IndirectableToMaybeConstReferenced =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_CONST(IndirectableToMaybeReferenced);

#undef MUSTARD_CONCEPT_ACCESS_TO_MAYBE_CONST

template<typename T, typename U>
concept IndirectableToMaybeVolatile =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_VOLATILE(IndirectableTo);

template<typename T, typename U>
concept IndirectableToMaybeVolatileReferenced =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_VOLATILE(IndirectableToMaybeReferenced);

#undef MUSTARD_CONCEPT_ACCESS_TO_MAYBE_VOLATILE

template<typename T, typename U>
concept IndirectableToMaybeQualified =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_QUALIFIED(IndirectableTo);

template<typename T, typename U>
concept IndirectableToMaybeQualifyReferenced =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_QUALIFIED(IndirectableToMaybeReferenced);

#undef MUSTARD_CONCEPT_ACCESS_TO_MAYBE_QUALIFIED

} // namespace Mustard::Concept
