// -*- C++ -*-
//
// Copyright (C) 2020-2026  Shihan Zhao and contributors
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

#include "Mustard/Data/Container/ArcTupleVector.h++"
#include "Mustard/Data/Model.h++"
#include "Mustard/Data/Object/Tuple.h++"
#include "Mustard/Data/Processing/CountRDFEntry.h++"
#include "Mustard/Data/Processing/RDFEntryReader.h++"
#include "Mustard/Data/Processing/RDFEventReader.h++"
#include "Mustard/Data/Processing/RDFReader.h++"
#include "Mustard/Data/Processing/impl3/ProcessorBase.h++"
#include "Mustard/Execution/Executor.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Memory/Arc.h++"

#include "ROOT/RDataFrame.hxx"

#include "mplr/mplr.hpp"

#include "muc/tuple"
#include "muc/utility"

#include "gsl/gsl"

#include "fmt/format.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <functional>
#include <future>
#include <string_view>
#include <tuple>
#include <utility>

namespace Mustard::Data::inline Processing {

/// @brief A distributed data processor.
/// @tparam AExecutor Underlying MPI executor type.
template<muc::instantiated_from<Executor> AExecutor = Executor<gsl::index>>
class Processor : public impl3::ProcessorBase<typename AExecutor::Index> {
private:
    using Base = impl3::ProcessorBase<typename AExecutor::Index>;
    using typename Base::Index;

public:
    explicit Processor(AExecutor executor = {});

    template<Modelized M>
    auto Run(ROOT::RDF::RNode rdf,
             std::invocable<bool, ArcTuple<M>> auto&& f) -> Index;

    template<Modelized... Ms>
    auto Run(std::array<ROOT::RDF::RNode, sizeof...(Ms)> rdf,
             std::invocable<bool, ArcTuple<Ms>...> auto&& f) -> Index;

    template<Modelized M, std::integral T>
    auto Run(ROOT::RDF::RNode rdf, muc::type_tag<T>, std::string eventIDColumnName,
             std::invocable<bool, ArcTupleVector<M>> auto&& f) -> Index;
    template<Modelized M, std::integral T>
    auto Run(ROOT::RDF::RNode rdf, muc::type_tag<T>, Arc<SingleRDFEventInfo<T>> rdfEventInfo,
             std::invocable<bool, ArcTupleVector<M>> auto&& f) -> Index;

    template<Modelized... Ms, std::integral T>
    auto Run(std::array<ROOT::RDF::RNode, sizeof...(Ms)> rdf, muc::type_tag<T>, const std::string& eventIDColumnName,
             std::invocable<bool, ArcTupleVector<Ms>...> auto&& f) -> Index;
    template<Modelized... Ms, std::integral T>
    auto Run(std::array<ROOT::RDF::RNode, sizeof...(Ms)> rdf, muc::type_tag<T>, std::array<std::string, sizeof...(Ms)> eventIDColumnName,
             std::invocable<bool, ArcTupleVector<Ms>...> auto&& f) -> Index;
    template<Modelized... Ms, std::integral T>
    auto Run(std::array<ROOT::RDF::RNode, sizeof...(Ms)> rdf, muc::type_tag<T>, Arc<MultiRDFEventInfo<T, sizeof...(Ms)>> rdfEventInfo,
             std::invocable<bool, ArcTupleVector<Ms>...> auto&& f) -> Index;

    auto Executor() const -> const auto& { return fExecutor; }
    auto Executor() -> auto& { return fExecutor; }

private:
    template<std::signed_integral T, typename D, std::size_t N>
    auto RunImpl(RDFReader<T, D, N>& reader, Index n, std::string_view what, auto&& f) -> Index;

    static auto ByPassOccurrenceCheck(Index n, std::string_view what) -> bool;

private:
    AExecutor fExecutor;
};

} // namespace Mustard::Data::inline Processing

#include "Mustard/Data/Processing/Processor.inl"
