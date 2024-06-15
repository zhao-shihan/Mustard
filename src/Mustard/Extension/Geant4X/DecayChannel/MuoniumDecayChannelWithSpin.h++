#pragma once

#include "Mustard/Extension/Geant4X/DecayChannel/AsMuoniumDecayChannel.h++"

#include "G4MuonDecayChannelWithSpin.hh"

namespace Mustard::inline Extension::Geant4X::inline DecayChannel {

using MuoniumDecayChannelWithSpin = AsMuoniumDecayChannel<G4MuonDecayChannelWithSpin,
                                                          "MuoniumDecayWithSpin">;

} // namespace Mustard::inline Extension::Geant4X::inline DecayChannel
