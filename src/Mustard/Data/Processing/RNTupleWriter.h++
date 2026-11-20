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

#include "Mustard/Data/Processing/impl3/WriterBase.h++"
#include "Mustard/IO/PrettyLog.h++"

#include "ROOT/RNTupleModel.hxx"
#include "ROOT/RNTupleWriter.hxx"

#include <cstddef>
#include <memory>
#include <string>
#include <tuple>

namespace Mustard::Data::inline Processing {

/// @brief Writes data into a ROOT @c RNTuple.
///
/// @tparam M Mustard data model type used by @c Tuple and @c ArcTuple.
///
/// Fields are created once during construction via @c ROOT::RNTupleModel::MakeField.
/// Each call to @ref Fill copies or moves values from the provided tuple,
/// converting each field from its in-memory type (@c Field::Type) to the
/// corresponding persistent storage type (@c Field::PersistentType) via
/// @c Field::As, before committing one
/// entry through @c ROOT::RNTupleWriter::Fill().
///
/// @par Typical usage
/// @code{.cpp}
/// Mustard::Data::RNTupleWriter<MyModel> writer{"analysis/data"};
/// writer.Fill(oneEntry);
/// writer.Flush(); // optional: flush current cluster without final commit
/// writer.Fill(batchEntries);
/// @endcode
///
/// @note Dataset finalization is handled by ROOT writer destruction
/// (i.e. the underlying @c ROOT::RNTupleWriter commits on destruction).
template<Modelized M>
class RNTupleWriter : public impl3::WriterBase<M, RNTupleWriter<M>> {
public:
    /// @brief Construct a writer and create the target @c RNTuple with fields.
    ///
    /// If @p name contains '/', the prefix is treated as a ROOT directory path
    /// (created on demand) and the suffix as ntuple name.
    ///
    /// Internally this wrapper creates a @c ROOT::RNTupleModel, registers each
    /// model field, and calls @c ROOT::RNTupleWriter::Append against the
    /// resolved ROOT directory.
    ///
    /// @param name Ntuple name, or directory path plus ntuple name.
    ///
    /// @note A warning is printed if the current @c gDirectory is not writable.
    /// @note An exception is thrown if any field type is not supported by RNTuple.
    explicit RNTupleWriter(const std::string& name);

    /// @brief Number of entries already filled into the underlying ntuple.
    /// @return Current ntuple entry count.
    auto NEntry() const -> long long;

    /// @brief Flush filled entries to storage without committing the dataset.
    ///
    /// This maps to @c ROOT::RNTupleWriter::FlushCluster().
    /// You can continue calling @ref Fill after flushing.
    ///
    /// @note Unlike dataset commit, flushing does not close the writer nor
    /// expire the model.
    /// @warning Frequent flushing can reduce I/O throughput.
    auto Flush() -> void { fWriter->FlushCluster(); }

private:
    friend class impl3::WriterBase<M, RNTupleWriter<M>>;

    /// @brief Return a reference to the persistent storage for field index @p I.
    template<gsl::index I>
    auto EntryRef() -> auto& { return *std::get<I>(fEntry); }

    /// @brief Commit the current entry via @c ROOT::RNTupleWriter::Fill().
    auto DoFill() -> void { fWriter->Fill(); }

private:
    /// @brief Maps a @c std::tuple of field types to a @c std::tuple of
    /// @c shared_ptr<PersistentType> for each field.
    template<typename AStdTuple>
    struct SharedPtrTuple;
    template<typename... AValues>
    struct SharedPtrTuple<std::tuple<AValues...>> {
        using Type = std::tuple<std::shared_ptr<typename AValues::PersistentType>...>;
    };

private:
    typename SharedPtrTuple<typename M::StdTuple>::Type fEntry; ///< Tuple of @c shared_ptr to persistent storage, one per model field.
    std::unique_ptr<ROOT::RNTupleWriter> fWriter;               ///< Underlying ROOT RNTuple writer.
};

} // namespace Mustard::Data::inline Processing

#include "Mustard/Data/Processing/RNTupleWriter.inl"
