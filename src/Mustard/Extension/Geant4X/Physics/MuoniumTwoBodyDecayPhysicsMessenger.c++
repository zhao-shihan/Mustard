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

#include "Mustard/Extension/Geant4X/Physics/MuoniumTwoBodyDecayPhysics.h++"
#include "Mustard/Extension/Geant4X/Physics/MuoniumTwoBodyDecayPhysicsMessenger.h++"

#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcommand.hh"
#include "G4UIdirectory.hh"

namespace Mustard::inline Extension::Geant4X::inline Physics {

MuoniumTwoBodyDecayPhysicsMessenger::MuoniumTwoBodyDecayPhysicsMessenger() :
    SingletonMessenger{},
    fDirectory{},
    fAnnihilationDecayBR{},
    fM2eeDecayBR{},
    fUpdateDecayBR{} {

    fDirectory = std::make_unique<G4UIdirectory>("/Mustard/Physics/MuoniumDecay/");
    fDirectory->SetGuidance("About muon(ium) decay channel and decay generators.");

    fAnnihilationDecayBR = std::make_unique<G4UIcmdWithADouble>("/Mustard/Physics/MuoniumDecay/AnnihilationDecay/BR", this);
    fAnnihilationDecayBR->SetGuidance("Set branching ratio for muonium annihilation decay channel.");
    fAnnihilationDecayBR->SetParameterName("BR", false);
    fAnnihilationDecayBR->SetRange("0 <= BR && BR <= 1");
    fAnnihilationDecayBR->AvailableForStates(G4State_PreInit, G4State_Idle);

    fM2eeDecayBR = std::make_unique<G4UIcmdWithADouble>("/Mustard/Physics/MuoniumDecay/M2eeDecay/BR", this);
    fM2eeDecayBR->SetGuidance("Set branching ratio for muonium two-body decay channel.");
    fM2eeDecayBR->SetParameterName("BR", false);
    fM2eeDecayBR->SetRange("0 <= BR && BR <= 1");
    fM2eeDecayBR->AvailableForStates(G4State_PreInit, G4State_Idle);

    fUpdateDecayBR = std::make_unique<G4UIcmdWithoutParameter>("/Mustard/Physics/MuoniumDecay/UpdateDecayBR", this);
    fUpdateDecayBR->SetGuidance("Update decay branching ratio.");
    fUpdateDecayBR->AvailableForStates(G4State_Idle);
}

MuoniumTwoBodyDecayPhysicsMessenger::~MuoniumTwoBodyDecayPhysicsMessenger() = default;

auto MuoniumTwoBodyDecayPhysicsMessenger::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fAnnihilationDecayBR.get()) {
        Deliver<MuoniumTwoBodyDecayPhysics>([&](auto&& r) {
            r.AnnihilationDecayBR(fAnnihilationDecayBR->GetNewDoubleValue(value));
        });
    } else if (command == fM2eeDecayBR.get()) {
        Deliver<MuoniumTwoBodyDecayPhysics>([&](auto&& r) {
            r.M2eeDecayBR(fM2eeDecayBR->GetNewDoubleValue(value));
        });
    } else if (command == fUpdateDecayBR.get()) {
        Deliver<MuoniumTwoBodyDecayPhysics>([&](auto&& r) {
            r.UpdateDecayBR();
        });
    }
}

} // namespace Mustard::inline Extension::Geant4X::inline Physics
