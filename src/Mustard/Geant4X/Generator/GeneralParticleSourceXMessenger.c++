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

#include "Mustard/Geant4X/Generator/GeneralParticleSourceX.h++"
#include "Mustard/Geant4X/Generator/GeneralParticleSourceXMessenger.h++"

#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAnInteger.hh"

namespace Mustard::Geant4X::inline Generator {

GeneralParticleSourceXMessenger::GeneralParticleSourceXMessenger() :
    SingletonMessenger{},
    fDirectory{},
    fNVertex{},
    fPulseWidth{} {

    fDirectory = std::make_unique<G4UIdirectory>("/GPSX/");
    fDirectory->SetGuidance("General particle source extension.");

    fNVertex = std::make_unique<G4UIcmdWithAnInteger>("/GPSX/NVertex", this);
    fNVertex->SetGuidance("Set number of vertices to generate in an event.");
    fNVertex->SetParameterName("N", false);
    fNVertex->SetRange("N >= 0");
    fNVertex->AvailableForStates(G4State_Idle);

    fPulseWidth = std::make_unique<G4UIcmdWithADoubleAndUnit>("/GPSX/PulseWidth", this);
    fPulseWidth->SetGuidance("Set pulse width of vertex time.");
    fPulseWidth->SetParameterName("T", false);
    fPulseWidth->SetUnitCategory("Time");
    fPulseWidth->AvailableForStates(G4State_Idle);
}

GeneralParticleSourceXMessenger::~GeneralParticleSourceXMessenger() = default;

auto GeneralParticleSourceXMessenger::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fNVertex.get()) {
        Deliver<GeneralParticleSourceX>([&](auto&& r) {
            r.NVertex(fNVertex->GetNewIntValue(value));
        });
    } else if (command == fPulseWidth.get()) {
        Deliver<GeneralParticleSourceX>([&](auto&& r) {
            r.PulseWidth(fPulseWidth->GetNewDoubleValue(value));
        });
    }
}

} // namespace Mustard::Geant4X::inline Generator
