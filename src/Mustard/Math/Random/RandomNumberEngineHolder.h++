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

#include "Mustard/Utility/ClassSemanticHelper.h++"

#include "muc/random"

#include <concepts>
#include <type_traits>

namespace Mustard::inline Math::Random {

/// @brief Concept of a random number engine that is either held by an object
/// or referred to by a reference.
/// @details @c R is either a random number engine type (an object is held) or
/// an lvalue reference to a random number engine (an object is referred to).
/// Const references and rvalue references are rejected: holding an engine
/// requires the right to advance its state.
template<typename R>
concept RandomNumberEngineOrReference =
    muc::random_number_engine<std::remove_cvref_t<R>> and
    (std::same_as<R, std::remove_cvref_t<R>> or
     std::same_as<R, std::remove_cvref_t<R>&>);

/// @brief Holder of a random number engine, either owned or referred to.
/// @tparam R A random number engine type (the holder owns an engine of that
/// type) or an lvalue reference to one (the holder refers to an engine
/// belonging to someone else).
/// @note A holder of a reference is a plain view: it neither creates, destroys,
/// nor seeds the engine it refers to.
template<RandomNumberEngineOrReference R>
class RandomNumberEngineHolder {
public:
    /// @brief The random number engine type, with any reference removed.
    using EngineType = std::remove_reference_t<R>;
    /// @brief The type the engine is seeded with, that is, its @c result_type,
    /// from which all engines admitted by RandomNumberEngineOrReference are
    /// seedable.
    using SeedType = typename EngineType::result_type;

protected:
    /// @brief Construct a holder owning a default-state engine.
    /// @note Only available when @c R is an engine type.
    constexpr RandomNumberEngineHolder()
        requires(not std::is_reference_v<R>);
    /// @brief Construct a holder owning an engine seeded with @p seed.
    /// @param seed Initialization seed of the engine.
    /// @note Only available when @c R is an engine type.
    constexpr explicit RandomNumberEngineHolder(SeedType seed)
        requires(not std::is_reference_v<R>);
    /// @brief Construct a holder referring to @p engine.
    /// @param engine Engine to refer to.
    /// @pre @p engine outlives this holder.
    /// @note Only available when @c R is an engine reference type.
    constexpr explicit RandomNumberEngineHolder(EngineType& engine)
        requires(std::is_reference_v<R>);
    MUSTARD_UTILITY_DECLARE_NONVIRTUAL_PROTECTED_DEFAULT_DESTRUCTOR_IN_DEFAULT_MOVEABLE(RandomNumberEngineHolder)

public:
    /// @brief Access the held engine.
    /// @return Reference to the held engine.
    constexpr auto Engine() -> auto& { return fEngine; }
    /// @brief Access the held engine.
    /// @return Reference to the held engine.
    constexpr auto Engine() const -> const auto& { return fEngine; }

private:
    R fEngine;
};

} // namespace Mustard::inline Math::Random

#include "Mustard/Math/Random/RandomNumberEngineHolder.inl"
