#pragma once

#include "RtypesCore.h"

#include "gsl/gsl"

#include <concepts>
#include <type_traits>

namespace Mustard::Concept {

template<typename T>
concept ROOTFundamental =
    std::same_as<std::decay_t<T>, gsl::zstring> or
    std::same_as<T, Char_t> or
    std::same_as<T, UChar_t> or
    std::same_as<T, Short_t> or
    std::same_as<T, UShort_t> or
    std::same_as<T, Int_t> or
    std::same_as<T, UInt_t> or
    std::same_as<T, Float_t> or
    std::same_as<T, Double_t> or
    std::same_as<T, Long64_t> or
    std::same_as<T, ULong64_t> or
    std::same_as<T, Long_t> or
    std::same_as<T, ULong_t> or
    std::same_as<T, Bool_t>;

/* template<typename T>
concept ROOTFundamentalArray =
    std::same_as<std::decay_t<T>, Char_t*> or
    std::same_as<std::decay_t<T>, UChar_t*> or
    std::same_as<std::decay_t<T>, Short_t*> or
    std::same_as<std::decay_t<T>, UShort_t*> or
    std::same_as<std::decay_t<T>, Int_t*> or
    std::same_as<std::decay_t<T>, UInt_t*> or
    std::same_as<std::decay_t<T>, Float_t*> or
    std::same_as<std::decay_t<T>, Double_t*> or
    std::same_as<std::decay_t<T>, Long64_t*> or
    std::same_as<std::decay_t<T>, ULong64_t*> or
    std::same_as<std::decay_t<T>, Long_t*> or
    std::same_as<std::decay_t<T>, ULong_t*> or
    std::same_as<std::decay_t<T>, Bool_t*>;

template<typename T>
concept ROOTFundamentalWithArray =
    ROOTFundamental<T> or ROOTFundamentalArray<T>; */

} // namespace Mustard::Concept
