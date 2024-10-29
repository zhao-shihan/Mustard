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

#include "Mustard/Extension/Geant4X/Generator/FromDataPrimaryGenerator.h++"
#include "Mustard/Extension/Geant4X/Generator/FromDataPrimaryGeneratorMessenger.h++"

#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcommand.hh"
#include "G4UIparameter.hh"
#include "G4ios.hh"

#include <string>

namespace Mustard::inline Extension::Geant4X::inline Generator {

FromDataPrimaryGeneratorMessenger::FromDataPrimaryGeneratorMessenger() :
    SingletonMessenger{},
    fDirectory{},
    fEventData{},
    fNVertex{} {

    fDirectory = std::make_unique<G4UIdirectory>("/Mustard/Generator/FromDataPrimaryGenerator/");
    fDirectory->SetGuidance("Vertex generator imported from event data.");

    fEventData = std::make_unique<G4UIcommand>("/Mustard/Generator/FromDataPrimaryGenerator/EventData", this);
    fEventData->SetGuidance("Set beam data ROOT file and dataset name.");
    fEventData->SetParameter(new G4UIparameter{"file", 's', false});
    fEventData->SetParameter(new G4UIparameter{"data", 's', false});
    fEventData->AvailableForStates(G4State_Idle);

    fNVertex = std::make_unique<G4UIcmdWithAnInteger>("/Mustard/Generator/FromDataPrimaryGenerator/NVertex", this);
    fNVertex->SetGuidance("Set number of particles to generate in an event.");
    fNVertex->SetParameterName("N", false);
    fNVertex->SetRange("N >= 0");
    fNVertex->AvailableForStates(G4State_Idle);
}

FromDataPrimaryGeneratorMessenger::~FromDataPrimaryGeneratorMessenger() = default;

auto FromDataPrimaryGeneratorMessenger::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fEventData.get()) {
        Deliver<FromDataPrimaryGenerator>([&](auto&& r) {
            const auto [file, data]{ParseFileNameAndObjectName(value)};
            try {
                r.EventData(std::string{file}, std::string{data});
            } catch (const std::runtime_error& e) {
                G4cerr << e.what() << G4endl;
            }
        });
    } else if (command == fNVertex.get()) {
        Deliver<FromDataPrimaryGenerator>([&](auto&& r) {
            r.NVertex(fNVertex->GetNewIntValue(value));
        });
    }
}

auto FromDataPrimaryGeneratorMessenger::ParseFileNameAndObjectName(std::string_view value) -> std::pair<std::string_view, std::string_view> {
    std::vector<std::string_view> parameter;
    parameter.reserve(2);
    for (auto&& token : value | std::views::split(' ')) {
        if (token.empty()) { continue; }
        parameter.emplace_back(token.begin(), token.end());
        if (parameter.size() == 2) { return {parameter.front(), parameter.back()}; }
    }
    return {};
}

} // namespace Mustard::inline Extension::Geant4X::inline Generator
