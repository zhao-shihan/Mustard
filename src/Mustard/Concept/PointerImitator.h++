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
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_CONST(WeakPointerImitatorOf)

template<typename T, typename U>
concept PointerImitatorOfMaybeConstReferenced =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_CONST(PointerImitatorOf)

#undef MUSTARD_CONCEPT_ACCESS_TO_MAYBE_CONST

template<typename T, typename U>
concept PointerImitatorOfMaybeVolatile =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_VOLATILE(WeakPointerImitatorOf)

template<typename T, typename U>
concept PointerImitatorOfMaybeVolatileReferenced =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_VOLATILE(PointerImitatorOf)

#undef MUSTARD_CONCEPT_ACCESS_TO_MAYBE_VOLATILE

template<typename T, typename U>
concept PointerImitatorOfMaybeQualified =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_QUALIFIED(WeakPointerImitatorOf)

template<typename T, typename U>
concept PointerImitatorOfMaybeQualifyReferenced =
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_QUALIFIED(PointerImitatorOf)

#undef MUSTARD_CONCEPT_ACCESS_TO_MAYBE_QUALIFIED

} // namespace Mustard::Concept
