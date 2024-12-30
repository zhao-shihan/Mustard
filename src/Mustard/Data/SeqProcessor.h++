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

#include "Mustard/Data/RDFEventSplit.h++"
#include "Mustard/Data/TakeFrom.h++"
#include "Mustard/Data/internal/ProcessorBase.h++"
#include "Mustard/Extension/ROOTX/RDataFrame.h++"
#include "Mustard/Extension/gslx/index_sequence.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "muc/concepts"
#include "muc/numeric"

#include "fmt/format.h"

#include <algorithm>
#include <array>
#include <functional>
#include <limits>
#include <memory>
#include <ranges>
#include <string_view>
#include <utility>
#include <vector>

namespace Mustard::Data {

/// @brief A sequential data processor.
class SeqProcessor : public internal::ProcessorBase<unsigned> {
public:
    SeqProcessor(Index batchSizeProposal = 100'000);

    template<TupleModelizable... Ts>
    auto Process(ROOT::RDF::RNode rdf,
                 std::invocable<std::shared_ptr<Tuple<Ts...>>&> auto&& F) -> Index;

    template<TupleModelizable... Ts>
    auto Process(ROOT::RDF::RNode rdf, std::string eventIDBranchName,
                 std::invocable<std::vector<std::shared_ptr<Tuple<Ts...>>>&> auto&& F) -> Index;
    template<TupleModelizable... Ts>
    auto Process(ROOT::RDF::RNode rdf, const std::vector<Index>& eventSplit,
                 std::invocable<std::vector<std::shared_ptr<Tuple<Ts...>>>&> auto&& F) -> Index;

    template<muc::instantiated_from<TupleModel>... Ts>
    auto Process(std::array<ROOT::RDF::RNode, sizeof...(Ts)> rdf,
                 std::string eventIDBranchName,
                 std::invocable<std::vector<std::shared_ptr<Tuple<Ts>>>&...> auto&& F) -> Index;
    template<muc::instantiated_from<TupleModel>... Ts>
    auto Process(std::array<ROOT::RDF::RNode, sizeof...(Ts)> rdf,
                 std::vector<std::string> eventIDBranchName,
                 std::invocable<std::vector<std::shared_ptr<Tuple<Ts>>>&...> auto&& F) -> Index;
    template<muc::instantiated_from<TupleModel>... Ts>
    auto Process(std::array<ROOT::RDF::RNode, sizeof...(Ts)> rdf,
                 const std::vector<std::array<RDFEntryRange, sizeof...(Ts)>>& eventSplit,
                 std::invocable<std::vector<std::shared_ptr<Tuple<Ts>>>&...> auto&& F) -> Index;
};

} // namespace Mustard::Data

#include "Mustard/Data/SeqProcessor.inl"
