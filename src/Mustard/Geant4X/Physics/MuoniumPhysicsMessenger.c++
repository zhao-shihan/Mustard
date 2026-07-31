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

#include "G4UIcmdWithADouble.hh"
#include "G4UIdirectory.hh"

namespace Mustard::Geant4X::inline Physics {

MuoniumPhysicsMessenger::MuoniumPhysicsMessenger() :
    SingletonMessenger<MuoniumPhysicsMessenger,
                       MuoniumFormation>{},
    fMuoniumPhysicsDirectory{},
    fDefaultParaMuoniumFraction{} {
    fMuoniumPhysicsDirectory = std::make_unique<G4UIdirectory>("/Mustard/Physics/MuoniumPhysics/");
    fMuoniumPhysicsDirectory->SetGuidance("Physics of muonium and anti-muonium.");

    fDefaultParaMuoniumFraction = std::make_unique<G4UIcmdWithADouble>("/Mustard/Physics/MuoniumPhysics/ParaFraction", this);
    fDefaultParaMuoniumFraction->SetGuidance("Set the default para-muonium fraction in muonium formation. Default is 0.25 (spin-statistics).");
    fDefaultParaMuoniumFraction->SetParameterName("frac", false);
    fDefaultParaMuoniumFraction->SetRange("0 <= frac && frac <= 1");
    fDefaultParaMuoniumFraction->AvailableForStates(G4State_PreInit, G4State_Idle);
}

MuoniumPhysicsMessenger::~MuoniumPhysicsMessenger() = default;

auto MuoniumPhysicsMessenger::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fDefaultParaMuoniumFraction.get()) {
        Deliver<MuoniumFormation>([&](auto&& r) {
            r.DefaultParaMuoniumFraction(fDefaultParaMuoniumFraction->GetNewDoubleValue(value));
        });
    }
}

} // namespace Mustard::Geant4X::inline Physics
