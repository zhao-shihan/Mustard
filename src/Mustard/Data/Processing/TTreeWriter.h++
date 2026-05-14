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
#include "Mustard/Data/Object/Field.h++"
#include "Mustard/Data/Object/FieldTypeName.h++"
#include "Mustard/Data/Object/Tuple.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Utility/NonCopyableBase.h++"

#include "TDirectory.h"
#include "TTree.h"

#include "gsl/gsl"

#include "fmt/format.h"

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <optional>
#include <ranges>
#include <string>
#include <type_traits>
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
/// @note This class is non-copyable.
/// @note Final persistence is handled by writer destruction
/// (i.e. the underlying @c TTree is written once on destruction).
template<Modelized M>
class TTreeWriter : public NonCopyableBase {
public:
    using Model = M;

public:
    /// @brief Construct a writer and create the target @c TTree with branches.
    ///
    /// If @p name contains '/', the prefix is treated as a ROOT directory path
    /// (created on demand) and the suffix as tree name.
    ///
    /// Internally this wrapper constructs @c TTree in the resolved ROOT directory,
    /// then creates one branch per model field and configures each branch title
    /// from the field type and field metadata.
    ///
    /// @param name Tree name, or directory path plus tree name.
    explicit TTreeWriter(const std::string& name);

    /// @brief Write tree content to the ROOT file on destruction.
    ~TTreeWriter();

    /// @brief Fill one entry into the underlying tree.
    /// @param tuple Tuple object to write.
    auto Fill(const Tuple<M>& tuple) -> void { FillImpl(tuple); }
    /// @brief Fill one entry into the underlying tree.
    /// @param tuple Tuple object to write.
    auto Fill(Tuple<M>&& tuple) -> void { FillImpl(std::move(tuple)); }
    /// @brief Fill one entry into the underlying tree
    /// if the shared entry object is not null.
    /// @param arcTuple Shared tuple object to write. Null is ignored.
    auto Fill(const ArcTuple<M>& arcTuple) -> void;
    /// @brief Fill one entry into the underlying tree
    /// if the shared entry object is not null.
    /// @param arcTuple Shared tuple object to write. Null is ignored.
    auto Fill(ArcTuple<M>&& arcTuple) -> void;

    /// @brief Fill a range of entries in iteration order.
    ///
    /// Elements are forwarded to @ref Fill. For lvalue ranges, elements are
    /// passed as lvalues; for rvalue ranges, elements are moved when possible.
    ///
    /// @param data Input range whose elements are consumable by @ref Fill.
    template<std::ranges::input_range R>
    auto Fill(R&& data) -> void;

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
    template<typename ATuple>
        requires std::same_as<std::remove_cvref_t<ATuple>, Tuple<M>>
    auto FillImpl(ATuple&& tuple) -> void;

private:
    /// @brief Maps a std::tuple of Field types to a std::tuple of PersistentTypes.
    template<typename AStdTuple>
    struct PersistentEntryTuple;
    template<typename... AFields>
    struct PersistentEntryTuple<std::tuple<AFields...>> {
        using Type = std::tuple<typename AFields::PersistentType...>;
    };

private:
    typename PersistentEntryTuple<typename M::StdTuple>::Type fEntry;
    std::optional<TTree> fTree;
};

} // namespace Mustard::Data::inline Processing

#include "Mustard/Data/Processing/TTreeWriter.inl"
