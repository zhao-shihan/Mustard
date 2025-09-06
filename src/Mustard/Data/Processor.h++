// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

#include "Mustard/Data/AsyncReader.h++"
#include "Mustard/Data/RDFEventSplit.h++"
#include "Mustard/Data/TakeFrom.h++"
#include "Mustard/Data/Tuple.h++"
#include "Mustard/Data/TupleModel.h++"
#include "Mustard/Data/internal/ProcessorBase.h++"
#include "Mustard/Execution/Executor.h++"
#include "Mustard/IO/PrettyLog.h++"

#include "ROOT/RDataFrame.hxx"

#include "mplr/mplr.hpp"

#include "muc/concepts"
#include "muc/ptrvec"
#include "muc/utility"

#include "gsl/gsl"

#include "fmt/core.h"

#include <algorithm>
#include <cmath>
#include <concepts>
#include <functional>
#include <future>
#include <memory>
#include <numeric>
#include <ranges>
#include <string_view>
#include <utility>
#include <vector>

namespace Mustard::Data {

/// @brief A distributed data processor.
/// @tparam AExecutor Underlying MPI executor type.
template<muc::instantiated_from<Executor> AExecutor = Executor<gsl::index>>
class Processor : public internal::ProcessorBase<typename AExecutor::Index> {
private:
    using Base = internal::ProcessorBase<typename AExecutor::Index>;
    using Index = typename Base::Index;

public:
    Processor(AExecutor executor = {});

    template<TupleModelizable... Ts>
    auto Process(ROOT::RDF::RNode rdf,
                 std::invocable<bool, std::shared_ptr<Tuple<Ts...>>> auto&& F) -> Index;

    template<TupleModelizable... Ts, std::integral AEventIDType>
    auto Process(ROOT::RDF::RNode rdf, muc::type_tag<AEventIDType>, std::string eventIDBranchName,
                 std::invocable<bool, muc::shared_ptrvec<Tuple<Ts...>>> auto&& F) -> Index;
    template<TupleModelizable... Ts, std::integral AEventIDType>
    auto Process(ROOT::RDF::RNode rdf, muc::type_tag<AEventIDType>, std::vector<gsl::index> eventSplit,
                 std::invocable<bool, muc::shared_ptrvec<Tuple<Ts...>>> auto&& F) -> Index;

    auto Executor() const -> const auto& { return fExecutor; }
    auto Executor() -> auto& { return fExecutor; }

private:
    template<typename AData>
    auto ProcessImpl(AsyncReader<AData>& asyncReader, Index n, std::string_view what,
                     std::invocable<bool, typename AData::value_type> auto&& F) -> Index;

    static auto ByPassOccurrenceCheck(Index n, std::string_view what) -> bool;

private:
    AExecutor fExecutor;
};

} // namespace Mustard::Data

#include "Mustard/Data/Processor.inl"
