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

#include "Mustard/Extension/Geant4X/DecayChannel/MuonInternalConversionDecayChannel.h++"
#include "Mustard/Extension/Geant4X/DecayChannel/MuonInternalConversionDecayChannelMessenger.h++"

#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIdirectory.hh"

namespace Mustard::inline Extension::Geant4X::inline DecayChannel {

MuonInternalConversionDecayChannelMessenger::MuonInternalConversionDecayChannelMessenger() :
    SingletonMessenger{},
    fDirectory{},
    fMetropolisDelta{},
    fMetropolisDiscard{} {

    fDirectory = std::make_unique<G4UIdirectory>("/Mustard/Physics/MuonDecay/ICDecay/");
    fDirectory->SetGuidance("Muon(ium) internal pair production decay channel (mu->eeevv / M->eeevve).");

    fMetropolisDelta = std::make_unique<G4UIcmdWithADouble>("/Mustard/Physics/MuonDecay/ICDecay/MetropolisDelta", this);
    fMetropolisDelta->SetGuidance("Set the 1D-displacement (20 dimensions in total) of the random walk in the Metropolis algorithm. "
                                  "Smaller values enhance autocorrelation, while larger values decrease performance and lead to biased results. "
                                  "The typical value is below 0.05.");
    fMetropolisDelta->SetParameterName("delta", false);
    fMetropolisDelta->SetRange("0 < delta && delta < 0.5");
    fMetropolisDelta->AvailableForStates(G4State_Idle);

    fMetropolisDiscard = std::make_unique<G4UIcmdWithAnInteger>("/Mustard/Physics/MuonDecay/ICDecay/MetropolisDiscard", this);
    fMetropolisDiscard->SetGuidance("Set how many samples are discarded between two outputs in the Metropolis algorithm. "
                                    "The more samples are discarded, the less significant the autocorrelation is, but it will reduce the performance of sampling. "
                                    "When the total number of samples (number of IPP decay events) is small, a larger number of discards should be set. "
                                    "When the number of samples is large, the number of discards can be appropriately reduced. "
                                    "This value is also related to the delta of the random walk, and the smaller the delta, the more samples that should be discarded.");
    fMetropolisDiscard->SetParameterName("n", false);
    fMetropolisDiscard->SetRange("n >= 0");
    fMetropolisDiscard->AvailableForStates(G4State_Idle);
}

MuonInternalConversionDecayChannelMessenger::~MuonInternalConversionDecayChannelMessenger() = default;

auto MuonInternalConversionDecayChannelMessenger::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fMetropolisDelta.get()) {
        Deliver<MuonInternalConversionDecayChannel>([&](auto&& r) {
            r.MetropolisDelta(fMetropolisDelta->GetNewDoubleValue(value));
        });
    } else if (command == fMetropolisDiscard.get()) {
        Deliver<MuonInternalConversionDecayChannel>([&](auto&& r) {
            r.MetropolisDiscard(fMetropolisDiscard->GetNewIntValue(value));
        });
    }
}

} // namespace Mustard::inline Extension::Geant4X::inline DecayChannel
