#pragma once

#include "Mustard/Extension/Geant4X/DecayChannel/AsMuoniumDecayChannel.h++"
#include "Mustard/Extension/Geant4X/DecayChannel/MuonInternalPairProductionDecayChannel.h++"

namespace Mustard::inline Extension::Geant4X::inline DecayChannel {

using MuoniumInternalPairProductionDecayChannel = Geant4X::AsMuoniumDecayChannel<MuonInternalPairProductionDecayChannel,
                                                                                 "MuoniumIPPDecay">;

} // namespace Mustard::inline Extension::Geant4X::inline DecayChannel
