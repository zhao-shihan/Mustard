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
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Utility/VectorCast.h++"

#include "TChain.h"
#include "TFile.h"

#include "G4Event.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4StateManager.hh"
#include "G4ThreeVector.hh"

#include "muc/array"

#include "gsl/gsl"

#include "fmt/core.h"

#include <limits>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace Mustard::Geant4X::inline Generator {

DataReaderPrimaryGenerator::DataReaderPrimaryGenerator() :
    fChain{},
    fEventData{},
    fNVertex{},
    fCurrentRun{},
    fEndEntryForCurrentRun{},
    fDataReaderPrimaryGeneratorMessengerRegister{this} {}

DataReaderPrimaryGenerator::DataReaderPrimaryGenerator(const std::filesystem::path& file, const std::string& data, int nVertex) :
    DataReaderPrimaryGenerator{} {
    EventData(file, data);
    NVertex(nVertex);
}

DataReaderPrimaryGenerator::~DataReaderPrimaryGenerator() = default;

auto DataReaderPrimaryGenerator::EventData(const std::filesystem::path& file, const std::string& data) -> void {
    CheckG4Status();
    fChain = std::make_unique<TChain>(data.c_str());
    fChain->Add(file.generic_string().c_str());
    fEventData.reader.SetTree(fChain.get());
    // Reset entry index reference
    fEndEntryForCurrentRun = 0;
}

auto DataReaderPrimaryGenerator::NVertex(int n) -> void {
    CheckG4Status();
    fNVertex = std::max(0, n);
}

auto DataReaderPrimaryGenerator::GeneratePrimaryVertex(G4Event* event) -> void {
    const auto run{G4RunManager::GetRunManager()->GetCurrentRun()};
    if (fCurrentRun != std::pair{run, run->GetRunID()}) {
        fCurrentRun = {run, run->GetRunID()};
        fEndEntryForCurrentRun += run->GetNumberOfEventToBeProcessed();
    }
    // use 'last entry' as reference index looks not perfect but G4Run may be destructed so I have to do so
    const auto iBegin{fEndEntryForCurrentRun - run->GetNumberOfEventToBeProcessed() + event->GetEventID()};

    auto& [reader, weight, t, x, y, z, thePDGID, theE, thePX, thePY, thePZ]{fEventData};
    if (reader.IsInvalid()) [[unlikely]] {
        Mustard::PrintError("TTreeReader is invalid");
        return;
    }
    if (reader.GetEntries() == 0) [[unlikely]] {
        Mustard::PrintError("TTreeReader has no entry to read");
        return;
    }
    if (reader.GetEntries() % fNVertex != 0) [[unlikely]] {
        Mustard::PrintWarning("The number of entries cannot be exactly divided by the number of vertices");
    }
    reader.SetEntry((iBegin * fNVertex) % reader.GetEntries());

    for (int iVertex{}; iVertex < fNVertex; ++iVertex) {
        if (not reader.Next()) {
            reader.Restart();
            if (not reader.Next()) [[unlikely]] {
                Mustard::PrintError("Failed to read event data");
                return;
            }
        }

        const auto& [pdgID, e, pX, pY, pZ]{std::tie(*thePDGID, *theE, *thePX, *thePY, *thePZ)};
        const auto nParticle{ssize(pdgID)};
        if (ssize(e) != nParticle or
            ssize(pX) != nParticle or
            ssize(pY) != nParticle or
            ssize(pZ) != nParticle) {
            Mustard::PrintError(fmt::format("pdgID.size() ({}), E.size() ({}), px.size() ({}), py.size() ({}), pz.size() ({}) inconsistent, skipping",
                                            pdgID.size(), e.size(), pX.size(), pY.size(), pZ.size()));
            return;
        }

        // clang-format off
        const auto primaryVertex{new G4PrimaryVertex{*x, *y, *z, *t}}; // clang-format on
        for (gsl::index i{}; i < nParticle; ++i) {
            primaryVertex->SetPrimary(new G4PrimaryParticle{pdgID[i], pX[i], pY[i], pZ[i], e[i]});
        }
        primaryVertex->SetWeight(*weight);
        event->AddPrimaryVertex(primaryVertex);
    }
}

auto DataReaderPrimaryGenerator::CheckG4Status() -> void {
    switch (G4StateManager::GetStateManager()->GetCurrentState()) {
    case G4State_PreInit:
    case G4State_Init:
    case G4State_Idle:
        return;
    case G4State_GeomClosed:
    case G4State_EventProc:
    case G4State_Quit:
    case G4State_Abort:
    default:
        Mustard::Throw<std::runtime_error>("Invalid G4 application state");
    }
}

} // namespace Mustard::Geant4X::inline Generator
