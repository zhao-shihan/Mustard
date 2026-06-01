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
#include "Mustard/Utility/MoveOnlyBase.h++"

#include "TDirectory.h"

#include "muc/utility"

#include "gsl/gsl"

#include "fmt/format.h"

#include <concepts>
#include <functional>
#include <ranges>
#include <string>
#include <type_traits>
#include <utility>

namespace Mustard::Data::inline Processing::impl3 {

/// @brief CRTP base class for writer backends.
///
/// Provides the common @ref Fill interface, shared constructor helpers
/// (@ref BuildFieldDescription , @ref WithSubdirectory), and a non-copyable
/// base. Derived classes supply backend-specific hooks:
/// - @c EntryRef<I>() returns an assignable lvalue reference to persistent storage.
/// - @c DoFill() commits the current entry to the backend.
///
/// @tparam M        Mustard data model type.
/// @tparam ADerived Concrete writer type (CRTP).
///
/// @note This class is non-copyable (inherits @c MoveOnlyBase).
template<Modelized M, typename ADerived>
class WriterBase : public MoveOnlyBase {
public:
    /// @brief Mustard data model type.
    using Model = M;

public:
    /// @brief Default constructor.
    /// @note A warning is printed if the current @c gDirectory is not writable.
    WriterBase();

    /// @brief Fill one entry.
    auto Fill(const Tuple<M>& tuple) -> void { FillImpl(tuple); }
    /// @brief Fill one entry.
    auto Fill(Tuple<M>&& tuple) -> void { FillImpl(std::move(tuple)); }

    /// @brief Fill one entry if the shared entry object is not null.
    auto Fill(const ArcTuple<M>& arcTuple) -> void;
    /// @brief Fill one entry if the shared entry object is not null.
    auto Fill(ArcTuple<M>&& arcTuple) -> void;

    /// @brief Fill a range of entries in iteration order.
    ///
    /// Elements are forwarded to @ref Fill preserving value category
    /// (lvalue elements are copied, rvalue elements are moved).
    ///
    /// @tparam R Input range type.
    /// @param data Input range whose elements are consumable by @ref Fill.
    template<std::ranges::input_range R>
    auto Fill(R&& data) -> void;

protected:
    /// @brief Build a human-readable description string for a model field.
    ///
    /// Returns a string of the form @c "(PersistentTypeName) description" if the
    /// field has a description, or just @c "PersistentTypeName" otherwise.
    ///
    /// @tparam Field Model field type, expected to expose @c PersistentType and
    ///               @c Name() / @c Description() members.
    /// @return A human-readable description string suitable for use as a branch
    ///         title or field metadata.
    template<typename Field>
    static auto BuildFieldDescription() -> std::string;

    /// @brief Create a ROOT subdirectory if @p name contains a path prefix,
    /// then invoke @p create with the object name in the resolved directory.
    ///
    /// If @p name contains a @c / separator, the prefix is treated as a ROOT
    /// directory path (created on demand) and the suffix as the object name.
    /// If no @c / is present, @p create is invoked directly with @p name.
    /// The original @c gDirectory is restored after the call via @c gsl::finally.
    ///
    /// @tparam F Callable type invocable with a @c const @c std::string&.
    /// @param name Full path string; may be a plain name or a directory-prefixed path.
    /// @param create Factory callable invoked with the resolved object name.
    template<typename F>
    static auto WithSubdirectory(const std::string& name, F&& create) -> void;

private:
    auto Self() -> ADerived& { return static_cast<ADerived&>(*this); }

    template<typename ATuple>
        requires std::same_as<std::remove_cvref_t<ATuple>, Tuple<M>>
    auto FillImpl(ATuple&& tuple) -> void;
};

} // namespace Mustard::Data::inline Processing::impl3

#include "Mustard/Data/Processing/impl3/WriterBase.inl"
