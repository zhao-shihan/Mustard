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

#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256PlusPlus.h++"
#include "Mustard/Parallel/ReseedRandomEngine.h++"

#include "CLHEP/Random/RandomEngine.h"

#include "TRandom.h"

#include "gtl/phmap.hpp"

#include "mplr/mplr.hpp"

#include "gsl/gsl"

#include <concepts>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

namespace Mustard::Parallel {

auto ReseedRandomEngine(CLHEP::HepRandomEngine& engine) -> void {
    static_assert(std::same_as<unsigned, decltype(engine.operator unsigned int())>);         // CLHEP hands out 32-bit words
    static_assert(sizeof(long) == sizeof(unsigned) or sizeof(long) == 2 * sizeof(unsigned)); // one or two words
    if (not impl::ReseedNeeded()) {
        return;
    }
    // CLHEP seeds are signed and its entropy comes in 32-bit words, so assemble
    // the material of the seed type from one or two words
    const auto material{impl::DrawSeedMaterial<long>([&engine] {
        unsigned word;
        do {
            word = engine.operator unsigned int();
        } while (word == 0 or word == std::numeric_limits<unsigned>::max()); // not 0x00...00 and not 0xff...ff
        return word;
    })};
    const auto rankSeed{static_cast<long>(impl::RankSeed(material, std::numeric_limits<long>::max()))};
    engine.setSeed(rankSeed, 3);
}

auto ReseedRandomEngine(TRandom& engine) -> void {
    static_assert(std::same_as<unsigned, decltype(engine.GetSeed())>); // TRandom keeps a 32-bit seed
    if (not impl::ReseedNeeded()) {
        return;
    }
    // ROOT both hands out and takes 32-bit values, so material and seed are 32-bit
    const auto material{impl::DrawSeedMaterial<unsigned>([&engine] {
        return engine.Integer(-2) + 1; // not 0x00...00
    })};
    const auto rankSeed{static_cast<unsigned long>(impl::RankSeed(material, std::numeric_limits<unsigned>::max() - 1))};
    engine.SetSeed(rankSeed);
}

namespace impl {
namespace {

/// @brief Generates a set of unique seeds for MPI ranks from @p material.
///
/// Helper function called only by MPI rank 0 to generate a set of unique seeds
/// for all MPI processes. The master seed material is stretched through a
/// Xoshiro256++ engine, whose state is fully determined by it.
///
/// @param material Seed material drawn from the engine to be reseeded.
/// @param worldSize Number of MPI processes to generate seeds for.
/// @param supremum Largest seed to generate; seeds lie in [1, supremum].
///
/// @return Unique seeds in [1, supremum], one for each MPI process.
///
/// @note Called exclusively by MPI rank 0 during seed generation.
/// @warning Seeds avoid 0, so that they never degenerate into a zero state.
auto MakeUniqueSeedSeries(std::uint64_t material, int worldSize, std::uint64_t supremum) -> std::vector<std::uint64_t> {
    static_assert(std::same_as<Random::Xoshiro256PlusPlus::SeedType, std::uint64_t>);
    Random::Xoshiro256PlusPlus master{material};
    Random::Uniform<std::uint64_t> uniform{1, supremum}; // not 0x00...00

    gtl::flat_hash_set<std::uint64_t> uniqueSeed;
    uniqueSeed.reserve(worldSize);
    do {
        uniqueSeed.emplace(uniform(master));
    } while (std::cmp_less(uniqueSeed.size(), worldSize));
    return {uniqueSeed.begin(), uniqueSeed.end()};
}

} // namespace

auto ReseedNeeded() -> bool {
    return mplr::available() and mplr::comm_world().size() > 1;
}

auto RankSeed(std::uint64_t material, std::uint64_t supremum) -> std::uint64_t {
    Expects(ReseedNeeded());
    const auto worldComm{mplr::comm_world()};
    Expects(std::cmp_less_equal(worldComm.size(), supremum)); // enough distinct seeds to go around
    std::uint64_t seed;
    if (worldComm.rank() == 0) {
        const auto seedSend{MakeUniqueSeedSeries(material, worldComm.size(), supremum)};
        Ensures(std::cmp_equal(seedSend.size(), worldComm.size()));
        worldComm.scatter(0, seedSend.data(), seed);
    } else {
        worldComm.scatter(0, seed);
    }
    Ensures(1 <= seed and seed <= supremum); // not 0x00...00
    return seed;
}

} // namespace impl

} // namespace Mustard::Parallel
