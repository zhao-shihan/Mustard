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

#include "Mustard/Data/Model.h++"
#include "Mustard/Data/Object/Tuple.h++"
#include "Mustard/Data/Processing/CountRDFEntry.h++"
#include "Mustard/Data/Processing/RDFReader.h++"
#include "Mustard/gslx/index_sequence.h++"

#include "RtypesCore.h"

#include "gsl/gsl"

#include <algorithm>
#include <array>
#include <concepts>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace Mustard::Data::inline Processing {

template<Modelized... Ms>
class RDFEntryReader;

/// @brief Single RDataFrame entry reader for a single data model.
/// @details
/// This specialization reads individual entries from a ROOT RDataFrame and packs them
/// into tuples conforming to the model @p M. Each entry from the RDF is converted to
/// an ArcTuple<M> containing the model's fields.
///
/// @tparam M A Modelized type defining the structure and columns to be read from RDF.
///
/// @note The total entry count is auto-detected from the RDataFrame, or can be
///       explicitly provided via the second constructor.
template<Modelized M>
class RDFEntryReader<M> : public RDFReader<gsl::index, ArcTuple<M>, 1> {
    template<Modelized...>
    friend class RDFEntryReader; // for accessing UncheckedAsyncRead

private:
    using Base = RDFReader<gsl::index, ArcTuple<M>, 1>;

public:
    using typename Base::Entry;

protected:
    using typename Base::DataFrameType;

public:
    /// @brief Construct a single RDF entry reader, auto-detecting the entry count.
    /// @param rdf DataFrame node containing all model columns required by model @p M.
    /// @details The total entry count is automatically detected from the RDataFrame structure.
    explicit RDFEntryReader(DataFrameType rdf);
    /// @brief Construct a single RDF entry reader with explicit entry count specification.
    /// @param rdf DataFrame node containing all model columns required by model @p M.
    /// @param nRDFEntry Maximum number of entries to be readable from this RDF.
    explicit RDFEntryReader(DataFrameType rdf, Entry nRDFEntry);
    ~RDFEntryReader() override { this->DestructorAction(); }

    /// @brief Number of entries in this reader.
    auto NEntry() const -> auto { return this->Size(); }
    /// @brief Next entry index to be produced by the iterator.
    /// @return Next unread entry index, or `-1` when queried during an active read.
    auto NextEntry() const -> Entry;

private:
    auto ReaderKernel() -> void override;
};

/// @brief Multi-RDataFrame entry reader for multiple aligned data models.
/// @details
/// This specialization reads entries from multiple independent ROOT RDataFrames in
/// a coordinated pass, packing corresponding entries from each RDF into tuples. It
/// coordinates reading across all RDF sources by limiting to the maximum entry count
/// across sources. Each read operation produces a vector of tuples, one tuple per
/// global entry index, with one `ArcTuple<Mi>` per model.
///
/// The reader maintains per-RDF sub-readers (RDFEntryReader<Mi>) which are coordinated
/// by this master reader. The reset operation propagates to all sub-readers to ensure
/// consistent state.
///
/// @tparam Ms Pack of Modelized types, each defining structure and columns for one RDF source.
///
/// @note Entry counts per RDF are auto-detected, with the effective range being
///       [0, max(per-RDF entry counts)).
template<Modelized... Ms>
class RDFEntryReader : public RDFReader<gsl::index, std::tuple<ArcTuple<Ms>...>, sizeof...(Ms)> {
private:
    using N = std::integral_constant<gsl::index, sizeof...(Ms)>;
    using Base = RDFReader<gsl::index, std::tuple<ArcTuple<Ms>...>, N{}>;

public:
    using typename Base::Entry;

protected:
    using typename Base::DataFrameType;

public:
    /// @brief Construct a multi-RDF entry reader, auto-detecting per-RDF entry counts.
    /// @param rdf Array of RDataFrame nodes aligned by tuple index with models in @p Ms...
    /// @details Per-RDF entry counts are detected internally. The effective readable range
    ///          is [0, max(per-RDF entry counts)).
    explicit RDFEntryReader(DataFrameType rdf);
    /// @brief Construct a multi-RDF entry reader with explicit per-RDF entry count bounds.
    /// @param rdf Array of RDataFrame nodes aligned by tuple index with models in @p Ms...
    /// @param nRDFEntry Array of maximum entry counts, one per RDF. The effective range is
    ///                  [0, max(nRDFEntry)).
    explicit RDFEntryReader(DataFrameType rdf, const std::array<Entry, N{}>& nRDFEntry);
    ~RDFEntryReader() override { this->DestructorAction(); }

    /// @brief Reset the multi-reader to its initial state.
    /// @details Resets this coordinator reader and all per-RDF sub-readers back to the beginning,
    ///          allowing the entire read process to restart from the first entry.
    auto Reset() -> void override;

    /// @brief Count of maximum accessible entries conforming to the alignment bounds.
    auto NEntry() const -> auto { return this->Size(); }
    /// @brief Next entry index to be jointly evaluated.
    /// @return Next unread global entry index, or `-1` when queried during an active read.
    auto NextEntry() const -> Entry;

    /// @brief Access the `K`-th RDF generic sub-reader used to retrieve models.
    /// @return Const reference to the `K`-th initialized single-RDF entry sub-reader.
    /// @pre Reader has been constructed successfully and `K < sizeof...(Ms)`.
    template<gsl::index K>
        requires(K < N{})
    auto SubReader() const -> const auto& { return get<K>(fSubReader).value(); }

private:
    auto ReaderKernel() -> void override;

private:
    std::tuple<std::optional<RDFEntryReader<Ms>>...> fSubReader; ///< One single-RDF entry reader per source.
};

} // namespace Mustard::Data::inline Processing

#include "Mustard/Data/Processing/RDFEntryReader.inl"
