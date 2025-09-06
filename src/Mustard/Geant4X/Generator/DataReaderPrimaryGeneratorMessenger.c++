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

#include "Mustard/Geant4X/Generator/DataReaderPrimaryGenerator.h++"
#include "Mustard/Geant4X/Generator/DataReaderPrimaryGeneratorMessenger.h++"

#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcommand.hh"
#include "G4UIparameter.hh"
#include "G4ios.hh"

#include <filesystem>
#include <sstream>
#include <string>

namespace Mustard::Geant4X::inline Generator {

DataReaderPrimaryGeneratorMessenger::DataReaderPrimaryGeneratorMessenger() :
    SingletonMessenger{},
    fDirectory{},
    fEventData{},
    fNVertex{} {

    fDirectory = std::make_unique<G4UIdirectory>("/Mustard/Generator/DataReaderPrimaryGenerator/");
    fDirectory->SetGuidance("Primary vertex generator imported from event data.");

    fEventData = std::make_unique<G4UIcommand>("/Mustard/Generator/DataReaderPrimaryGenerator/EventData", this);
    fEventData->SetGuidance("Set primary vertices data ROOT file and dataset name.");
    fEventData->SetParameter(new G4UIparameter{"file", 's', false});
    fEventData->SetParameter(new G4UIparameter{"data", 's', false});
    fEventData->AvailableForStates(G4State_Idle);

    fNVertex = std::make_unique<G4UIcmdWithAnInteger>("/Mustard/Generator/DataReaderPrimaryGenerator/NVertex", this);
    fNVertex->SetGuidance("Set number of vertices to generate in an event.");
    fNVertex->SetParameterName("N", false);
    fNVertex->SetRange("N >= 0");
    fNVertex->AvailableForStates(G4State_Idle);
}

DataReaderPrimaryGeneratorMessenger::~DataReaderPrimaryGeneratorMessenger() = default;

auto DataReaderPrimaryGeneratorMessenger::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fEventData.get()) {
        Deliver<DataReaderPrimaryGenerator>([&](auto&& r) {
            std::filesystem::path file;
            std::string data;
            std::istringstream is{value};
            is >> file >> data;
            r.EventData(file, data);
        });
    } else if (command == fNVertex.get()) {
        Deliver<DataReaderPrimaryGenerator>([&](auto&& r) {
            r.NVertex(fNVertex->GetNewIntValue(value));
        });
    }
}

} // namespace Mustard::Geant4X::inline Generator
