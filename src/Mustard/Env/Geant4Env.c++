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

#include "Mustard/CLI/Module/DetectorDescriptionModule.h++"
#include "Mustard/CLI/Module/Geant4Module.h++"
#include "Mustard/Env/Geant4Env.h++"
#include "Mustard/IO/PrettyLog.h++"

#include "mplr/mplr.hpp"

namespace Mustard::Env {

Geant4Env::Geant4Env(NoBanner, int argc, char* argv[], CLI::CLI<>& cli,
                     enum VerboseLevel verboseLevel,
                     bool showBannerHint) :
    BasicEnv{{}, argc, argv, cli, verboseLevel, showBannerHint},
    MPIEnv{{}, argc, argv, cli, verboseLevel, showBannerHint},
    MonteCarloEnv<512>{{}, argc, argv, cli, verboseLevel, showBannerHint},
    PassiveSingleton<Geant4Env>{this} {
    if (not dynamic_cast<CLI::Geant4Module*>(&cli)) {
        Mustard::MasterPrintWarning("Geant4 CLI module (Mustard::CLI::Geant4Module) not found");
    }
    if (const auto ddCLI{dynamic_cast<CLI::DetectorDescriptionModule<>*>(&cli)}) {
        ddCLI->DetectorDescriptionIOIfFlagged();
    }
}

Geant4Env::Geant4Env(int argc, char* argv[], CLI::CLI<>& cli,
                     enum VerboseLevel verboseLevel,
                     bool showBannerHint) :
    Geant4Env{{}, argc, argv, cli, verboseLevel, showBannerHint} {
    if (fShowBanner and mplr::comm_world().rank() == 0) {
        PrintStartBannerSplitLine();
        PrintStartBannerBody(argc, argv);
        PrintStartBannerSplitLine();
    }
}

} // namespace Mustard::Env
