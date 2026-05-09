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

#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/IO/Print.h++"
#include "Mustard/Memory/Arc.h++"
#include "Mustard/Utility/MoveOnlyBase.h++"

#include "ROOT/RDataFrame.hxx"
#include "RtypesCore.h"
#include "TROOT.h"

#include "gtl/phmap.hpp"
#include "gtl/vector.hpp"

#include "mplr/mplr.hpp"

#include "mpi.h"

#include "muc/algorithm"
#include "muc/chrono"
#include "muc/numeric"

#include "gsl/gsl"

#include "fmt/format.h"

#include <algorithm>
#include <array>
#include <concepts>
#include <cstring>
#include <future>
#include <memory>
#include <optional>
#include <queue>
#include <span>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace Mustard::Data::inline Processing {

/// @brief Per-RDF metadata for contiguous event blocks from one event-ID column.
/// @tparam T Integral event-ID type.
/// @tparam U Event-block index type.
/// @details
/// Scanning one RDF produces two aligned arrays:
/// @li @ref fEventID: event IDs in first-appearance order of contiguous blocks.
/// @li @ref fEntry: block start entry indices plus one sentinel.
///
/// Terminology:
/// @li Event ID means the physical event number carried by data ("event ID" in HEP).
/// @li Event-block index means a local contiguous-block ordinal in this metadata object.
/// @li Event-block index is an internal sequence number and must not be interpreted as event ID.
///
/// For @f$N@f$ detected contiguous event blocks:
/// @li @f$\mathrm{fEventID}[i]@f$ is the event ID of event-block @f$i@f$.
/// @li @f$\mathrm{fEntry}[i]@f$ is the @c rdfentry_ of the first entry in block @f$i@f$.
/// @li @f$\mathrm{fEntry}[N]@f$ is a sentinel equal to total RDF entry count.
///
/// The scan enforces that each event ID appears in exactly one contiguous block.
///
/// Execution model:
/// @li If MPI is unavailable, or MPI world size is @c 1, metadata are built locally.
/// @li In MPI multi-process environment, one node leader scans once and broadcasts
///     metadata to other node leaders.
/// @li Each node stores @ref fEventID and @ref fEntry in one shared-memory;
///     local ranks attach spans to that memory.
///
/// Empty-input behavior:
/// @li @ref NEvent() returns @c 0.
/// @li @ref NEntry() returns @c 0.
/// @li @ref fEntry still contains one sentinel element.
template<std::integral T, std::signed_integral U = std::make_signed_t<T>>
class SingleRDFEventInfo : public MoveOnlyBase {
public:
    /// @brief Event ID integral type.
    using EventIDType = T;
    /// @brief RDF entry integral type.
    using EntryType = gsl::index;
    /// @brief Event-block index integral type.
    using IndexType = U;

public:
    /// @brief Construct per-RDF event-block metadata with explicit node communicators.
    /// @param rdf Input RDF node to scan.
    /// @param eventIDColumnName Name of the event-ID column.
    /// @param rootNodeIdx Node leader index used as broadcast root when MPI is enabled.
    /// @param intraNodeComm Intra-node communicator.
    /// @param interNodeComm Inter-node communicator.
    /// @pre In @p rdf, entries with the same event ID are contiguous.
    /// @pre ROOT IMT (Implicit Multi-Threading) is disabled.
    /// @pre If MPI path is used, @p rootNodeIdx must be in @c [0,ClusterSize()).
    /// @note If an event ID reappears after another ID has appeared, an exception is thrown.
    /// @warning In MPI multi-process environment, this constructor performs collective
    /// communications; all processes in the world communicator must call it with
    /// compatible arguments.
    explicit SingleRDFEventInfo(ROOT::RDF::RNode rdf, std::string eventIDColumnName, int rootNodeIdx = 0,
                                std::optional<std::pair<mplr::communicator, mplr::communicator>> intraInterNodeComm = {});

    /// @brief Destroy event metadata storage.
    /// @details
    /// Releases the MPI shared-memory window when MPI-backed storage is active;
    /// otherwise destroys locally owned vectors.
    ~SingleRDFEventInfo();

    /// @brief Number of contiguous event blocks (event-index domain size).
    auto NEvent() const -> auto { return gsl::narrow_cast<IndexType>(fEventID.size()); }
    /// @brief Total RDF entry count represented by the sentinel in @ref fEntry.
    auto NEntry() const -> auto { return fEntry.back(); }
    /// @brief Event ID of event block @p i.
    /// @note @p i is an event-block index, not an event ID.
    /// @pre @p i is in @c [0,NEvent()).
    auto EventID(IndexType i) const -> auto { return fEventID[i]; }
    /// @brief Start entry index of event block @p i, or
    /// total entry count if @p i is the sentinel index.
    /// @note @p i is an event-block index, not an event ID.
    /// @pre @p i is in @c [0,NEvent()+1).
    auto Entry(IndexType i) const -> auto { return fEntry[i]; }
    /// @brief Entry count of event block @p i.
    /// @note @p i is an event-block index, not an event ID.
    /// @pre @p i is in @c [0,NEvent()).
    auto Size(IndexType i) const -> auto { return Entry(i + 1) - Entry(i); }

private:
    struct Data {
        gtl::vector<T> eventID;       ///< Owning storage of event-block IDs.
        gtl::vector<EntryType> entry; ///< Owning storage of block boundaries, including sentinel.
    };

private:
    std::span<const T> fEventID;                                                          ///< Non-owning view of event IDs (into shared-memory or local data).
    std::span<const EntryType> fEntry;                                                    ///< Non-owning view of entry boundaries (into shared-memory or local data).
    std::variant<MPI_Win, Data> fShmWinOrData;                                            ///< Storage backend: shared-memory handle or local data.
    std::optional<std::pair<mplr::communicator, mplr::communicator>> fIntraInterNodeComm; ///< Optional communicators.
};

/// @brief Multi-RDF event alignment table and index mappings.
/// @tparam T Integral event-ID type.
/// @tparam N Number of RDF sources.
/// @tparam U Integral index type.
/// @details
/// Each global index corresponds to one distinct event ID in the union of
/// all RDF-local event IDs.
///
/// Terminology:
/// @li Event ID means the physical event number carried by data ("event ID" in HEP).
/// @li Global/local event-block index means alignment/lookup ordinal used by this class.
/// @li Event-block index is an internal sequence number and must not be interpreted as event ID.
///
/// Core table:
/// @li @f$\mathrm{fToLocalEvtIdx}[g][k]@f$ is the local event-block index in RDF @f$k@f$
///     for global index @f$g@f$, or @c -1 if RDF @f$k@f$ does not contain that event ID.
///
/// Derived reverse mapping:
/// @li @f$\mathrm{fToGlobEvtIdx}[k][l]@f$ is the global index that contains
///     local event-block index @f$l@f$ of RDF @f$k@f$.
///
/// Derived suffix-min table:
/// @li @f$\mathrm{fMinLocalEvtIdxAfter}[g][k]@f$ is the minimum local
///     event-block index of RDF @f$k@f$ among global indices not earlier than
///     @f$g@f$ (inclusive suffix minimum).
/// @li If no event exists in RDF @f$k@f$ at or after @f$g@f$, the value is
///     the number of event blocks in RDF @f$k@f$ (sentinel).
///
/// Per-RDF event-block metadata are stored in @ref fPerRDFEventInfo and addressed
/// by local event-block indices.
///
/// Global-index ordering policy:
/// @li RDF sources are ranked by descending total entry count (@c NEntry).
/// @li Each global event is sorted by its local event-block index in the
///     highest-ranked RDF that contains it; events absent from the
///     top-ranked RDF are ordered by their index in the next-ranked
///     RDF that contains them, and so on.
/// @li The order is not guaranteed to be event-ID ascending.
/// @li Events with the same ordering key may keep container-dependent
///     relative order.
///
/// Alignment semantics:
/// @li One global index is produced for each distinct event ID in the union of
///     all RDF-local event IDs.
/// @li Missing events are represented by @c -1 in @ref fToLocalEvtIdx.
///
/// Execution model:
/// @li If MPI is unavailable, or MPI world size is @c 1, all per-RDF metadata and
///     alignment tables are built locally.
/// @li In MPI environment, RDF @f$i@f$ is scanned by node-@f$i\bmod P@f$ leader,
///     where @f$P@f$ is cluster size.
/// @li After all per-RDF metadata are available, each node leader builds the same
///     alignment table locally and shares it with local processes via shared memory.
template<std::integral T, std::size_t N, std::signed_integral U = std::make_signed_t<T>>
    requires(N >= 2)
class MultiRDFEventInfo : public MoveOnlyBase {
public:
    /// @brief Event ID integral type.
    using EventIDType = T;
    /// @brief RDF entry integral type.
    using EntryType = gsl::index;
    /// @brief Event-block index integral type.
    using IndexType = U;

public:
    /// @brief Construct a multi-RDF alignment using one shared event-ID column name.
    /// @param rdf Input RDF nodes.
    /// @param eventIDColumnName Shared event-ID column name used by all RDFs.
    /// @pre For every RDF, entries with the same event ID are contiguous.
    /// @pre ROOT IMT (Implicit Multi-Threading) is disabled.
    /// @note This overload forwards to the per-RDF-name overload after expanding
    /// @p eventIDColumnName to all RDFs.
    /// @warning In MPI multi-process environment, this constructor performs collective
    /// communications; all processes in the world communicator must call it with
    /// compatible arguments.
    explicit MultiRDFEventInfo(std::array<ROOT::RDF::RNode, N> rdf, const std::string& eventIDColumnName);
    /// @brief Construct a multi-RDF alignment using per-RDF event-ID column names.
    /// @param rdf Input RDF nodes.
    /// @param eventIDColumnName Event-ID column name for each RDF source.
    /// @pre For every RDF, entries with the same event ID are contiguous.
    /// @pre ROOT IMT (Implicit Multi-Threading) is disabled.
    /// @note If an event ID reappears after another ID has appeared inside any RDF,
    /// an exception is thrown.
    /// @warning In MPI multi-process environment, this constructor performs collective
    /// communications; all processes in the world communicator must call it with
    /// compatible arguments.
    explicit MultiRDFEventInfo(std::array<ROOT::RDF::RNode, N> rdf, std::array<std::string, N> eventIDColumnName);

    /// @brief Destroy alignment-table storage.
    /// @details
    /// Releases the MPI shared-memory window when MPI-backed storage is active;
    /// otherwise destroys locally owned alignment vectors.
    ~MultiRDFEventInfo();

    /// @brief Number of global aligned events (global event-index domain size).
    auto NEvent() const -> auto { return gsl::narrow_cast<IndexType>(fToLocalEvtIdx.size()); }

    /// @brief Whether global event @p globIdx exists in RDF @p kRDF.
    /// @note Both indices are event-block indices, not event IDs.
    /// @pre @p globIdx is in @c [0,NEvent()) and @p kRDF is in @c [0,N).
    auto HasEvent(IndexType globIdx, gsl::index kRDF) const -> auto { return ToLocalEvtIdx(globIdx, kRDF) != -1; }
    /// @brief Map global aligned event-block index to local event-block index in one RDF.
    /// @note Both indices are event-block indices, not event IDs.
    /// @pre @p globIdx is in @c [0,NEvent()) and @p kRDF is in @c [0,N).
    /// @return Local event-block index, or @c -1 when absent.
    auto ToLocalEvtIdx(IndexType globIdx, gsl::index kRDF) const -> auto { return fToLocalEvtIdx[globIdx][kRDF]; }
    /// @brief Map local event-block index in one RDF to global aligned event-block index.
    /// @note Both indices are event-block indices, not event IDs.
    /// @pre @p kRDF is in @c [0,N) and @p localIdx is in
    /// @c [0,RDF(kRDF).NEvent()).
    auto ToGlobEvtIdx(IndexType localIdx, gsl::index kRDF) const -> auto { return fToGlobEvtIdx[kRDF][localIdx]; }
    /// @brief Query minimum local event-block index at or after a global index.
    /// @note Both indices are event-block indices, not event IDs.
    /// @param globIdx Global index @f$g@f$.
    /// @param kRDF RDF index @f$k@f$.
    /// @pre @p globIdx is in @c [0,NEvent()) and @p kRDF is in @c [0,N).
    /// @return @f$\min\{\mathrm{fToLocalEvtIdx}[h][kRDF] \mid h\ge globIdx,\ \mathrm{fToLocalEvtIdx}[h][kRDF]\neq-1\}@f$;
    /// returns @c RDF(kRDF).NEvent() when the set is empty.
    auto MinLocalEvtIdxAfter(IndexType globIdx, gsl::index kRDF) const -> auto { return fMinLocalEvtIdxAfter[globIdx][kRDF]; }

    /// @brief Access per-RDF event-block metadata.
    /// @pre @p kRDF is in @c [0,N).
    auto RDF(gsl::index kRDF) const -> const auto& { return *fPerRDFEventInfo[kRDF]; }
    /// @brief Shared pointer to per-RDF event-block metadata.
    /// @pre @p kRDF is in @c [0,N).
    auto PerRDFEventInfo(gsl::index kRDF) const -> auto { return fPerRDFEventInfo.at(kRDF); }

private:
    struct Data {
        gtl::vector<std::array<U, N>> toLocalEvtIdx;       ///< Owning storage of global->local index table.
        std::array<gtl::vector<U>, N> toGlobEvtIdx;        ///< Owning storage of per-RDF local->global index tables.
        gtl::vector<std::array<U, N>> minLocalEvtIdxAfter; ///< Owning storage of per-RDF suffix-min local index table.
    };

private:
    auto BuildData() const -> Data;

private:
    std::span<const std::array<U, N>> fToLocalEvtIdx;              ///< Non-owning view of global->local index table (into shared-memory or local data).
    std::array<std::span<const U>, N> fToGlobEvtIdx;               ///< Non-owning views of local->global index tables (into shared-memory or local data).
    std::span<const std::array<U, N>> fMinLocalEvtIdxAfter;        ///< Non-owning view of per-RDF suffix-min local index table (into shared-memory or local data).
    std::array<Arc<SingleRDFEventInfo<T, U>>, N> fPerRDFEventInfo; ///< Shared ownership of per-RDF event-block metadata.
    std::variant<MPI_Win, Data> fShmWinOrData;                     ///< Storage backend: shared-memory handle or local data.
};

} // namespace Mustard::Data::inline Processing

#include "Mustard/Data/Processing/RDFEventInfo.inl"
