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

#include "Mustard/Geant4X/DecayChannel/MuonNeutrinolessDoubleRadiativeDecayChannel.h++"
#include "Mustard/IO/PrettyLog.h++"

#include "fmt/core.h"

#include <stdexcept>

namespace Mustard::Geant4X::inline DecayChannel {

MuonNeutrinolessDoubleRadiativeDecayChannel::MuonNeutrinolessDoubleRadiativeDecayChannel(const G4String& parentName, G4double br, G4int verbose) :
    G4PhaseSpaceDecayChannel{parentName, br, 3, DaughterLeptonName(parentName), "gamma", "gamma"} {
    SetVerboseLevel(verbose);
}

auto MuonNeutrinolessDoubleRadiativeDecayChannel::DaughterLeptonName(const G4String& parentName) -> G4String {
    if (parentName == "mu-") {
        return "e-";
    } else if (parentName == "mu+") {
        return "e+";
    } else {
        Throw<std::invalid_argument>(fmt::format("Parent particle is not mu- or mu+ but {}", parentName));
    }
}

} // namespace Mustard::Geant4X::inline DecayChannel
