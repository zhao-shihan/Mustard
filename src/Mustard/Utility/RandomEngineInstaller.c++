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

#include "Mustard/CLI/Module/MonteCarloModule.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256StarStar.h++"
#include "Mustard/Parallel/ReseedRandomEngine.h++"
#include "Mustard/Utility/RandomEngineInstaller.h++"

#include "CLHEP/Random/Random.h"

#include "TRandom.h"

#include <utility>

namespace Mustard::inline Utility {

RandomEngineInstaller::RandomEngineInstaller(muc::optional_ref<const CLI::CLI<>> cli) :
    RandomEngineInstaller{{}, cli} {}

RandomEngineInstaller::RandomEngineInstaller(std::optional<muc::any_rng> engine, muc::optional_ref<const CLI::CLI<>> cli) :
    NoncopyableBase{},
    fEngine{engine ? std::move(*engine) : muc::any_rng{Random::Xoshiro256StarStar{}}},
    fCLHEPEngine{fEngine},
    fROOTEngine{fEngine} {
    // Install the unified engine
    CLHEP::HepRandom::setTheEngine(&fCLHEPEngine);
    delete gRandom, gRandom = &fROOTEngine;
    if (cli.has_value()) {
        if (const auto monteCarloModule{dynamic_cast<const CLI::MonteCarloModule*>(&cli.value().get())}) {
            // Select the engine through --rng, if given, then seed it through --seed, if given
            if (auto selectedEngine{monteCarloModule->RandomEngineIfFlagged()}) {
                fEngine = std::move(*selectedEngine);
            }
            static_cast<void>(monteCarloModule->SeedRandomIfFlagged(fEngine));
        }
    }
    // Reseed in parallel computing
    Parallel::ReseedRandomEngine(fEngine);
}

RandomEngineInstaller::~RandomEngineInstaller() {
    CLHEP::HepRandom::setTheEngine(nullptr);
    gRandom = nullptr;
}

} // namespace Mustard::inline Utility
