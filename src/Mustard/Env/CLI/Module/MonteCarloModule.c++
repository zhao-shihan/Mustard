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

#include "Mustard/Env/CLI/Module/MonteCarloModule.h++"

#include "CLHEP/Random/Random.h"

#include "TRandom.h"

#include <bit>
#include <random>

namespace Mustard::Env::CLI::inline Module {

MonteCarloModule::MonteCarloModule(argparse::ArgumentParser& argParser) :
    ModuleBase{argParser} {
    ArgParser()
        .add_argument("--seed")
        .help("Set random seed. 0 means using random device (non deterministic random seed). Predefined deterministic seed is used by default.")
        .scan<'i', long>();
}

auto MonteCarloModule::SeedRandomIfFlagged() const -> bool {
    auto seed{ArgParser().present<long>("--seed")};
    if (not seed.has_value()) {
        return false;
    }
    const auto theSeed{*seed != 0 ? *seed :
                                    std::bit_cast<int>(std::random_device{}())};
    if (const auto clhepRandom{CLHEP::HepRandom::getTheEngine()};
        clhepRandom) {
        clhepRandom->setSeed(theSeed, 3);
    }
    if (gRandom) {
        // Try to decorrelate with CLHEP
        const auto rootSeed{std::mt19937_64{static_cast<unsigned long>(theSeed)}()};
        gRandom->SetSeed(rootSeed);
    }
    return true;
}

} // namespace Mustard::Env::CLI::inline Module
