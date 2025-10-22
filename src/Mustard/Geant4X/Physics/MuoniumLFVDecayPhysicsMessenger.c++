// -*- C++ -*-
//
// Copyright (C) 2020-2025  Mustard developers
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

#include "Mustard/Geant4X/Physics/MuoniumLFVDecayPhysics.h++"
#include "Mustard/Geant4X/Physics/MuoniumLFVDecayPhysicsMessenger.h++"

#include "G4UIcmdWithADouble.hh"
#include "G4UIcommand.hh"

namespace Mustard::Geant4X::inline Physics {

MuoniumLFVDecayPhysicsMessenger::MuoniumLFVDecayPhysicsMessenger() :
    SingletonMessenger{},
    fAnnihilativeDecayBR{},
    fElectronPairDecayBR{} {
    fAnnihilativeDecayBR = std::make_unique<G4UIcmdWithADouble>("/Mustard/Physics/AnnihilativeDecay/ElectronPairDecay/BR", this);
    fAnnihilativeDecayBR->SetGuidance("Set branching ratio for muonium annihilative decay channel.");
    fAnnihilativeDecayBR->SetParameterName("BR", false);
    fAnnihilativeDecayBR->SetRange("0 <= BR && BR <= 1");
    fAnnihilativeDecayBR->AvailableForStates(G4State_PreInit, G4State_Idle);

    fElectronPairDecayBR = std::make_unique<G4UIcmdWithADouble>("/Mustard/Physics/MuoniumDecay/ElectronPairDecay/BR", this);
    fElectronPairDecayBR->SetGuidance("Set branching ratio for muonium pair production decay channel.");
    fElectronPairDecayBR->SetParameterName("BR", false);
    fElectronPairDecayBR->SetRange("0 <= BR && BR <= 1");
    fElectronPairDecayBR->AvailableForStates(G4State_PreInit, G4State_Idle);
}

MuoniumLFVDecayPhysicsMessenger::~MuoniumLFVDecayPhysicsMessenger() = default;

auto MuoniumLFVDecayPhysicsMessenger::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fAnnihilativeDecayBR.get()) {
        Deliver<MuoniumLFVDecayPhysics>([&](auto&& r) {
            r.AnnihilativeDecayBR(fAnnihilativeDecayBR->GetNewDoubleValue(value));
        });
    } else if (command == fElectronPairDecayBR.get()) {
        Deliver<MuoniumLFVDecayPhysics>([&](auto&& r) {
            r.ElectronPairDecayBR(fElectronPairDecayBR->GetNewDoubleValue(value));
        });
    }
}

} // namespace Mustard::Geant4X::inline Physics
