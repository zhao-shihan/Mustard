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

#include "Mustard/Geant4X/Physics/ThermalScatteringMessenger.h++"
#include "Mustard/Geant4X/Process/ThermalScattering.h++"

namespace Mustard::Geant4X::inline Physics {

ThermalScatteringMessenger::ThermalScatteringMessenger() :
    SingletonMessenger<ThermalScatteringMessenger,
                       ThermalScattering>{},
    fThermalScatteringDirectory{},
    fDisableCoarseGraining{},
    fMinCoarseGrainingScale{} {
    fThermalScatteringDirectory = std::make_unique<G4UIdirectory>("/Mustard/Physics/ThermalScattering/");
    fThermalScatteringDirectory->SetGuidance("Generic thermal scattering process.");

    fDisableCoarseGraining = std::make_unique<G4UIcmdWithABool>("/Mustard/Physics/ThermalScattering/DisableCoarseGraining", this);
    fDisableCoarseGraining->SetGuidance("If set, disable coarse graining in thermal scattering process.\n"
                                        "Warning: just for test purposes and can be time consuming if set.");
    fDisableCoarseGraining->SetParameterName("bool", false);
    fDisableCoarseGraining->AvailableForStates(G4State_Idle);

    fMinCoarseGrainingScale = std::make_unique<G4UIcmdWithADouble>("/Mustard/Physics/ThermalScattering/MinCoarseGrainingScale", this);
    fMinCoarseGrainingScale->SetGuidance("Set the minimum coarse-graining scale (in units of the mean free path).");
    fMinCoarseGrainingScale->SetParameterName("scale", false);
    fMinCoarseGrainingScale->AvailableForStates(G4State_Idle);
}

auto ThermalScatteringMessenger::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fDisableCoarseGraining.get()) {
        Deliver<ThermalScattering>([&](auto&& r) {
            r.DisableCoarseGraining(fDisableCoarseGraining->GetNewBoolValue(value));
        });
    } else if (command == fMinCoarseGrainingScale.get()) {
        Deliver<ThermalScattering>([&](auto&& r) {
            r.MinCoarseGrainingScale(fMinCoarseGrainingScale->GetNewDoubleValue(value));
        });
    }
}

} // namespace Mustard::Geant4X::inline Physics
