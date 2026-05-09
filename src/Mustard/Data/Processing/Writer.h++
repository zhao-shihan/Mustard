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

#include "Mustard/Data/Model.h++"
#include "Mustard/Data/Object/Tuple.h++"
#include "Mustard/Data/Processing/RNTupleWriter.h++"
#include "Mustard/Data/Processing/TTreeWriter.h++"
#include "Mustard/IO/PrettyLog.h++"

#include "envparse/parse.h++"

#include "fmt/format.h"

#include <cstddef>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

namespace Mustard::Data::inline Processing {

/// @brief Writes data into either a ROOT @c TTree or @c RNTuple.
///
/// @tparam M Mustard data model type used by @c Tuple and @c ArcTuple.
///
/// This class is a thin routing wrapper over @c TTreeWriter and
/// @c RNTupleWriter. It delegates all filling and flushing operations to the
/// selected backend.
///
/// The backend can be selected explicitly by constructor argument, or by the
/// @c MUSTARD_DATA_WRITER_DEFAULT_TARGET environment variable. Accepted values
/// are @c TTree and @c RNTuple.
///
/// @par Typical usage
/// @code{.cpp}
/// Mustard::Data::Writer<MyModel> writer{"analysis/data"};
/// writer.Fill(oneEntry);
/// writer.Flush();
/// writer.Fill(batchEntries);
/// @endcode
///
/// @note This class is non-copyable.
template<Modelized M>
class Writer {
public:
    using Model = M;

public:
    /// @brief Construct a writer by reading the default target from the
    /// @c MUSTARD_DATA_WRITER_DEFAULT_TARGET environment variable.
    ///
    /// Empty or unset values default to @c RNTuple. Invalid values log a
    /// warning and default to @c RNTuple.
    ///
    /// @param name Output name passed to the selected backend.
    explicit Writer(const std::string& name);
    /// @brief Construct a writer by explicitly selecting the backend.
    ///
    /// Accepted target names are @c TTree and @c RNTuple. An empty target
    /// defaults to @c RNTuple. Any other target logs a warning and also
    /// defaults to @c RNTuple.
    ///
    /// @param name Output name passed to the selected backend.
    /// @param target Backend selector string.
    explicit Writer(const std::string& name, std::string_view target);

    /// @brief Fill one entry into the underlying backend.
    /// @param tuple Tuple object to write.
    auto Fill(const Tuple<M>& tuple) -> void;
    /// @brief Fill one entry into the underlying backend.
    /// @param tuple Tuple object to write.
    auto Fill(Tuple<M>&& tuple) -> void;
    /// @brief Fill one entry into the underlying backend
    /// if the shared entry object is not null.
    /// @param arcTuple Shared tuple object to write. Null is ignored.
    auto Fill(const ArcTuple<M>& arcTuple) -> void;
    /// @brief Fill one entry into the underlying backend
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

    /// @brief Number of entries already filled into the underlying backend.
    /// @return Current entry count.
    auto NEntry() const -> std::size_t;

    /// @brief Flush current backend state to storage without finalizing the writer.
    ///
    /// This delegates to the selected backend's flush operation.
    ///
    /// @warning Frequent flushing can reduce I/O throughput. For @c TTree backend,
    /// it can also increase output file size.
    auto Flush() -> void;

private:
    template<typename F>
    auto VisitWriter(F&& visitor) const -> decltype(auto);
    template<typename F>
    auto VisitWriter(F&& visitor) -> decltype(auto);

    enum struct Target {
        RNTuple,
        TTree
    };
    static auto ResolveTarget(std::string_view target) -> Target;

private:
    std::optional<std::variant<TTreeWriter<M>, RNTupleWriter<M>>> fWriter;
};

} // namespace Mustard::Data::inline Processing

#include "Mustard/Data/Processing/Writer.inl"
