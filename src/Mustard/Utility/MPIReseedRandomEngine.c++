// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
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

#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Env/Memory/Singleton.h++"
#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256PP.h++"
#include "Mustard/Utility/MPIReseedRandomEngine.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "CLHEP/Random/Random.h"

#include "TRandom.h"

#include "mpi.h"

#include "gsl/gsl"

#include <array>
#include <bit>
#include <cassert>
#include <concepts>
#include <numeric>
#include <set>
#include <utility>

namespace Mustard::inline Utility {

namespace internal {
namespace {

template<std::unsigned_integral T>
auto MasterMakeUniqueSeedSeries(auto xsr256Seed) -> std::set<T> {
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    assert(mpiEnv.OnCommWorldMaster());

    static_assert(std::same_as<Math::Random::Xoshiro256PP::SeedType, std::uint64_t>);
    Math::Random::Xoshiro256PP xsr256{std::bit_cast<std::uint64_t>(xsr256Seed)};
    Math::Random::Uniform<T> uniform{1, std::numeric_limits<T>::max() - 1}; // not 0x00...00 and not 0xff...ff

    std::set<T> uniqueSeeds;
    const auto worldSize{mpiEnv.CommWorldSize()};
    do {
        uniqueSeeds.emplace(uniform(xsr256));
    } while (ssize(uniqueSeeds) < worldSize);
    return uniqueSeeds;
}

} // namespace
} // namespace internal

auto MPIReseedRandomEngine(CLHEP::HepRandomEngine* clhepRng, TRandom* tRandom) -> void {
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    if (mpiEnv.Sequential()) { return; }

    if (clhepRng == nullptr) { clhepRng = CLHEP::HepRandom::getTheEngine(); }
    if (tRandom == nullptr) { tRandom = gRandom; }

    struct Seed {
        bool clhepNull{true};
        bool rootNull{true};
        long clhep{};
        unsigned root{};
    };
    static_assert(std::same_as<decltype(Seed::clhep), long>);
    static_assert(std::same_as<decltype(Seed::clhep), decltype(clhepRng->getSeed())>);
    static_assert(std::same_as<decltype(Seed::root), unsigned>);
    static_assert(std::same_as<decltype(Seed::root), decltype(tRandom->GetSeed())>);

    MPI_Datatype structSeed;
    MPI_Type_create_struct(4,                                                 // count
                           std::array<int, 4>{1,                              // array_of_block_lengths
                                              1,                              // array_of_block_lengths
                                              1,                              // array_of_block_lengths
                                              1}                              // array_of_block_lengths
                               .data(),                                       // array_of_block_lengths
                           std::array<MPI_Aint, 4>{offsetof(Seed, clhepNull), // array_of_displacements
                                                   offsetof(Seed, rootNull),  // array_of_displacements
                                                   offsetof(Seed, clhep),     // array_of_displacements
                                                   offsetof(Seed, root)}      // array_of_displacements
                               .data(),                                       // array_of_displacements
                           std::array<MPI_Datatype, 4>{MPI_CXX_BOOL,          // array_of_types
                                                       MPI_CXX_BOOL,          // array_of_types
                                                       MPI_LONG,              // array_of_types
                                                       MPI_UNSIGNED}          // array_of_types
                               .data(),                                       // array_of_types
                           &structSeed);                                      // newtype

    std::vector<Seed> seedSend(mpiEnv.CommWorldSize());
    if (mpiEnv.OnCommWorldMaster()) {
        if (clhepRng != nullptr) {
            std::array<unsigned int, sizeof(std::uint64_t) / sizeof(unsigned int)> xsr256Seed;
            std::ranges::generate(xsr256Seed, [&] { return clhepRng->operator unsigned int(); });
            const auto uniqueSeed{internal::MasterMakeUniqueSeedSeries<unsigned long>(xsr256Seed)};
            assert(uniqueSeed.size() == seedSend.size());
            for (gsl::index i{}; const auto& s : uniqueSeed) {
                seedSend[i].clhepNull = false;
                seedSend[i++].clhep = std::bit_cast<long>(s);
            }
        }
        if (tRandom != nullptr) {
            static_assert(std::same_as<decltype(tRandom->Integer(-1) + 1), unsigned>);
            std::array<unsigned, sizeof(std::uint64_t) / sizeof(unsigned)> xsr256Seed;
            std::ranges::generate(xsr256Seed, [&] { return tRandom->Integer(-1) + 1; });
            const auto uniqueSeed{internal::MasterMakeUniqueSeedSeries<unsigned>(xsr256Seed)};
            assert(uniqueSeed.size() == seedSend.size());
            for (gsl::index i{}; const auto& s : uniqueSeed) {
                seedSend[i].rootNull = false;
                seedSend[i++].root = s;
            }
        }
    }

    Seed seedRecv;
    MPI_Type_commit(&structSeed);
    MPI_Scatter(seedSend.data(), // sendbuf
                1,               // sendcount
                structSeed,      // sendtype
                &seedRecv,       // recvbuf
                1,               // recvcount
                structSeed,      // recvtype
                0,               // root
                MPI_COMM_WORLD); // comm
    MPI_Type_free(&structSeed);

    if (seedRecv.clhepNull xor (clhepRng == nullptr)) { Throw<std::logic_error>("CLHEP random engine null/!null inconsistent"); }
    if (seedRecv.rootNull xor (tRandom == nullptr)) { Throw<std::logic_error>("ROOT random engine null/!null inconsistent"); }
    if (clhepRng != nullptr) {
        assert(seedRecv.clhep != 0 and seedRecv.clhep != -1); // not 0x00...00 and not 0xff...ff
        clhepRng->setSeed(seedRecv.clhep, 3);
    }
    if (tRandom != nullptr) {
        assert(seedRecv.root != 0 and seedRecv.root != static_cast<decltype(seedRecv.root)>(-1)); // not 0x00...00 and not 0xff...ff
        tRandom->SetSeed(seedRecv.root);
    }
}

} // namespace Mustard::inline Utility
