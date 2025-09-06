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

#include "Mustard/Geant4X/Physics/MuoniumSMAndLFVDecayPhysics.h++"
#include "Mustard/Geant4X/Physics/MuoniumSMAndLFVDecayPhysicsMessenger.h++"

#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcommand.hh"
#include "G4UIdirectory.hh"

namespace Mustard::Geant4X::inline Physics {

MuoniumSMAndLFVDecayPhysicsMessenger::MuoniumSMAndLFVDecayPhysicsMessenger() :
    SingletonMessenger{},
    fDirectory{},
    fDoubleRadiativeDecayBR{},
    fElectronPairDecayBR{},
    fUpdateDecayBR{},
    fResetDecayBR{} {

    fDirectory = std::make_unique<G4UIdirectory>("/Mustard/Physics/MuoniumDecay/");
    fDirectory->SetGuidance("About muonium rare decay channels.");

    fDoubleRadiativeDecayBR = std::make_unique<G4UIcmdWithADouble>("/Mustard/Physics/MuoniumDecay/DoubleRadiativeDecay/BR", this);
    fDoubleRadiativeDecayBR->SetGuidance("Set branching ratio for muonium double radiative decay channel.");
    fDoubleRadiativeDecayBR->SetParameterName("BR", false);
    fDoubleRadiativeDecayBR->SetRange("0 <= BR && BR <= 1");
    fDoubleRadiativeDecayBR->AvailableForStates(G4State_PreInit, G4State_Idle);

    fElectronPairDecayBR = std::make_unique<G4UIcmdWithADouble>("/Mustard/Physics/MuoniumDecay/ElectronPairDecay/BR", this);
    fElectronPairDecayBR->SetGuidance("Set branching ratio for muonium pair production decay channel.");
    fElectronPairDecayBR->SetParameterName("BR", false);
    fElectronPairDecayBR->SetRange("0 <= BR && BR <= 1");
    fElectronPairDecayBR->AvailableForStates(G4State_PreInit, G4State_Idle);

    fUpdateDecayBR = std::make_unique<G4UIcmdWithoutParameter>("/Mustard/Physics/MuoniumDecay/UpdateDecayBR", this);
    fUpdateDecayBR->SetGuidance("Update decay branching ratios.");
    fUpdateDecayBR->AvailableForStates(G4State_Idle);

    fResetDecayBR = std::make_unique<G4UIcmdWithoutParameter>("/Mustard/Physics/MuonDecay/ResetDecayBR", this);
    fResetDecayBR->SetGuidance("Reset decay branching ratios.");
    fResetDecayBR->AvailableForStates(G4State_Idle);
}

MuoniumSMAndLFVDecayPhysicsMessenger::~MuoniumSMAndLFVDecayPhysicsMessenger() = default;

auto MuoniumSMAndLFVDecayPhysicsMessenger::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fDoubleRadiativeDecayBR.get()) {
        Deliver<MuoniumSMAndLFVDecayPhysics>([&](auto&& r) {
            r.DoubleRadiativeDecayBR(fDoubleRadiativeDecayBR->GetNewDoubleValue(value));
        });
    } else if (command == fElectronPairDecayBR.get()) {
        Deliver<MuoniumSMAndLFVDecayPhysics>([&](auto&& r) {
            r.ElectronPairDecayBR(fElectronPairDecayBR->GetNewDoubleValue(value));
        });
    } else if (command == fUpdateDecayBR.get()) {
        Deliver<MuoniumSMAndLFVDecayPhysics>([&](auto&& r) {
            r.UpdateDecayBR();
        });
    } else if (command == fResetDecayBR.get()) {
        Deliver<MuoniumSMAndLFVDecayPhysics>([&](auto&& r) {
            r.ResetDecayBR();
        });
    }
}

} // namespace Mustard::Geant4X::inline Physics
