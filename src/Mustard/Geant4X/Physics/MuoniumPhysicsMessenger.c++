// -*- C++ -*-
//
// Copyright (C) 2020-2026  Shihan Zhao and contributors
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

#include "Mustard/Geant4X/Physics/MuoniumPhysicsMessenger.h++"
#include "Mustard/Geant4X/Process/MuoniumFormation.h++"

namespace Mustard::Geant4X::inline Physics {

MuoniumPhysicsMessenger::MuoniumPhysicsMessenger() :
    SingletonMessenger<MuoniumPhysicsMessenger,
                       MuoniumFormation>{},
    fMuoniumPhysicsDirectory{},
    fFormationProcessDirectory{},
    fConversionProbability{} {
    fMuoniumPhysicsDirectory = std::make_unique<G4UIdirectory>("/Mustard/Physics/MuoniumPhysics/");
    fMuoniumPhysicsDirectory->SetGuidance("Physics of muonium and anti-muonium.");

    fFormationProcessDirectory = std::make_unique<G4UIdirectory>("/Mustard/Physics/MuoniumPhysics/Formation/");
    fFormationProcessDirectory->SetGuidance("Muonium formation and transition process.");

    fConversionProbability = std::make_unique<G4UIcmdWithADouble>("/Mustard/Physics/MuoniumPhysics/Formation/ConversionProbability", this);
    fConversionProbability->SetGuidance("Set integrated probability of muonium to anti-muonium conversion.");
    fConversionProbability->SetParameterName("P", false);
    fConversionProbability->SetRange("0 <= P && P <= 1");
    fConversionProbability->AvailableForStates(G4State_Idle);
}

auto MuoniumPhysicsMessenger::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fConversionProbability.get()) {
        Deliver<MuoniumFormation>([&](auto&& r) {
            r.ConversionProbability(fConversionProbability->GetNewDoubleValue(value));
        });
    }
}

} // namespace Mustard::Geant4X::inline Physics
