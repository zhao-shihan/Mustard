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

#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Extension/Geant4X/Run/MPIRunManager.h++"
#include "Mustard/Utility/MPIReseedRandomEngine.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "G4ApplicationState.hh"
#include "G4Exception.hh"
#include "G4Run.hh"
#include "G4StateManager.hh"
#include "Randomize.hh"

#include "muc/math"
#include "muc/utility"

#include "fmt/chrono.h"
#include "fmt/format.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <streambuf>
#include <string>
#include <string_view>

namespace Mustard::inline Extension::Geant4X::inline Run {

namespace internal {

FlipG4cout::FlipG4cout() {
    if (Env::MPIEnv::Instance().OnCommWorldWorker() or
        not Env::VerboseLevelReach<'E'>()) {
        static std::streambuf* gG4coutBufExchanger{};
        gG4coutBufExchanger = G4cout.rdbuf(gG4coutBufExchanger);
    }
}

} // namespace internal

MPIRunManager::MPIRunManager() :
    NonMoveableBase{},
    internal::PreG4RunManagerInitFlipG4cout{},
    G4RunManager{},
    internal::PostG4RunManagerInitFlipG4cout{},
    fExecutor{},
    fMessengerRegister{this} {
    printModulo = -1;
    SetVerboseLevel(muc::to_underlying(Env::MPIEnv::Instance().VerboseLevel()));
    fExecutor.TaskName("G4Event");
}

auto MPIRunManager::BeamOn(G4int nEvent, gsl::czstring macroFile, G4int nSelect) -> void {
    MPIReseedRandomEngine();
    fakeRun = nEvent <= 0;
    if (ConfirmBeamOnCondition()) {
        numberOfEventToBeProcessed = nEvent;
        numberOfEventProcessed = 0;
        ConstructScoringWorlds();
        RunInitialization();
        DoEventLoop(nEvent, macroFile, nSelect);
        RunTermination();
    }
    fakeRun = false;
}

auto MPIRunManager::ConfirmBeamOnCondition() -> G4bool {
    const auto& mpiEnv{Env::MPIEnv::Instance()};

    if (const auto currentState{G4StateManager::GetStateManager()->GetCurrentState()};
        currentState != G4State_PreInit and currentState != G4State_Idle) {
        if (mpiEnv.OnCommWorldMaster()) {
            G4cerr << "Illegal application state - BeamOn ignored." << G4endl;
        }
        return false;
    }

    if (not initializedAtLeastOnce) {
        if (mpiEnv.OnCommWorldMaster()) {
            G4cerr << "Geant4 kernel should be initialized before the first BeamOn - BeamOn ignored." << G4endl;
        }
        return false;
    }

    if (not geometryInitialized or not physicsInitialized) {
        if (verboseLevel > 0 and mpiEnv.OnCommWorldMaster()) {
            G4cout << "Start re-initialization because \n";
            if (not geometryInitialized) { G4cout << "  Geometry\n"; }
            if (not physicsInitialized) { G4cout << "  Physics processes\n"; }
            G4cout << "has been modified since last Run." << G4endl;
        }
        Initialize();
    }
    return true;
}

auto MPIRunManager::DoEventLoop(G4int nEvent, const char* macroFile, G4int nSelect) -> void {
    InitializeEventLoop(nEvent, macroFile, nSelect);
    // Set name for message
    if (currentRun) { fExecutor.ExecutionName(fmt::format("G4Run {}", currentRun->GetRunID())); }
    // Event loop
    fExecutor.Execute(numberOfEventToBeProcessed,
                      [this](auto eventID) {
                          ProcessOneEvent(eventID);
                          TerminateOneEvent();
                          if (runAborted) { throw std::runtime_error{PrettyException("G4Run aborted")}; }
                      });
    // If multi-threading, TerminateEventLoop() is invoked after all threads are finished.
    // MPIRunManager::runManagerType is sequentialRM.
    if (runManagerType == sequentialRM) { TerminateEventLoop(); }
}

} // namespace Mustard::inline Extension::Geant4X::inline Run
