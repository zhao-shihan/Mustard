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

#include "Mustard/Data/RDFEventSplitPoint.h++"
#include "Mustard/Data/TakeFrom.h++"
#include "Mustard/Data/internal/ProcessorBase.h++"
#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Extension/MPIX/Execution/Executor.h++"
#include "Mustard/Extension/ROOTX/RDataFrame.h++"

#include "muc/concepts"

#include <algorithm>
#include <functional>
#include <memory>
#include <numeric>
#include <ranges>
#include <string_view>
#include <utility>
#include <vector>

namespace Mustard::Data {

/// @brief A distributed data processor.
/// @tparam AExecutor Underlying MPI executor type.
template<muc::instantiated_from<MPIX::Executor> AExecutor = MPIX::Executor<unsigned>>
class Processor : public internal::ProcessorBase<typename AExecutor::Index> {
private:
    using Base = internal::ProcessorBase<typename AExecutor::Index>;
    using Index = typename Base::Index;

public:
    Processor(AExecutor executor = {}, Index batchSizeProposal = 5'000'000);

    template<TupleModelizable... Ts>
    auto Process(ROOTX::RDataFrame auto&& rdf,
                 std::invocable<bool, std::shared_ptr<Tuple<Ts...>>&> auto&& F) -> Index;

    template<TupleModelizable... Ts>
    auto Process(ROOTX::RDataFrame auto&& rdf, std::string_view eventIDBranchName,
                 std::invocable<bool, std::vector<std::shared_ptr<Tuple<Ts...>>>&> auto&& F) -> Index;
    template<TupleModelizable... Ts>
    auto Process(ROOTX::RDataFrame auto&& rdf, const std::vector<unsigned>& eventSplitPoint,
                 std::invocable<bool, std::vector<std::shared_ptr<Tuple<Ts...>>>&> auto&& F) -> Index;

    auto Executor() const -> const auto& { return fExecutor; }
    auto Executor() -> auto& { return fExecutor; }

private:
    static auto ByPassCheck(Index nBatch) -> bool;

private:
    AExecutor fExecutor;
};

} // namespace Mustard::Data

#include "Mustard/Data/Processor.inl"
