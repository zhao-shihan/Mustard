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

#include "Mustard/Geant4X/Physics/MuonLFVDecayPhysics.h++"
#include "Mustard/Geant4X/Physics/MuonLFVDecayPhysicsMessenger.h++"
#include "Mustard/Geant4X/Physics/MuoniumLFVDecayPhysics.h++"

#include "G4UIcmdWithADouble.hh"
#include "G4UIcommand.hh"

namespace Mustard::Geant4X::inline Physics {

MuonLFVDecayPhysicsMessenger::MuonLFVDecayPhysicsMessenger() :
    SingletonMessenger{},
    fDoubleRadiativeDecayBR{} {
    fDoubleRadiativeDecayBR = std::make_unique<G4UIcmdWithADouble>("/Mustard/Physics/MuonDecay/NeutrinolessDoubleRadiativeDecay/BR", this);
    fDoubleRadiativeDecayBR->SetGuidance("Set branching ratio for muon double radiative decay channel.");
    fDoubleRadiativeDecayBR->SetParameterName("BR", false);
    fDoubleRadiativeDecayBR->SetRange("0 <= BR && BR <= 1");
    fDoubleRadiativeDecayBR->AvailableForStates(G4State_PreInit, G4State_Idle);
}

MuonLFVDecayPhysicsMessenger::~MuonLFVDecayPhysicsMessenger() = default;

auto MuonLFVDecayPhysicsMessenger::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fDoubleRadiativeDecayBR.get()) {
        Deliver<MuonLFVDecayPhysics,
                MuoniumLFVDecayPhysics>([&](auto&& r) {
            r.DoubleRadiativeDecayBR(fDoubleRadiativeDecayBR->GetNewDoubleValue(value));
        });
    }
}

} // namespace Mustard::Geant4X::inline Physics
