#pragma once

#include "Mustard/Extension/Geant4X/DecayChannel/AsMuoniumDecayChannel.h++"

#include "G4MuonDecayChannel.hh"

namespace Mustard::inline Extension::Geant4X::inline DecayChannel {

using MuoniumDecayChannel = AsMuoniumDecayChannel<G4MuonDecayChannel,
                                                  "MuoniumDecay">;

} // namespace Mustard::inline Extension::Geant4X::inline DecayChannel
