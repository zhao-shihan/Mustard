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

#include "muc/random"

#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace CLHEP {
class HepRandomEngine;
} // namespace CLHEP
class TRandom;

namespace Mustard::Parallel {

/// @brief Reseeds @p engine so that every MPI rank continues with a distinct stream.
///
/// The master rank draws a new master seed from the current state of @p engine
/// and derives a series of unique seeds from it, which is then scattered to all
/// ranks. Every rank, including the master one, finally reseeds @p engine with
/// its own seed, taken from [1, max(E::result_type) - 1].
///
/// @tparam E A random number engine satisfying muc::random_number_engine.
/// @param engine Engine to reseed; normally the unified engine of the process.
///
/// @note Collective MPI operation: every rank must call it, and all ranks must
/// call the same overload, since the overloads do not agree on the seed range.
/// @note No-op if MPI is unavailable or a single rank is running, in which case
/// @p engine is left untouched.
/// @note Distinct streams require the engine to map distinct seeds onto distinct
/// states. An engine that folds its seed into a narrower state, e.g. one seeded
/// from an integer wider than its internal state, may in principle map two ranks
/// onto the same stream.
template<muc::random_number_engine E>
auto ReseedRandomEngine(E& engine) -> void;

/// @brief Reseeds @p engine so that every MPI rank continues with a distinct stream.
///
/// Same as the engine overload, but draws the master seed through the CLHEP
/// interface of @p engine and reseeds it with CLHEP's luxury level 3. Seeds are
/// taken from [1, max(long)], so they are directly representable in CLHEP's seed
/// type and `getSeed()` never reports a negative seed.
///
/// @param engine CLHEP engine to reseed; normally a view of the unified engine.
///
/// @note Collective MPI operation: every rank must call it, and all ranks must
/// call the same overload, since the overloads do not agree on the seed range.
/// @note No-op if MPI is unavailable or a single rank is running.
/// @note Distinct streams require the engine to map distinct seeds onto distinct
/// states, which fails for an engine whose own seed type is narrower than
/// @c long, e.g. a 32-bit engine.
/// @warning When several views refer to the same engine, reseed through one of
/// them only: each call scatters new seeds, and the last reseed wins.
auto ReseedRandomEngine(CLHEP::HepRandomEngine& engine) -> void;

/// @brief Reseeds @p engine so that every MPI rank continues with a distinct stream.
///
/// Same as the engine overload, but draws the master seed through the ROOT
/// interface of @p engine and reseeds it with TRandom::SetSeed(). Seeds are
/// taken from [1, max(unsigned) - 1], the width of the seed ROOT keeps in
/// `TRandom::GetSeed()`.
///
/// @param engine ROOT engine to reseed; normally a view of the unified engine.
///
/// @note Collective MPI operation: every rank must call it, and all ranks must
/// call the same overload, since the overloads do not agree on the seed range.
/// @note No-op if MPI is unavailable or a single rank is running.
/// @note Distinct streams require the engine to map distinct seeds onto distinct
/// states, which fails for an engine whose own seed type is narrower than
/// @c unsigned.
/// @warning When several views refer to the same engine, reseed through one of
/// them only: each call scatters new seeds, and the last reseed wins.
auto ReseedRandomEngine(TRandom& engine) -> void;

namespace impl {

/// @brief Draws master seed material of type @p T through @p draw.
/// @tparam T Integral type of the material; signed types are allowed, since
/// CLHEP seeds are signed.
/// @tparam ADraw Callable returning an unsigned integral random value.
/// @param draw Functor drawing a word of entropy from an engine; it is called
/// as many times as the material is wide, or once if a word is at least as wide.
/// @return The drawn material, its words laid out as by std::bit_cast; only the
/// value drawn on the master rank is used.
template<std::integral T, typename ADraw>
    requires std::unsigned_integral<std::invoke_result_t<ADraw&>>
auto DrawSeedMaterial(ADraw draw) -> T;

/// @brief Whether a collective reseed is needed in this process.
/// @return True if MPI is available and more than one rank is running.
auto ReseedNeeded() -> bool;

/// @brief Generates a distinct seed for every MPI rank, collectively.
/// @param material Master seed material; used on the master rank only.
/// @param supremum Largest seed to generate; seeds lie in [1, supremum].
/// @return Seed of the calling rank, lying in [1, @p supremum].
/// @pre ReseedNeeded() is true, and @p supremum is not less than the number of
/// ranks, so that distinct seeds are available.
/// @note Collective MPI operation; must be called by all ranks.
/// @note Callers passing `max(T) - 1` as @p supremum for some unsigned integral
/// type @c T get a value that is directly representable in @c T, hence
/// converting it to @c T never wraps around and never maps two ranks onto the
/// same seed.
auto RankSeed(std::uint64_t material, std::uint64_t supremum) -> std::uint64_t;

} // namespace impl

} // namespace Mustard::Parallel

#include "Mustard/Parallel/ReseedRandomEngine.inl"
