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
#include "Mustard/Parallel/ReseedRandomEngine.h++"
#include "Mustard/Utility/UseXoshiro.h++"

#include "CLHEP/Random/Random.h"

#include "TRandom.h"

namespace Mustard::inline Utility {

template<unsigned ABitWidth>
    requires(ABitWidth == 256 or ABitWidth == 512)
UseXoshiro<ABitWidth>::UseXoshiro(muc::optional_ref<const CLI::CLI<>> cli) :
    NoncopyableBase{},
    fEngine{},
    fCLHEPEngine{fEngine},
    fROOTEngine{fEngine} {
    // Install the unified engine
    CLHEP::HepRandom::setTheEngine(&fCLHEPEngine);
    delete gRandom, gRandom = &fROOTEngine;
    // Seed the unified engine from CLI (if the Monte Carlo module is present
    // and the --seed option is set); otherwise keep its default state
    if (cli.has_value()) {
        if (const auto monteCarloModule{dynamic_cast<const CLI::MonteCarloModule*>(&cli.value().get())}) {
            static_cast<void>(monteCarloModule->SeedRandomIfFlagged(fEngine));
        }
    }
    // Reseed in parallel computing
    Parallel::ReseedRandomEngine(fEngine);
}

template<unsigned ABitWidth>
    requires(ABitWidth == 256 or ABitWidth == 512)
UseXoshiro<ABitWidth>::~UseXoshiro() {
    CLHEP::HepRandom::setTheEngine(nullptr);
    gRandom = nullptr;
}

template class UseXoshiro<256>;
template class UseXoshiro<512>;

} // namespace Mustard::inline Utility
