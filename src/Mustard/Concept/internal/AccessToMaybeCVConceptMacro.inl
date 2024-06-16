#define MUSTARD_CONCEPT_ACCESS_TO_MAYBE_CONST(MotherConcept) \
    MotherConcept<T, U> or                                   \
        MotherConcept<T, const U>

#define MUSTARD_CONCEPT_ACCESS_TO_MAYBE_VOLATILE(MotherConcept) \
    MotherConcept<T, U> or                                      \
        MotherConcept<T, volatile U>

#define MUSTARD_CONCEPT_ACCESS_TO_MAYBE_QUALIFIED(MotherConcept) \
    MotherConcept<T, U> or                                       \
        MotherConcept<T, const U> or                             \
        MotherConcept<T, volatile U> or                          \
        MotherConcept<T, const volatile U>
