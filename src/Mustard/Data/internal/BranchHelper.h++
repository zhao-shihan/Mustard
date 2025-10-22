// -*- C++ -*-
//
// Copyright (C) 2020-2025  Mustard developers
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

#include "Mustard/Data/Tuple.h++"
#include "Mustard/Data/internal/TypeTraits.h++"
#include "Mustard/ROOTX/Fundamental.h++"
#include "Mustard/gslx/index_sequence.h++"

#include "TTree.h"

#include "muc/ceta_string"
#include "muc/concepts"

#include "gsl/gsl"

#include <concepts>
#include <memory>
#include <tuple>
#include <type_traits>

namespace Mustard::Data::internal {

template<muc::instantiated_from<Tuple> ATuple>
class BranchHelper {
public:
    BranchHelper(ATuple& tuple);

    template<muc::ceta_string AName>
    auto CreateBranch(std::derived_from<TTree> auto& tree) -> TBranch*;
    template<muc::ceta_string AName>
    auto ConnectBranch(std::derived_from<TTree> auto& tree) -> std::pair<int, TBranch*>;
    template<muc::ceta_string AName>
    auto ConnectBranchNoCheck(std::derived_from<TTree> auto& tree) -> TBranch*;

private:
    ATuple* fTuple;
    decltype([]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        return std::tuple<typename std::tuple_element_t<Is, ATuple>::Type*...>{};
    }(gslx::make_index_sequence<ATuple::Size()>{})) fClassPointer;
};

} // namespace Mustard::Data::internal

#include "Mustard/Data/internal/BranchHelper.inl"
