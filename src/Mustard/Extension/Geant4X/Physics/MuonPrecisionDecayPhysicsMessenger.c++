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

#include "Mustard/Extension/Geant4X/Physics/MuonPrecisionDecayPhysics.h++"
#include "Mustard/Extension/Geant4X/Physics/MuonPrecisionDecayPhysicsMessenger.h++"

#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcommand.hh"
#include "G4UIdirectory.hh"

namespace Mustard::inline Extension::Geant4X::inline Physics {

MuonPrecisionDecayPhysicsMessenger::MuonPrecisionDecayPhysicsMessenger() :
    SingletonMessenger{},
    fDirectory{},
    fRadiativeDecayBR{},
    fICDecayBR{},
    fUpdateDecayBR{} {

    fDirectory = std::make_unique<G4UIdirectory>("/Mustard/Physics/MuonDecay/");
    fDirectory->SetGuidance("About muon(ium) decay channel and decay generators.");

    fRadiativeDecayBR = std::make_unique<G4UIcmdWithADouble>("/Mustard/Physics/MuonDecay/RadiativeDecay/BR", this);
    fRadiativeDecayBR->SetGuidance("Set branching ratio for muon(ium) internal pair production decay channel.");
    fRadiativeDecayBR->SetParameterName("BR", false);
    fRadiativeDecayBR->SetRange("0 <= BR && BR <= 1");
    fRadiativeDecayBR->AvailableForStates(G4State_PreInit, G4State_Idle);

    fICDecayBR = std::make_unique<G4UIcmdWithADouble>("/Mustard/Physics/MuonDecay/ICDecay/BR", this);
    fICDecayBR->SetGuidance("Set branching ratio for muon(ium) internal pair production decay channel.");
    fICDecayBR->SetParameterName("BR", false);
    fICDecayBR->SetRange("0 <= BR && BR <= 1");
    fICDecayBR->AvailableForStates(G4State_PreInit, G4State_Idle);

    fUpdateDecayBR = std::make_unique<G4UIcmdWithoutParameter>("/Mustard/Physics/MuonDecay/UpdateDecayBR", this);
    fUpdateDecayBR->SetGuidance("Update decay branching ratio.");
    fUpdateDecayBR->AvailableForStates(G4State_Idle);
}

MuonPrecisionDecayPhysicsMessenger::~MuonPrecisionDecayPhysicsMessenger() = default;

auto MuonPrecisionDecayPhysicsMessenger::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fRadiativeDecayBR.get()) {
        Deliver<MuonPrecisionDecayPhysics>([&](auto&& r) {
            r.RadiativeDecayBR(fRadiativeDecayBR->GetNewDoubleValue(value));
        });
    } else if (command == fICDecayBR.get()) {
        Deliver<MuonPrecisionDecayPhysics>([&](auto&& r) {
            r.ICDecayBR(fICDecayBR->GetNewDoubleValue(value));
        });
    } else if (command == fUpdateDecayBR.get()) {
        Deliver<MuonPrecisionDecayPhysics>([&](auto&& r) {
            r.UpdateDecayBR();
        });
    }
}

} // namespace Mustard::inline Extension::Geant4X::inline Physics
