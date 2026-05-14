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
#include "Mustard/Data/Object/FieldTypeName.h++"
#include "Mustard/Data/Object/Tuple.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Utility/NonCopyableBase.h++"

#include "ROOT/RNTupleModel.hxx"
#include "ROOT/RNTupleWriter.hxx"
#include "TDirectory.h"

#include "gsl/gsl"

#include "fmt/format.h"

#include <concepts>
#include <cstddef>
#include <memory>
#include <ranges>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

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
/// @note This class is non-copyable.
/// @note Dataset finalization is handled by ROOT writer destruction
/// (i.e. the underlying @c ROOT::RNTupleWriter commits on destruction).
template<Modelized M>
class RNTupleWriter : public NonCopyableBase {
public:
    using Model = M;

public:
    /// @brief Construct a writer and create the target @c RNTuple with fields.
    ///
    /// If @p name contains '/', the prefix is treated as a ROOT directory path
    /// (created on demand) and the suffix as ntuple name.
    ///
    /// Internally this wrapper uses @c ROOT::RNTupleWriter::Append against the
    /// resolved ROOT directory.
    ///
    /// @param name Ntuple name, or directory path plus ntuple name.
    explicit RNTupleWriter(const std::string& name);

    /// @brief Fill one entry into the underlying ntuple.
    /// @param tuple Tuple object to write.
    auto Fill(const Tuple<M>& tuple) -> void { FillImpl(tuple); }
    /// @brief Fill one entry into the underlying ntuple.
    /// @param tuple Tuple object to write.
    auto Fill(Tuple<M>&& tuple) -> void { FillImpl(std::move(tuple)); }
    /// @brief Fill one entry into the underlying ntuple
    /// if the shared entry object is not null.
    /// @param arcTuple Shared tuple object to write. Null is ignored.
    auto Fill(const ArcTuple<M>& arcTuple) -> void;
    /// @brief Fill one entry into the underlying ntuple
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
    template<typename ATuple>
        requires std::same_as<std::remove_cvref_t<ATuple>, Tuple<M>>
    auto FillImpl(ATuple&& tuple) -> void;

private:
    template<typename AStdTuple>
    struct SharedPtrTuple;
    template<typename... AValues>
    struct SharedPtrTuple<std::tuple<AValues...>> {
        using Type = std::tuple<std::shared_ptr<typename AValues::PersistentType>...>;
    };

private:
    typename SharedPtrTuple<typename M::StdTuple>::Type fEntry;
    std::unique_ptr<ROOT::RNTupleWriter> fWriter;
};

} // namespace Mustard::Data::inline Processing

#include "Mustard/Data/Processing/RNTupleWriter.inl"
