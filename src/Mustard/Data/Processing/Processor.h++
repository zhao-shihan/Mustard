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
#include "Mustard/Data/Processing/RDFEntryReader.h++"
#include "Mustard/Data/Processing/RDFEventReader.h++"
#include "Mustard/Data/Processing/RDFReader.h++"
#include "Mustard/Data/Processing/impl3/ProcessorBase.h++"
#include "Mustard/Execution/Executor.h++"
#include "Mustard/IO/PrettyLog.h++"

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
/// @details
/// Processor drives the event loop over one or more RDataFrame sources. It accepts a
/// pre-constructed reader (@ref RDFEntryReader or @ref RDFEventReader), partitions the
/// total work into batches, and dispatches them through the configured @ref Executor.
///
/// Each batch is processed asynchronously: the next batch is read in the background
/// while the current batch is handed to the user-provided callable. This pipelining
/// hides I/O latency behind computation.
///
/// The user callable @c f is invoked as @c f(bool bypass, Data&&). When @c bypass is
/// @c true (MPI processes outnumber work items), the callable receives a
/// default-constructed @c Data so users can still perform collective MPI communication
/// to avoid deadlocks.
///
/// @tparam AExecutor MPI executor type, must be an instantiation of @ref Executor.
template<muc::instantiated_from<Executor> AExecutor = Executor<gsl::index>>
class Processor : public impl3::ProcessorBase<typename AExecutor::Index> {
private:
    using Base = impl3::ProcessorBase<typename AExecutor::Index>;
    using typename Base::Index;

public:
    /// @brief Construct a processor with the given executor.
    /// @param executor MPI executor for work dispatch. Default-constructed if omitted.
    explicit Processor(AExecutor executor = {});

    /// @brief Process entry-level data from a single RDF.
    /// @tparam M Data model.
    /// @param reader Pre-constructed entry reader.
    /// @param f Callable invoked as @c f(bool bypass, ArcTuple\<M\>).
    /// @return Total number of entries processed.
    template<Modelized M>
    auto Run(RDFEntryReader<M>& reader,
             std::invocable<bool, ArcTuple<M>> auto&& f) -> Index;
    /// @brief Process entry-level data from multiple RDFs.
    /// @tparam Ms Data models, one per RDF.
    /// @param reader Pre-constructed multi-RDF entry reader.
    /// @param f Callable invoked as @c f(bool bypass, ArcTuple\<Ms\>...).
    /// @return Total number of entry groups processed.
    template<Modelized... Ms>
    auto Run(RDFEntryReader<Ms...>& reader,
             std::invocable<bool, ArcTuple<Ms>...> auto&& f) -> Index;
    /// @brief Process event-level data from a single RDF.
    /// @tparam M Data model.
    /// @tparam T Integral type of the event ID column.
    /// @param reader Pre-constructed event reader.
    /// @param f Callable invoked as @c f(bool bypass, ArcTupleVector\<M\>).
    /// @return Total number of events processed.
    template<Modelized M, std::integral T>
    auto Run(RDFEventReader<T, M>& reader,
             std::invocable<bool, ArcTupleVector<M>> auto&& f) -> Index;
    /// @brief Process event-level data from multiple RDFs with aligned event IDs.
    /// @tparam Ms Data models, one per RDF.
    /// @tparam T Integral type of the event ID columns.
    /// @param reader Pre-constructed multi-RDF event reader.
    /// @param f Callable invoked as @c f(bool bypass, ArcTupleVector\<Ms\>...).
    /// @return Total number of global events processed.
    template<Modelized... Ms, std::integral T>
    auto Run(RDFEventReader<T, Ms...>& reader,
             std::invocable<bool, ArcTupleVector<Ms>...> auto&& f) -> Index;

    /// @brief Immutable access to the underlying executor.
    auto Executor() const -> const auto& { return fExecutor; }
    /// @brief Mutable access to the underlying executor.
    auto Executor() -> auto& { return fExecutor; }

private:
    /// @brief Core execution loop shared by all @ref Run overloads.
    /// @tparam T Signed integral index type.
    /// @tparam D Data container type produced by the reader.
    /// @tparam N Number of RDF sources.
    /// @param reader Reader providing @c AsyncRead.
    /// @param n Total number of items (entries or events).
    /// @param what Human-readable label for log messages.
    /// @param f User callable.
    /// @return Total number of items processed.
    template<std::signed_integral T, typename D, std::size_t N>
    auto RunImpl(RDFReader<T, D, N>& reader, Index n, std::string_view what, auto&& f) -> Index;

    /// @brief Check whether bypass is needed (MPI processes > work items).
    /// @param n Number of work items.
    /// @param what Human-readable label for the warning message.
    /// @return @c true if some processes will not receive real work.
    static auto ByPassOccurrenceCheck(Index n, std::string_view what) -> bool;

private:
    AExecutor fExecutor; ///< Underlying MPI executor for work dispatch.
};

} // namespace Mustard::Data::inline Processing

#include "Mustard/Data/Processing/Processor.inl"
