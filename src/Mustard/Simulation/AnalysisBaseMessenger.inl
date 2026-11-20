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

namespace Mustard::Simulation {

template<typename AReceiver>
AnalysisBaseMessenger<AReceiver>::AnalysisBaseMessenger() :
    Geant4X::SingletonMessenger<AnalysisBaseMessenger<AReceiver>, AReceiver>{},
    fDirectory{},
    fFilePath{},
    fFileMode{} {

    fDirectory = std::make_unique<G4UIdirectory>("/Mustard/Analysis/");
    fDirectory->SetGuidance("Simulation analysis controller.");

    fFilePath = std::make_unique<G4UIcmdWithAString>("/Mustard/Analysis/FilePath", this);
    fFilePath->SetGuidance("Set file path.");
    fFilePath->SetParameterName("path", false);
    fFilePath->AvailableForStates(G4State_Idle);

    fFileMode = std::make_unique<G4UIcmdWithAString>("/Mustard/Analysis/FileMode", this);
    fFileMode->SetGuidance("Set mode (NEW, RECREATE, or UPDATE) for opening ROOT file(s).");
    fFileMode->SetParameterName("mode", false);
    fFileMode->AvailableForStates(G4State_Idle);
}

template<typename AReceiver>
auto AnalysisBaseMessenger<AReceiver>::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fFilePath.get()) {
        this->template Deliver<AReceiver>([&](auto&& r) {
            r.FilePath(std::string_view(value));
        });
    } else if (command == fFileMode.get()) {
        this->template Deliver<AReceiver>([&](auto&& r) {
            r.FileMode(value);
        });
    }
}

} // namespace Mustard::Simulation
