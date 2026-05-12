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
#include "Mustard/Data/Processing/RDFEventInfo.h++"
#include "Mustard/Data/Processing/RDFReader.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Memory/Arc.h++"
#include "Mustard/gslx/index_sequence.h++"

#include "RtypesCore.h"

#include "gsl/gsl"

#include "fmt/format.h"

#include <algorithm>
#include <array>
#include <concepts>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace Mustard::Data::inline Processing {

template<std::integral T, Modelized... Ms>
class RDFEventReader;

/// @brief Single RDataFrame event reader for a single data model.
/// @details
/// This specialization reads entries from a ROOT RDataFrame, groups them by an event
/// identifier column (typically an event ID), and packages grouped entries into vectors
/// of tuples. Each event block is represented as an ArcTupleVector<M> containing all
/// entries belonging to that event.
///
/// Event boundaries are determined by transitions in the event ID column. The reader
/// internally maintains an RDFEventInfo structure that maps event indices to entry
/// ranges, enabling efficient event-based iteration and boundary detection.
///
/// @tparam T Integral type of the event ID column (e.g., uint32_t, int64_t).
/// @tparam M A Modelized type defining the structure and columns to be read from RDF.
///
/// @note The event ID column name must be provided at construction.
template<std::integral T, Modelized M>
class RDFEventReader<T, M> : public RDFReader<std::make_signed_t<T>, ArcTupleVector<M>, 1> {
    template<std::integral, Modelized...>
    friend class RDFEventReader; // for accessing UncheckedAsyncRead

private:
    using Base = RDFReader<std::make_signed_t<T>, ArcTupleVector<M>, 1>;

public:
    using typename Base::Entry;
    using typename Base::Index;

protected:
    using typename Base::DataFrameType;

public:
    /// @brief Construct a single RDF event reader, identifying events by the given column.
    /// @param rdf DataFrame node containing all model columns required by model @p M.
    /// @param eventIDColumnName Name of the column containing event identifiers for grouping.
    explicit RDFEventReader(DataFrameType rdf, std::string eventIDColumnName);
    /// @brief Construct a single RDF event reader reusing precomputed event boundary metadata.
    /// @param rdf DataFrame node containing all model columns required by model @p M.
    /// @param rdfEventInfo Precomputed event-boundary metadata (RDFEventInfo) that can be
    ///                     shared across multiple readers for efficiency.
    explicit RDFEventReader(DataFrameType rdf, Arc<SingleRDFEventInfo<T>> rdfEventInfo);
    ~RDFEventReader() override { this->DestructorAction(); }

    /// @brief Total aggregated number of event blocks in this reader.
    /// @details This returns the maximum event count that can be read by this reader,
    ///          determined by the event ID grouping in the underlying RDF.
    auto NEvent() const -> auto { return this->Size(); }
    /// @brief Retrieve the index of the next global event block to fetch.
    /// @details The returned index represents the next unread event block in the sequence.
    /// @return Next unread event-block index, or `-1` when queried during an active read operation.
    auto NextEvtIdx() const -> Index;
    /// @brief Current underlying entry index of the next grouping limit.
    /// @return Next unread entry index in the physical tree, or `-1` while reading.
    auto NextEntry() const -> Entry;

    /// @brief Access the event boundary metadata for this reader.
    /// @details Returns a shared Arc to the internal RDFEventInfo structure that maps
    ///          event indices to entry ranges in the RDF.
    /// @return Shared Arc to the event-boundary descriptor (RDFEventInfo) used by this reader.
    auto RDFEventInfo() const -> auto { return fRDFEventInfo; }

private:
    auto ReaderKernel() -> void override;

private:
    Entry fEntry;                             ///< Physical tree entry index currently tracked during event fence operations for boundary detection.
    Arc<SingleRDFEventInfo<T>> fRDFEventInfo; ///< Event-boundary mapping table for single-RDF event grouping and index conversion.
};

/// @brief Multi-RDataFrame event reader for multiple aligned data models.
/// @details
/// This specialization reads entries from multiple independent ROOT RDataFrames,
/// groups them by event ID columns, and maintains alignment across all sources.
/// It handles cases where RDFs may have different event structures or misaligned
/// boundaries by buffering out-of-window events and providing configurable
/// warning/fatal thresholds for excessive buffering.
///
/// Each read operation produces a vector of per-event tuples, where each tuple
/// element is an `ArcTupleVector<Mi>` containing the entries from the corresponding
/// RDF source for that global event block.
///
/// Event boundaries can be specified globally (same column name for all RDFs) or
/// per-RDF with custom column names. The reader coordinates event indices across
/// all sources via a MultiRDFEventInfo structure.
///
/// Per-RDF sub-readers (RDFEventReader<T, Mi>) are maintained and coordinated by
/// this master reader. The reset operation propagates to all sub-readers.
///
/// @tparam T Integral type of event ID columns across all RDFs.
/// @tparam Ms Pack of Modelized types, each defining structure and columns for one RDF source.
///
/// @note The event ID column name(s) must be provided at construction.
/// @note Misaligned events (entries with same event ID across misaligned RDFs) are
///       buffered with configurable thresholds to detect anomalies.
template<std::integral T, Modelized... Ms>
class RDFEventReader : public RDFReader<std::make_signed_t<T>, std::tuple<ArcTupleVector<Ms>...>, sizeof...(Ms)> {
private:
    using N = std::integral_constant<gsl::index, sizeof...(Ms)>;
    using Base = RDFReader<std::make_signed_t<T>, std::tuple<ArcTupleVector<Ms>...>, N{}>;

public:
    using typename Base::Data;
    using typename Base::Index;

protected:
    using typename Base::DataFrameType;

public:
    /// @brief Construct a multi-RDF event reader with a uniform event ID column name.
    /// @param rdf Array of RDataFrame nodes aligned by tuple index with models in @p Ms...
    /// @param eventIDColumnName Name of the event ID column used by all RDFs for grouping.
    ///                          Each RDF must contain this column with compatible event IDs.
    explicit RDFEventReader(DataFrameType rdf, const std::string& eventIDColumnName);
    /// @brief Construct a multi-RDF event reader with per-RDF event ID column names.
    /// @param rdf Array of RDataFrame nodes aligned by tuple index with models in @p Ms...
    /// @param eventIDColumnName Array of event ID column names, one per RDF, specifying which
    ///                          column in each RDF contains the event identifiers for grouping.
    explicit RDFEventReader(DataFrameType rdf, std::array<std::string, N{}> eventIDColumnName);
    /// @brief Construct a multi-RDF event reader using precomputed global event metadata.
    /// @param rdf Array of RDataFrame nodes aligned by tuple index with models in @p Ms...
    /// @param rdfEventInfo Precomputed multi-RDF event boundary metadata (MultiRDFEventInfo)
    ///                     that coordinates event indices across all sources.
    explicit RDFEventReader(DataFrameType rdf, Arc<MultiRDFEventInfo<T, N{}>> rdfEventInfo);
    ~RDFEventReader() override { this->DestructorAction(); }

    /// @brief Reset the multi-reader to its initial state.
    /// @details Resets this coordinator reader and all per-RDF event sub-readers back to the
    ///          beginning, allowing the entire event read process to restart from the first event.
    auto Reset() -> void override;

    /// @brief Total count of global event blocks coordinated across all RDF sources.
    /// @details Returns the maximum number of global event blocks that can be read by this reader,
    ///          determined by coordinating event boundaries across all source RDFs.
    auto NEvent() const -> auto { return this->Size(); }
    /// @brief Retrieve the index of the next global event block to fetch.
    /// @details The returned index represents the next unread event block in the global sequence
    ///          coordinated across all source RDFs.
    /// @return Next unread global event-block index, or `-1` when queried during an active read operation.
    auto NextEvtIdx() const -> Index;

    /// @brief Access the global event boundary metadata coordinating all sources.
    /// @details Returns a shared Arc to the internal MultiRDFEventInfo structure that
    ///          coordinates event indices and boundaries across all RDF sources.
    /// @return Shared Arc to the multi-RDF event alignment descriptor (MultiRDFEventInfo) used by this reader.
    auto RDFEventInfo() const -> auto { return fRDFEventInfo; }

    /// @brief Access a per-RDF event sub-reader for detailed event handling.
    /// @details Provides access to the internal single-RDF event reader responsible for
    ///          managing event grouping and reading for a specific RDF source.
    /// @tparam K Zero-based index of the RDF source (must be < sizeof...(Ms))
    /// @return Const reference to the `K`-th initialized single-RDF event sub-reader.
    /// @pre Reader has been constructed successfully and `K < sizeof...(Ms)`.
    template<gsl::index K>
        requires(K < N{})
    auto SubReader() const -> const auto& { return get<K>(fSubReader).value(); }

    /// @brief Query the warning threshold for misaligned event buffering.
    /// @details The warning threshold triggers a one-time warning message when the number of
    ///          out-of-window buffered events reaches or exceeds this limit for any RDF source.
    /// @return Current warning threshold for buffered misaligned events per RDF.
    /// @note Default value is 1000.
    auto MisalignedEventCountWarningThreshold() const -> auto { return fMisalignedEventCountWarningThreshold; }
    /// @brief Set the warning threshold for buffered misaligned events.
    /// @param n New warning threshold value.
    /// @note Rejected while reading is in progress; the threshold remains unchanged if called during read.
    /// @note This threshold applies per RDF source independently.
    auto MisalignedEventCountWarningThreshold(Index n) -> void;

    /// @brief Query the fatal threshold for misaligned event buffering.
    /// @details The fatal threshold triggers an exception when the number of out-of-window
    ///          buffered events reaches or exceeds this limit for any RDF source, terminating the read operation.
    /// @return Current fatal threshold for buffered misaligned events per RDF.
    /// @note Default value is 10000.
    auto MisalignedEventCountFatalThreshold() const -> auto { return fMisalignedEventCountFatalThreshold; }
    /// @brief Set the fatal threshold for buffered misaligned events.
    /// @param n New fatal threshold value.
    /// @note Rejected while reading is in progress; the threshold remains unchanged if called during read.
    /// @note This threshold applies per RDF source independently.
    auto MisalignedEventCountFatalThreshold(Index n) -> void;

private:
    auto ReaderKernel() -> void override;

private:
    Arc<MultiRDFEventInfo<T, N{}>> fRDFEventInfo;                   ///< Global/local event-index mapping coordinating all RDF sources.
    std::tuple<std::optional<RDFEventReader<T, Ms>>...> fSubReader; ///< One single-RDF event reader per source, managing per-source event grouping.
    std::array<bool, N{}> fMisalignedEventCountWarningEmitted;      ///< Per-source one-shot warning emission flags to prevent repeated warnings.
    Index fMisalignedEventCountWarningThreshold;                    ///< Threshold for warning on buffered out-of-window events (default: 1000).
    Index fMisalignedEventCountFatalThreshold;                      ///< Threshold for fatal exception on buffered out-of-window events (default: 10000).
};

} // namespace Mustard::Data::inline Processing

#include "Mustard/Data/Processing/RDFEventReader.inl"
