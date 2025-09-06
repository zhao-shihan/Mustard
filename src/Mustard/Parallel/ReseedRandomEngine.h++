// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

namespace CLHEP {
class HepRandomEngine;
} // namespace CLHEP
class TRandom;

namespace Mustard::Parallel {

/// @brief Reseeds random engines for parallel MPI environments
///
/// Ensures independent random streams across MPI ranks by:
///   - Generating unique seeds on rank 0 using CLHEP/ROOT RNGs
///   - Scattering seeds to all ranks
///   - Reseeding local engines with rank-specific seeds
///
/// Behavior:
///   - No-op if MPI unavailable or world size = 1
///   - Uses CLHEP default engine if clhepRng=nullptr
///   - Uses ROOT gRandom if tRandom=nullptr
///   - Master RNG (Xoshiro256++) seeded from:
///       * CLHEP: Uniform integers in [1, max_unsigned-1]
///       * ROOT: TRandom::Integer() adjusted to avoid 0/max
///
/// @param clhepRng CLHEP engine to reseed (uses default if null)
/// @param tRandom ROOT engine to reseed (uses gRandom if null)
///
/// @throws std::invalid_argument If engine null-state mismatches between rank 0 and current rank
///
/// @note Collective MPI operation (must be called by all ranks)
/// @warning Seeds avoid 0 and max values to prevent engine-specific edge cases
/// @warning CLHEP seed set with luxury=3, ROOT with SetSeed()
/// @remark Intra-rank consistency checks prevent null/non-null engine mismatches
auto ReseedRandomEngine(CLHEP::HepRandomEngine* clhepRng = {}, TRandom* tRandom = {}) -> void;

} // namespace Mustard::Parallel
