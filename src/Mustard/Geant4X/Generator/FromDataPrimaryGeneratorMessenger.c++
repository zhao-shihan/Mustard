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

#include "Mustard/Geant4X/Generator/FromDataPrimaryGenerator.h++"
#include "Mustard/Geant4X/Generator/FromDataPrimaryGeneratorMessenger.h++"

#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcommand.hh"
#include "G4UIparameter.hh"
#include "G4ios.hh"

#include <filesystem>
#include <sstream>
#include <string>

namespace Mustard::Geant4X::inline Generator {

FromDataPrimaryGeneratorMessenger::FromDataPrimaryGeneratorMessenger() :
    SingletonMessenger{},
    fDirectory{},
    fEventData{},
    fNVertex{} {

    fDirectory = std::make_unique<G4UIdirectory>("/Mustard/Generator/FromDataPrimaryGenerator/");
    fDirectory->SetGuidance("Primary vertex generator imported from event data.");

    fEventData = std::make_unique<G4UIcommand>("/Mustard/Generator/FromDataPrimaryGenerator/EventData", this);
    fEventData->SetGuidance("Set primary vertices data ROOT file and dataset name.");
    fEventData->SetParameter(new G4UIparameter{"file", 's', false});
    fEventData->SetParameter(new G4UIparameter{"data", 's', false});
    fEventData->AvailableForStates(G4State_Idle);

    fNVertex = std::make_unique<G4UIcmdWithAnInteger>("/Mustard/Generator/FromDataPrimaryGenerator/NVertex", this);
    fNVertex->SetGuidance("Set number of vertices to generate in an event.");
    fNVertex->SetParameterName("N", false);
    fNVertex->SetRange("N >= 0");
    fNVertex->AvailableForStates(G4State_Idle);
}

FromDataPrimaryGeneratorMessenger::~FromDataPrimaryGeneratorMessenger() = default;

auto FromDataPrimaryGeneratorMessenger::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fEventData.get()) {
        Deliver<FromDataPrimaryGenerator>([&](auto&& r) {
            std::filesystem::path file;
            std::string data;
            std::istringstream is{value};
            is >> file >> data;
            r.EventData(file, data);
        });
    } else if (command == fNVertex.get()) {
        Deliver<FromDataPrimaryGenerator>([&](auto&& r) {
            r.NVertex(fNVertex->GetNewIntValue(value));
        });
    }
}

} // namespace Mustard::Geant4X::inline Generator
