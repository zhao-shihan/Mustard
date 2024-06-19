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
