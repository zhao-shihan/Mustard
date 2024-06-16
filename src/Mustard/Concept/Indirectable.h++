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
    MUSTARD_CONCEPT_ACCESS_TO_MAYBE_QUALIFIED(IndirectableToMaybeReferenced)

#undef MUSTARD_CONCEPT_ACCESS_TO_MAYBE_QUALIFIED

} // namespace Mustard::Concept
