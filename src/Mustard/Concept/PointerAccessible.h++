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

#include "Mustard/Concept/Pointer.h++"

#include <concepts>
#include <type_traits>

namespace Mustard::Concept {

template<typename T>
concept PointerAccessible =
    std::is_pointer_v<T> or
    requires(T p) {
        p.operator->();
    };

template<typename T, typename U>
concept PointerAccessibleTo =
    PointerOf<T, U> or
    requires(T p) {
        { p.operator->() } -> std::same_as<std::add_pointer_t<U>>;
    };

#include "Mustard/Concept/internal/AccessToMaybeCVConceptMacro.inl"

template<typename T, typename U>
concept PointerAccessibleToMaybeConst =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_CONST(PointerAccessibleTo);

#undef MUSTARD_CONCEPT_ACCESS_TO_MAYBE_CONST

template<typename T, typename U>
concept PointerAccessibleToMaybeVolatile =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_VOLATILE(PointerAccessibleTo);

#undef MUSTARD_CONCEPT_ACCESS_TO_MAYBE_VOLATILE

template<typename T, typename U>
concept PointerAccessibleToMaybeQualified =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_QUALIFIED(PointerAccessibleTo);

#undef MUSTARD_CONCEPT_ACCESS_TO_MAYBE_QUALIFIED

} // namespace Mustard::Concept
