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

namespace Mustard::Geant4X::inline Interface {

template<typename ADerived, std::derived_from<G4VUserDetectorConstruction> ADetectorConstruction, muc::ceta_string AAppName>
    requires requires { typename ADetectorConstruction::ProminentDescription; }
DetectorMessenger<ADerived, ADetectorConstruction, AAppName>::DetectorMessenger() :
    Geant4X::SingletonMessenger<ADerived>{},
    fDirectory{},
    fImportDescription{},
    fExportDescription{},
    fEmportDescription{} {
    static_assert(std::derived_from<ADerived, DetectorMessenger<ADerived, ADetectorConstruction, AAppName>>);

    fDirectory = std::make_unique<G4UIdirectory>("/Mustard/Detector/");

    fImportDescription = std::make_unique<G4UIcmdWithAString>("/Mustard/Detector/Description/Import", this);
    fImportDescription->SetGuidance("Import detector description required by this program from a yaml file.");
    fImportDescription->SetParameterName("yaml", false);
    fImportDescription->AvailableForStates(G4State_PreInit);

    fExportDescription = std::make_unique<G4UIcmdWithAString>("/Mustard/Detector/Description/Export", this);
    fExportDescription->SetGuidance("Export detector description used by this program to a yaml file.");
    fExportDescription->SetParameterName("yaml", false);
    fExportDescription->AvailableForStates(G4State_PreInit, G4State_Idle);

    fEmportDescription = std::make_unique<G4UIcmdWithAString>("/Mustard/Detector/Description/Emport", this);
    fEmportDescription->SetGuidance("Export, Import, then export detector description used by this program. "
                                    "Exported files have '.prev' (previous) or '.curr' (current) suffix, respectively.");
    fEmportDescription->SetParameterName("yaml", false);
    fEmportDescription->AvailableForStates(G4State_PreInit);
}

template<typename ADerived, std::derived_from<G4VUserDetectorConstruction> ADetectorConstruction, muc::ceta_string AAppName>
    requires requires { typename ADetectorConstruction::ProminentDescription; }
auto DetectorMessenger<ADerived, ADetectorConstruction, AAppName>::SetNewValue(G4UIcommand* command, G4String value) -> void {
    using ProminentDescription = ADetectorConstruction::ProminentDescription;
    using Detector::Description::DescriptionIO;
    const auto annotation{[] {
        if constexpr (AAppName) {
            return fmt::format("{}: detector description", AAppName.sv());
        } else {
            return std::string{};
        }
    }()};
    if (command == fImportDescription.get()) {
        DescriptionIO::Import<ProminentDescription>(std::string_view{value});
    } else if (command == fExportDescription.get()) {
        DescriptionIO::Export<ProminentDescription>(std::string_view{value}, annotation);
    } else if (command == fEmportDescription.get()) {
        DescriptionIO::Emport<ProminentDescription>(std::string_view{value}, annotation);
    }
}

} // namespace Mustard::Geant4X::inline Interface
