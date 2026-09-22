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

#include "Mustard/CLHEPX/Random/Xoshiro.h++"
#include "Mustard/CLI/MonteCarloCLI.h++"
#include "Mustard/Env/MPIMonteCarloEnv.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/IO/Print.h++"
#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256StarStar.h++"
#include "Mustard/Parallel/ReseedRandomEngine.h++"
#include "Mustard/ROOTX/Math/AsTRandomEngine.h++"
#include "Mustard/ROOTX/Math/Xoshiro.h++"
#include "Mustard/Testing/TestUnifiedRandomEngine/TestUnifiedRandomEngine.h++"

#include "CLHEP/Random/Random.h"

#include "TRandom.h"

#include "mplr/mplr.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace Mustard::Testing {

TestUnifiedRandomEngine::TestUnifiedRandomEngine() :
    Subprogram{"TestUnifiedRandomEngine", "Test that Mustard's unified random engine backs CLHEP and ROOT."} {}

auto TestUnifiedRandomEngine::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::MonteCarloCLI<> cli;
    Mustard::Env::MPIMonteCarloEnv<512> env{argc, argv, cli};

    auto& engine{env.RandomEngine()};
    auto& clhepEngine{env.CLHEPRandomEngine()};
    auto& rootEngine{env.ROOTRandomEngine()};
    using Engine = std::remove_reference_t<decltype(engine)>;
    using SeedType = typename Engine::SeedType;
    const auto worldSize{static_cast<int>(mplr::comm_world().size())};

    // The 256-bit width, which this test does not run, selects the 256-bit engine
    // and its two handles all the same
    using Env256 = Mustard::Env::MPIMonteCarloEnv<256>;
    static_assert(std::same_as<std::remove_reference_t<decltype(std::declval<Env256&>().RandomEngine())>,
                               Mustard::Random::Xoshiro256StarStar>);
    static_assert(std::same_as<std::remove_reference_t<decltype(std::declval<Env256&>().CLHEPRandomEngine())>,
                               CLHEPX::Random::AsHepRandomEngine<Mustard::Random::Xoshiro256StarStar&>>);
    static_assert(std::same_as<std::remove_reference_t<decltype(std::declval<Env256&>().ROOTRandomEngine())>,
                               ROOTX::Math::AsTRandom<Mustard::Random::Xoshiro256StarStar&>>);

    // The three handles are views of one and the same engine
    if (CLHEP::HepRandom::getTheEngine() != &clhepEngine) {
        Throw<std::runtime_error>("CLHEP's global engine is not the CLHEP handle of the unified engine.");
    }
    if (static_cast<TRandom*>(&rootEngine) != gRandom) {
        Throw<std::runtime_error>("ROOT's gRandom is not the ROOT handle of the unified engine.");
    }
    if (&clhepEngine.Engine() != &engine or &rootEngine.Engine() != &engine) {
        Throw<std::runtime_error>("A CLHEP or ROOT handle owns an engine instead of referring to the unified one.");
    }

    // A referring handle does not know how the engine was seeded
    if (clhepEngine.getSeed() != 0 or rootEngine.GetSeed() != 0) {
        Throw<std::runtime_error>("A referring handle claims to know the seed of an external engine.");
    }

    // The engine keeps its deterministic default state unless --seed says otherwise,
    // which is only observable when no collective reseed overwrites the seed
    if (worldSize == 1) {
        const auto cliSeed{cli->present<std::int64_t>("--seed")};
        if (cliSeed.has_value() and *cliSeed == 0) {
            PrintWarning("--seed 0 asks for a non-deterministic seed, which cannot be checked.");
        } else {
            Engine expected;
            if (cliSeed.has_value()) {
                expected.seed(static_cast<SeedType>(*cliSeed));
            }
            if (engine() != expected()) {
                Throw<std::runtime_error>("The unified engine was not seeded as the --seed option requested.");
            }
        }
    }

    // MPI ranks continue with distinct streams after the reseed the environment
    // did at construction, and reseeding through a handle gives them distinct
    // seeds of its own; both are checked before this test seeds the engine itself.
    // Gather one value from every rank and report whether all ranks differ
    const auto allRanksDistinct{[worldSize](auto draw) -> bool {
        std::vector<std::remove_cvref_t<decltype(draw())>> allDraw(worldSize);
        mplr::comm_world().allgather(draw(), allDraw.data());
        std::ranges::sort(allDraw);
        return std::ranges::adjacent_find(allDraw) == allDraw.end();
    }};

    if (worldSize > 1) {
        // The environment already reseeded the unified engine at construction
        if (not allRanksDistinct([&engine] { return static_cast<std::uint64_t>(engine()); })) {
            Throw<std::runtime_error>("MPI ranks share the same random number stream.");
        }
        // Reseeding through the CLHEP handle scatters a seed of its own to every rank
        Parallel::ReseedRandomEngine(clhepEngine);
        if (clhepEngine.getSeed() == 0 or not allRanksDistinct([&clhepEngine] { return clhepEngine.getSeed(); })) {
            Throw<std::runtime_error>("MPI ranks did not get distinct seeds from the CLHEP handle.");
        }
        // Reseeding through the ROOT handle does the same
        Parallel::ReseedRandomEngine(rootEngine);
        if (rootEngine.GetSeed() == 0 or not allRanksDistinct([&rootEngine] { return static_cast<unsigned long>(rootEngine.GetSeed()); })) {
            Throw<std::runtime_error>("MPI ranks did not get distinct seeds from the ROOT handle.");
        }
    } else {
        // Without MPI, reseeding is documented to leave the engine untouched
        const Engine snapshot{engine};
        Parallel::ReseedRandomEngine(engine);
        Parallel::ReseedRandomEngine(clhepEngine);
        Parallel::ReseedRandomEngine(rootEngine);
        if (engine != snapshot) {
            Throw<std::runtime_error>("Reseeding without MPI changed the unified engine.");
        }
        PrintWarning("Running with a single rank: MPI stream-distinctness checks were skipped.");
    }

    // Every handle advances the very same stream
    constexpr SeedType seed{0x123456789ABCDEF0};
    Engine reference{seed};
    engine.seed(seed);
    for (int i = 0; i < 1000; ++i) {
        if (clhepEngine.flat() != Random::Uniform<double>{}(reference)) {
            Throw<std::runtime_error>("The CLHEP handle draws from a different stream.");
        }
        if (rootEngine.Rndm() != Random::Uniform<double>{}(reference)) {
            Throw<std::runtime_error>("The ROOT handle draws from a different stream.");
        }
        if (engine() != reference()) {
            Throw<std::runtime_error>("The native engine draws from a different stream.");
        }
    }

    // Uniform values lie strictly inside the unit interval, as CLHEP and ROOT
    // require of flat() and Rndm()
    for (int i = 0; i < 100; ++i) {
        const auto u{clhepEngine.flat()};
        const auto v{rootEngine.Rndm()};
        if (not(0 < u and u < 1) or not(0 < v and v < 1)) {
            Throw<std::runtime_error>("A handle returned a value outside the open unit interval.");
        }
    }
    {
        std::array<double, 16> array;
        clhepEngine.flatArray(array.size(), array.data());
        if (std::ranges::any_of(array, [](double u) { return not(0 < u and u < 1); })) {
            Throw<std::runtime_error>("flatArray() returned a value outside the open unit interval.");
        }
        rootEngine.RndmArray(array.size(), array.data());
        if (std::ranges::any_of(array, [](double u) { return not(0 < u and u < 1); })) {
            Throw<std::runtime_error>("RndmArray() returned a value outside the open unit interval.");
        }
    }

    // The lightweight ROOT::Math::TRandomEngine adapter refers to the same engine
    ROOTX::Math::AsTRandomEngine<Engine&> rootMathEngine{engine};
    engine.seed(seed);
    reference.seed(seed);
    if (rootMathEngine.Rndm() != Random::Uniform<double>{}(reference)) {
        Throw<std::runtime_error>("The ROOT::Math handle draws from a different stream.");
    }
    if (rootMathEngine.IntRndm() != reference()) {
        Throw<std::runtime_error>("The ROOT::Math handle draws from a different stream.");
    }

    // A handle can save and restore the shared state, in memory and on disk
    std::ostringstream state;
    clhepEngine.put(state);
    const auto afterPut{clhepEngine.flat()};
    std::istringstream restoredState{state.str()};
    clhepEngine.get(restoredState);
    if (clhepEngine.flat() != afterPut) {
        Throw<std::runtime_error>("The CLHEP handle failed to restore the shared state from a stream.");
    }
    // ProcessSpecificFile is collective, hence the rank-uniform path
    const auto statusFile{(std::filesystem::temp_directory_path() / "test_unified_random_engine.status").string()};
    clhepEngine.saveStatus(statusFile.c_str());
    const auto afterSave{clhepEngine.flat()};
    if (mplr::comm_world().rank() == 0) {
        clhepEngine.showStatus();
    }
    clhepEngine.restoreStatus(statusFile.c_str());
    if (clhepEngine.flat() != afterSave) {
        Throw<std::runtime_error>("The CLHEP handle failed to restore the shared state from a file.");
    }

    // Setting a seed through a handle is recorded by it, and only by it
    constexpr long seedArray[]{7, 0};
    clhepEngine.setSeed(42, 0);
    if (clhepEngine.getSeed() != 42) {
        Throw<std::runtime_error>("The CLHEP handle did not record the seed of setSeed().");
    }
    clhepEngine.setSeeds(seedArray, 0);
    if (clhepEngine.getSeed() != seedArray[0]) {
        Throw<std::runtime_error>("The CLHEP handle did not record the seed of setSeeds().");
    }
    clhepEngine.setSeeds(nullptr, 0);
    if (clhepEngine.getSeed() != 0) {
        Throw<std::runtime_error>("The CLHEP handle did not forget the seed after a default reseed.");
    }

    // An owning handle draws exactly like a native engine seeded the same way,
    // each owning handle having an engine of its own
    {
        CLHEPX::Random::Xoshiro512StarStar clhepOwning{seed};
        ROOTX::Math::Xoshiro512StarStar rootOwning{seed};
        ROOTX::Math::AsTRandomEngine<Engine> rootMathOwning{seed};
        if (clhepOwning.getSeed() != static_cast<long>(seed)) {
            Throw<std::runtime_error>("An owning CLHEP handle did not record its seed.");
        }
        if (rootOwning.GetSeed() != static_cast<unsigned>(seed)) {
            Throw<std::runtime_error>("An owning ROOT handle did not record its seed.");
        }
        Engine clhepReference{seed};
        Engine rootReference{seed};
        Engine rootMathReference{seed};
        for (int i = 0; i < 10; ++i) {
            if (clhepOwning.flat() != Random::Uniform<double>{}(clhepReference)) {
                Throw<std::runtime_error>("An owning CLHEP handle draws from a different stream.");
            }
            if (rootOwning.Rndm() != Random::Uniform<double>{}(rootReference)) {
                Throw<std::runtime_error>("An owning ROOT handle draws from a different stream.");
            }
            if (rootMathOwning.Rndm() != Random::Uniform<double>{}(rootMathReference)) {
                Throw<std::runtime_error>("An owning ROOT::Math handle draws from a different stream.");
            }
            if (rootMathOwning.IntRndm() != rootMathReference()) {
                Throw<std::runtime_error>("An owning ROOT::Math handle draws from a different stream.");
            }
        }
        // A zero seed asks ROOT for a non-deterministic one, not for zero
        const auto previousSeed{rootOwning.GetSeed()};
        rootOwning.SetSeed(0);
        if (rootOwning.GetSeed() == 0 or rootOwning.GetSeed() == previousSeed) {
            Throw<std::runtime_error>("SetSeed(0) did not draw a non-deterministic seed.");
        }
    }

    PrintLn("CLHEP and ROOT share one single random number stream of {} MPI rank(s).", worldSize);
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
