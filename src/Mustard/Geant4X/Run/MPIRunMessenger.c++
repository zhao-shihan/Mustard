// -*- C++ -*-
//
// Copyright 2020-2025  The Mustard development team
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

#include "Mustard/Geant4X/Run/MPIRunManager.h++"
#include "Mustard/Geant4X/Run/MPIRunMessenger.h++"

#include "G4SystemOfUnits.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIdirectory.hh"

#include "muc/chrono"

namespace Mustard::Geant4X::inline Run {

MPIRunMessenger::MPIRunMessenger() :
    SingletonMessenger{},
    fDirectory{},
    fPrintProgress{},
    fPrintProgressInterval{},
    fPrintRunSummary{} {

    fDirectory = std::make_unique<G4UIdirectory>("/Mustard/Run/");
    fDirectory->SetGuidance("Specialized settings for MPIRunManager.");

    fPrintProgress = std::make_unique<G4UIcmdWithABool>("/Mustard/Run/PrintProgress", this);
    fPrintProgress->SetGuidance("Set whether to display the run progress. /run/printprogress is disabled once this is set.");
    fPrintProgress->SetParameterName("b", false);
    fPrintProgress->AvailableForStates(G4State_PreInit, G4State_Idle);

    fPrintProgressInterval = std::make_unique<G4UIcmdWithADoubleAndUnit>("/Mustard/Run/PrintProgressInterval", this);
    fPrintProgressInterval->SetGuidance("Set display time interval of run progress. /run/printprogress is disabled once this is set.");
    fPrintProgressInterval->SetParameterName("interval", false);
    fPrintProgressInterval->SetUnitCategory("Time");
    fPrintProgressInterval->SetRange("interval > 0");
    fPrintProgressInterval->AvailableForStates(G4State_PreInit, G4State_Idle);

    fPrintRunSummary = std::make_unique<G4UIcmdWithoutParameter>("/Mustard/Run/PrintRunSummary", this);
    fPrintRunSummary->SetGuidance("Print MPI run performace summary.");
    fPrintRunSummary->AvailableForStates(G4State_Idle);
}

MPIRunMessenger::~MPIRunMessenger() = default;

auto MPIRunMessenger::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fPrintProgress.get()) {
        Deliver<MPIRunManager>([&](auto&& r) {
            r.PrintProgress(fPrintProgress->GetNewBoolValue(value));
        });
    } else if (command == fPrintProgressInterval.get()) {
        Deliver<MPIRunManager>([&](auto&& r) {
            const muc::chrono::seconds<double> interval{fPrintProgressInterval->GetNewDoubleValue(value) / s};
            r.PrintProgressInterval(interval);
        });
    } else if (command == fPrintRunSummary.get()) {
        Deliver<MPIRunManager>([&](auto&& r) {
            r.PrintRunSummary();
        });
    }
}

} // namespace Mustard::Geant4X::inline Run
