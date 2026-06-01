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

#include "Mustard/Data/Object/Field.h++"
#include "Mustard/Data/Object/FieldTypeName.h++"
#include "Mustard/Data/Processing/impl3/WriterBase.h++"
#include "Mustard/IO/PrettyLog.h++"

#include "TTree.h"

#include "fmt/format.h"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>

namespace Mustard::Data::inline Processing {

/// @brief Writes data into a ROOT @c TTree.
///
/// @tparam M Mustard data model type used by @c Tuple and @c ArcTuple.
///
/// Tree branches are created once during construction by walking the fields in
/// @c M::StdTuple and binding them to an internal persistent-type entry buffer.
/// Each field's in-memory type (@c Field::Type) is converted to the
/// corresponding persistent storage type (@c Field::PersistentType) via
/// @c Field::As before filling.
///
/// @par Typical usage
/// @code{.cpp}
/// Mustard::Data::TTreeWriter<MyModel> writer{"analysis/data"};
/// writer.Fill(oneEntry);
/// writer.Flush(); // optional: flush current tree state
/// writer.Fill(batchEntries);
/// @endcode
///
/// @note Final persistence is handled by writer destruction
/// (i.e. the underlying @c TTree is written once on destruction).
template<Modelized M>
class TTreeWriter : public impl3::WriterBase<M, TTreeWriter<M>> {
public:
    /// @brief Construct a writer and create the target @c TTree with branches.
    ///
    /// If @p name contains '/', the prefix is treated as a ROOT directory path
    /// (created on demand) and the suffix as tree name.
    ///
    /// Internally this wrapper constructs @c TTree in the resolved ROOT directory,
    /// then creates one branch per model field and configures each branch title
    /// from the field type and field metadata. The tree is removed from its
    /// directory's internal list to prevent ROOT from auto-deleting it when the
    /// directory closes.
    ///
    /// @param name Tree name, or directory path plus tree name.
    ///
    /// @note A warning is printed if the current @c gDirectory is not writable.
    /// @note A @c std::runtime_error is thrown if any branch creation fails
    /// (e.g. due to unsupported field types).
    explicit TTreeWriter(const std::string& name);

    /// @brief Write tree content to the ROOT file on destruction.
    ///
    /// Saves the current @c gDirectory, switches to the tree's owning directory,
    /// calls @c fTree->Write(), and restores the original @c gDirectory via
    /// @c gsl::finally.
    ~TTreeWriter();

    /// @brief Number of entries already filled into the underlying tree.
    /// @return Current tree entry count.
    auto NEntry() const -> long long;

    /// @brief Flush current tree state to storage without finalizing the writer.
    ///
    /// This maps to @c TTree::AutoSave("SaveSelf").
    /// You can continue calling @ref Fill after flushing.
    ///
    /// @note Flushing does not replace the final @c Write performed on destruction.
    /// @warning Frequent flushing can reduce I/O throughput and increase output file size.
    auto Flush() -> void { fTree->AutoSave("SaveSelf"); }

private:
    friend class impl3::WriterBase<M, TTreeWriter<M>>;

    /// @brief Return a reference to the persistent storage for field index @p I.
    template<gsl::index I>
    auto EntryRef() -> auto& { return std::get<I>(*fEntry); }

    /// @brief Commit the current entry via @c TTree::Fill().
    auto DoFill() -> void { fTree->Fill(); }

private:
    /// @brief Maps a @c std::tuple of field types to a @c std::tuple of
    /// @c PersistentType values.
    ///
    /// Unlike @c RNTupleWriter which uses @c shared_ptr to values, this alias
    /// resolves to plain value types suitable for direct TTree branch binding.
    template<typename AStdTuple>
    struct PersistentEntryTuple;
    template<typename... AFields>
    struct PersistentEntryTuple<std::tuple<AFields...>> {
        using Type = std::tuple<typename AFields::PersistentType...>;
    };

private:
    std::unique_ptr<typename PersistentEntryTuple<typename M::StdTuple>::Type> fEntry; ///< Tuple of persistent-type values, one per model field.
    std::unique_ptr<TTree> fTree;                                                      ///< Underlying ROOT TTree.
};

} // namespace Mustard::Data::inline Processing

#include "Mustard/Data/Processing/TTreeWriter.inl"
