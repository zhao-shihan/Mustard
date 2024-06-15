#pragma once

#include "Mustard/Extension/Geant4X/DecayChannel/AsMuoniumDecayChannel.h++"

#include "G4MuonRadiativeDecayChannelWithSpin.hh"

namespace Mustard::inline Extension::Geant4X::inline DecayChannel {

using MuoniumRadiativeDecayChannelWithSpin = AsMuoniumDecayChannel<G4MuonRadiativeDecayChannelWithSpin,
                                                                   "MuoniumRadiativeDecayChannelWithSpin">;

} // namespace Mustard::inline Extension::Geant4X::inline DecayChannel
