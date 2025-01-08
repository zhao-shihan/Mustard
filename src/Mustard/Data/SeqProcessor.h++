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

#include "indicators/block_progress_bar.hpp"

#include "muc/concepts"
#include "muc/numeric"
#include "muc/ptrvec"
#include "muc/time"

#include "gsl/gsl"

#include "fmt/format.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <future>
#include <limits>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace Mustard::Data {

/// @brief A sequential data processor.
class SeqProcessor : public internal::ProcessorBase<unsigned> {
public:
    SeqProcessor();

    auto PrintProgress(bool val) -> void { fPrintProgress = val; }
    auto PrintProgress() const -> auto { return fPrintProgress; }

    template<TupleModelizable... Ts>
    auto Process(ROOT::RDF::RNode rdf,
                 std::invocable<std::shared_ptr<Tuple<Ts...>>> auto&& F) -> Index;

    template<TupleModelizable... Ts>
    auto Process(ROOT::RDF::RNode rdf, std::string eventIDBranchName,
                 std::invocable<muc::shared_ptrvec<Tuple<Ts...>>> auto&& F) -> Index;
    template<TupleModelizable... Ts>
    auto Process(ROOT::RDF::RNode rdf, const std::vector<Index>& eventSplit,
                 std::invocable<muc::shared_ptrvec<Tuple<Ts...>>> auto&& F) -> Index;

    template<muc::instantiated_from<TupleModel>... Ts>
    auto Process(std::array<ROOT::RDF::RNode, sizeof...(Ts)> rdf,
                 std::string eventIDBranchName,
                 std::invocable<muc::shared_ptrvec<Tuple<Ts>>...> auto&& F) -> Index;
    template<muc::instantiated_from<TupleModel>... Ts>
    auto Process(std::array<ROOT::RDF::RNode, sizeof...(Ts)> rdf,
                 std::vector<std::string> eventIDBranchName,
                 std::invocable<muc::shared_ptrvec<Tuple<Ts>>...> auto&& F) -> Index;
    template<muc::instantiated_from<TupleModel>... Ts>
    auto Process(std::array<ROOT::RDF::RNode, sizeof...(Ts)> rdf,
                 const std::vector<std::array<RDFEntryRange, sizeof...(Ts)>>& eventSplit,
                 std::invocable<muc::shared_ptrvec<Tuple<Ts>>...> auto&& F) -> Index;

private:
    auto LoopBeginAction(Index nTotal) -> void;
    auto IterationEndAction(Index nProcessed, Index nTotal) -> void;
    auto LoopEndAction(Index nTotal) -> void;

    auto ReportProgress(Index nProcessed, Index nTotal, double msElapsed) -> void;

private:
    bool fPrintProgress;

    std::optional<indicators::BlockProgressBar> fProgressBar;
    std::optional<muc::wall_time_stopwatch<>> fWallTimeStopWatch;
    double fLastReportTime;
    std::future<void> fAsyncReportProgress;
};

} // namespace Mustard::Data

#include "Mustard/Data/SeqProcessor.inl"
