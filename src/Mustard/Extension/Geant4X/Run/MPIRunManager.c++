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

#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/Extension/Geant4X/Run/MPIRunManager.h++"
#include "Mustard/Parallel/ReseedRandomEngine.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "G4Run.hh"

#include "mplr/mplr.hpp"

#include "muc/utility"

#include "fmt/core.h"

#include <exception>
#include <streambuf>

namespace Mustard::inline Extension::Geant4X::inline Run {

namespace internal {

FlipG4cout::FlipG4cout() {
    if (mplr::comm_world().rank() != 0 or
        not Env::VerboseLevelReach<'E'>()) {
        static std::streambuf* gG4coutBufExchanger{};
        gG4coutBufExchanger = G4cout.rdbuf(gG4coutBufExchanger);
    }
}

} // namespace internal

MPIRunManager::MPIRunManager() :
    internal::PreG4RunManagerInitFlipG4cout{},
    G4RunManager{},
    internal::PostG4RunManagerInitFlipG4cout{},
    fExecutor{"G4Run", "G4Event"},
    fMessengerRegister{this} {
    printModulo = -1;
    SetVerboseLevel(muc::to_underlying(Env::BasicEnv::Instance().VerboseLevel()));
}

auto MPIRunManager::BeamOn(G4int nEvent, gsl::czstring macroFile, G4int nSelect) -> void {
    Parallel::ReseedRandomEngine();
    G4RunManager::BeamOn(nEvent, macroFile, nSelect);
}

auto MPIRunManager::DoEventLoop(G4int nEvent, gsl::czstring macroFile, G4int nSelect) -> void {
    InitializeEventLoop(nEvent, macroFile, nSelect);
    // Set name for message
    if (currentRun) {
        fExecutor.ExecutionName(fmt::format("G4Run {}", currentRun->GetRunID()));
    }
    // Event loop
    fExecutor.Execute(numberOfEventToBeProcessed,
                      [this](auto eventID) {
                          ProcessOneEvent(eventID);
                          TerminateOneEvent();
                          if (runAborted) {
                              Throw<std::runtime_error>("G4Run aborted");
                          }
                      });
    // If multi-threading, TerminateEventLoop() is invoked after all threads are finished.
    // MPIRunManager::runManagerType is sequentialRM.
    if (runManagerType == sequentialRM) {
        TerminateEventLoop();
    }
}

} // namespace Mustard::inline Extension::Geant4X::inline Run
