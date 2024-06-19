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

#include "Mustard/Concept/Indirectable.h++"
#include "Mustard/Concept/Pointer.h++"
#include "Mustard/Concept/PointerAccessible.h++"
#include "Mustard/Concept/Subscriptable.h++"

#include <concepts>
#include <type_traits>

namespace Mustard::Concept {

template<typename T>
concept WeakPointerImitator =
    requires {
        requires Indirectable<T>;
        requires PointerAccessible<T>;
    };

template<typename T>
concept PointerImitator =
    requires {
        requires WeakPointerImitator<T>;
        requires Subscriptable<T>;
    };

template<typename T, typename U>
concept WeakPointerImitatorOf =
    requires {
        requires IndirectableToMaybeReferenced<T, U>;
        requires PointerAccessibleTo<T, U>;
    };

template<typename T, typename U>
concept PointerImitatorOf =
    requires {
        requires WeakPointerImitatorOf<T, U>;
        requires SubscriptableToMaybeReferenced<T, U>;
    };

#include "Mustard/Concept/internal/AccessToMaybeCVConceptMacro.inl"

template<typename T, typename U>
concept PointerImitatorOfMaybeConst =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_CONST(WeakPointerImitatorOf);

template<typename T, typename U>
concept PointerImitatorOfMaybeConstReferenced =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_CONST(PointerImitatorOf);

#undef MUSTARD_CONCEPT_ACCESS_TO_MAYBE_CONST

template<typename T, typename U>
concept PointerImitatorOfMaybeVolatile =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_VOLATILE(WeakPointerImitatorOf);

template<typename T, typename U>
concept PointerImitatorOfMaybeVolatileReferenced =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_VOLATILE(PointerImitatorOf);

#undef MUSTARD_CONCEPT_ACCESS_TO_MAYBE_VOLATILE

template<typename T, typename U>
concept PointerImitatorOfMaybeQualified =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_QUALIFIED(WeakPointerImitatorOf);

template<typename T, typename U>
concept PointerImitatorOfMaybeQualifyReferenced =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_QUALIFIED(PointerImitatorOf);

#undef MUSTARD_CONCEPT_ACCESS_TO_MAYBE_QUALIFIED

} // namespace Mustard::Concept
