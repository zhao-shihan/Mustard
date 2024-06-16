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
