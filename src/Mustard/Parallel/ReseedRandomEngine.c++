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

#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256PlusPlus.h++"
#include "Mustard/Parallel/ReseedRandomEngine.h++"

#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/Random.h"

#include "TRandom.h"

#include "mplr/mplr.hpp"

#include "muc/hash_set"

#include "gsl/gsl"

#include <array>
#include <bit>
#include <concepts>
#include <cstdint>
#include <exception>
#include <limits>
#include <numeric>
#include <tuple>
#include <utility>
#include <vector>

namespace Mustard::Parallel {

namespace internal {
namespace {

/// @brief Generates a set of unique seeds for MPI ranks using a master RNG
///
/// Helper function called only by MPI rank 0 to generate a set of unique seeds
/// for all MPI processes. Uses Xoshiro256++ RNG initialized with the provided seed.
///
/// @tparam T Unsigned integral type for seeds (must be same width as seed components)
/// @param xsr256Seed Seed value for Xoshiro256++ (bit-castable to uint64_t)
///
/// @return Set of unique seeds in range [1, max-1] with size = MPI world size
///
/// @note Called exclusively by MPI rank 0 during seed generation
/// @warning Must satisfy: worldComm.rank() == 0 (enforced by Expects)
template<std::unsigned_integral T>
auto MasterMakeUniqueSeedSeries(auto xsr256Seed) -> muc::flat_hash_set<T> {
    const auto worldComm{mplr::comm_world()};
    Expects(worldComm.rank() == 0);

    static_assert(std::same_as<Math::Random::Xoshiro256PlusPlus::SeedType, std::uint64_t>);
    Math::Random::Xoshiro256PlusPlus xsr256{std::bit_cast<std::uint64_t>(xsr256Seed)};
    Math::Random::Uniform<T> uniform{1, std::numeric_limits<T>::max() - 1}; // not 0x00...00 and not 0xff...ff

    muc::flat_hash_set<T> uniqueSeeds;
    const auto worldSize{worldComm.size()};
    uniqueSeeds.reserve(worldSize);
    do {
        uniqueSeeds.emplace(uniform(xsr256));
    } while (ssize(uniqueSeeds) < worldSize);
    return uniqueSeeds;
}

} // namespace
} // namespace internal

auto ReseedRandomEngine(CLHEP::HepRandomEngine* clhepRng, TRandom* tRandom) -> void {
    if (not mplr::available()) {
        return;
    }
    const auto worldComm{mplr::comm_world()};
    if (worldComm.size() == 1) {
        return;
    }

    if (clhepRng == nullptr) {
        clhepRng = CLHEP::HepRandom::getTheEngine();
    }
    if (tRandom == nullptr) {
        tRandom = gRandom;
    }

    static_assert(std::same_as<long, decltype(clhepRng->getSeed())>);
    static_assert(std::same_as<unsigned, decltype(tRandom->GetSeed())>);

    using Seed = std::tuple<bool, long, bool, unsigned>; // (clhepNull, clhepSeed, rootNull, rootSeed)
    Seed seedRecv;
    if (worldComm.rank() == 0) {
        std::vector<Seed> seedSend(worldComm.size(), {true, 0, true, 0});
        if (clhepRng) {
            std::array<unsigned, sizeof(std::uint64_t) / sizeof(unsigned)> xsr256Seed;
            std::ranges::generate(xsr256Seed, [clhepRng] {
                unsigned seed;
                do {
                    seed = clhepRng->operator unsigned int();
                } while (seed == 0 or seed == std::numeric_limits<unsigned>::max());
                return seed;
            });
            const auto uniqueSeed{internal::MasterMakeUniqueSeedSeries<unsigned long>(xsr256Seed)};
            Ensures(uniqueSeed.size() == seedSend.size());
            for (gsl::index i{}; const auto& s : uniqueSeed) {
                get<0>(seedSend[i]) = false;
                get<1>(seedSend[i++]) = std::bit_cast<long>(s);
            }
        }
        if (tRandom) {
            static_assert(std::same_as<decltype(tRandom->Integer(-2) + 1), unsigned>);
            std::array<unsigned, sizeof(std::uint64_t) / sizeof(unsigned)> xsr256Seed;
            std::ranges::generate(xsr256Seed, [tRandom] {
                return tRandom->Integer(-2) + 1;
            });
            const auto uniqueSeed{internal::MasterMakeUniqueSeedSeries<unsigned>(xsr256Seed)};
            Ensures(uniqueSeed.size() == seedSend.size());
            for (gsl::index i{}; const auto& s : uniqueSeed) {
                get<2>(seedSend[i]) = false;
                get<3>(seedSend[i++]) = s;
            }
        }
        worldComm.scatter(0, seedSend.data(), seedRecv);
    } else {
        worldComm.scatter(0, seedRecv);
    }

    if (get<0>(seedRecv) != (clhepRng == nullptr)) {
        Throw<std::invalid_argument>("CLHEP random engine null/!null inconsistent");
    }
    if (get<2>(seedRecv) != (tRandom == nullptr)) {
        Throw<std::invalid_argument>("ROOT random engine null/!null inconsistent");
    }
    if (clhepRng) {
        Ensures(get<1>(seedRecv) != 0 and get<1>(seedRecv) != -1); // not 0x00...00 and not 0xff...ff
        clhepRng->setSeed(get<1>(seedRecv), 3);
    }
    if (tRandom) {
        Ensures(get<3>(seedRecv) != 0 and get<3>(seedRecv) != static_cast<unsigned>(-1)); // not 0x00...00 and not 0xff...ff
        tRandom->SetSeed(get<3>(seedRecv));
    }
}

} // namespace Mustard::Parallel
