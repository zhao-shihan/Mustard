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

namespace Mustard::Geant4X::inline Physics {

template<TargetForMuoniumPhysics ATarget>
MuoniumPhysicsMessenger<ATarget>::MuoniumPhysicsMessenger() :
    Geant4X::SingletonMessenger<MuoniumPhysicsMessenger<ATarget>,
                                MuoniumFormation<ATarget>,
                                MuoniumThermalScattering<ATarget>>{},
    fMuoniumPhysicsDirectory{},
    fFormationProcessDirectory{},
    fConversionProbability{},
    fTransportProcessDirectory{},
    fManipulateAllSteps{} {
    fMuoniumPhysicsDirectory = std::make_unique<G4UIdirectory>("/Mustard/Physics/MuoniumPhysics/");
    fMuoniumPhysicsDirectory->SetGuidance("Physics of muonium and anti-muonium.");

    fFormationProcessDirectory = std::make_unique<G4UIdirectory>("/Mustard/Physics/MuoniumPhysics/Formation/");
    fFormationProcessDirectory->SetGuidance("Muonium formation and transition process.");

    fConversionProbability = std::make_unique<G4UIcmdWithADouble>("/Mustard/Physics/MuoniumPhysics/Formation/ConversionProbability", this);
    fConversionProbability->SetGuidance("Set integrated probability of muonium to anti-muonium conversion.");
    fConversionProbability->SetParameterName("P", false);
    fConversionProbability->SetRange("0 <= P && P <= 1");
    fConversionProbability->AvailableForStates(G4State_Idle);

    fTransportProcessDirectory = std::make_unique<G4UIdirectory>("/Mustard/Physics/MuoniumPhysics/Transport/");
    fTransportProcessDirectory->SetGuidance("The thermal scattering process of atoms.");

    fManipulateAllSteps = std::make_unique<G4UIcmdWithABool>("/Mustard/Physics/MuoniumPhysics/Transport/DisableCoarseGraining", this),
    fManipulateAllSteps->SetGuidance("If set, disable coarse graining in atom thermal scattering process.\n"
                                     "Warning: just for test purposes and can be time consuming if set.");
    fManipulateAllSteps->SetParameterName("bool", false);
    fManipulateAllSteps->AvailableForStates(G4State_Idle);
}

template<TargetForMuoniumPhysics ATarget>
auto MuoniumPhysicsMessenger<ATarget>::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fConversionProbability.get()) {
        this->template Deliver<MuoniumFormation<ATarget>>([&](auto&& r) {
            r.ConversionProbability(fConversionProbability->GetNewDoubleValue(value));
        });
    } else if (command == fManipulateAllSteps.get()) {
        this->template Deliver<MuoniumThermalScattering<ATarget>>([&](auto&& r) {
            r.DisableCoarseGraining(fManipulateAllSteps->GetNewBoolValue(value));
        });
    }
}

} // namespace Mustard::Geant4X::inline Physics
