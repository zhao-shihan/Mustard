#pragma once

#include "Mustard/Concept/ROOTFundamental.h++"

#include "RtypesCore.h"

#include "gsl/gsl"

#include <concepts>
#include <type_traits>

namespace Mustard::inline Extension::ROOTX {

template<Concept::ROOTFundamental T>
constexpr auto LeafTypeCode() -> char {
    if constexpr (std::same_as<std::decay_t<T>, gsl::zstring>) {
        return 'C';
    } else if constexpr (std::same_as<T, Char_t>) {
        return 'B';
    } else if constexpr (std::same_as<T, UChar_t>) {
        return 'b';
    } else if constexpr (std::same_as<T, Short_t>) {
        return 'S';
    } else if constexpr (std::same_as<T, UShort_t>) {
        return 's';
    } else if constexpr (std::same_as<T, Int_t>) {
        return 'I';
    } else if constexpr (std::same_as<T, UInt_t>) {
        return 'i';
    } else if constexpr (std::same_as<T, Float_t>) {
        return 'F';
    } else if constexpr (std::same_as<T, Double_t>) {
        return 'D';
    } else if constexpr (std::same_as<T, Long64_t>) {
        return 'L';
    } else if constexpr (std::same_as<T, ULong64_t>) {
        return 'l';
    } else if constexpr (std::same_as<T, Long_t>) {
        return 'G';
    } else if constexpr (std::same_as<T, ULong_t>) {
        return 'g';
    } else if constexpr (std::same_as<T, Bool_t>) {
        return 'O';
    }
}

} // namespace Mustard::inline Extension::ROOTX
