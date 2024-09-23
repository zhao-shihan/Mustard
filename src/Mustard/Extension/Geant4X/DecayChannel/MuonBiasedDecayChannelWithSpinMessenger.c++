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

#include "Mustard/Extension/Geant4X/DecayChannel/MuonBiasedDecayChannelWithSpin.h++"
#include "Mustard/Extension/Geant4X/DecayChannel/MuonBiasedDecayChannelWithSpinMessenger.h++"

#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcommand.hh"
#include "G4UIdirectory.hh"

namespace Mustard::inline Extension::Geant4X::inline DecayChannel {

MuonBiasedDecayChannelWithSpinMessenger::MuonBiasedDecayChannelWithSpinMessenger() :
    SingletonMessenger{},
    fDirectory{},
    fEnergyCut{} {

    fDirectory = std::make_unique<G4UIdirectory>("/Mustard/Physics/MuonDecay/");
    fDirectory->SetGuidance("About muon(ium) decay channel and decay generators.");

    fEnergyCut = std::make_unique<G4UIcmdWithADouble>("/Mustard/Physics/MuonDecay/Biasing/Energy", this);
    fEnergyCut->SetGuidance("Set energy cut for muon(ium) decay channel.");
    fEnergyCut->SetParameterName("E", false);
    fEnergyCut->SetRange("0 <= E");
    fEnergyCut->AvailableForStates(G4State_PreInit, G4State_Idle);
}

MuonBiasedDecayChannelWithSpinMessenger::~MuonBiasedDecayChannelWithSpinMessenger() = default;

auto MuonBiasedDecayChannelWithSpinMessenger::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fEnergyCut.get()) {
        Deliver<MuonBiasedDecayChannelWithSpin>([&](auto&& r) {
            r.EnergyCut(fEnergyCut->GetNewDoubleValue(value));
        });
    }
}

} // namespace Mustard::inline Extension::Geant4X::inline DecayChannel
