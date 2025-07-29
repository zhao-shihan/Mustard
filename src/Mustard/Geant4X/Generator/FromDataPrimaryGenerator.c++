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
#include "Mustard/Utility/PrettyLog.h++"
#include "Mustard/Utility/VectorCast.h++"

#include "TChain.h"
#include "TFile.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"

#include "G4Event.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4ThreeVector.hh"

#include "muc/array"

#include "gsl/gsl"

#include "fmt/core.h"

#include <limits>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace Mustard::Geant4X::inline Generator {

struct FromDataPrimaryGenerator::EventData {
    TTreeReader reader;
    TTreeReaderValue<double> t{reader, "t"};
    TTreeReaderValue<float> x{reader, "x"};
    TTreeReaderValue<float> y{reader, "y"};
    TTreeReaderValue<float> z{reader, "z"};
    TTreeReaderValue<std::vector<int>> pdgID{reader, "pdgID"};
    TTreeReaderValue<std::vector<float>> px{reader, "px"};
    TTreeReaderValue<std::vector<float>> py{reader, "py"};
    TTreeReaderValue<std::vector<float>> pz{reader, "pz"};
    TTreeReaderValue<float> w{reader, "w"};
};

FromDataPrimaryGenerator::FromDataPrimaryGenerator() :
    fChain{},
    fEventData{std::make_unique_for_overwrite<struct EventData>()},
    fNVertex{1},
    fCurrentRun{},
    fEndEntryForCurrentRun{},
    fFromDataPrimaryGeneratorMessengerRegister{this} {}

FromDataPrimaryGenerator::FromDataPrimaryGenerator(const std::filesystem::path& file, const std::string& data) :
    FromDataPrimaryGenerator{} {
    EventData(file, data);
}

FromDataPrimaryGenerator::~FromDataPrimaryGenerator() = default;

auto FromDataPrimaryGenerator::EventData(const std::filesystem::path& file, const std::string& data) -> void {
    fChain = std::make_unique<TChain>(data.c_str());
    fChain->Add(file.generic_string().c_str());
    fEventData->reader.SetTree(fChain.get());
    // Reset entry index reference
    fEndEntryForCurrentRun = 0;
}

auto FromDataPrimaryGenerator::GeneratePrimaryVertex(G4Event* event) -> void {
    const auto run{G4RunManager::GetRunManager()->GetCurrentRun()};
    if (fCurrentRun != std::pair{run, run->GetRunID()}) {
        fCurrentRun = {run, run->GetRunID()};
        fEndEntryForCurrentRun += run->GetNumberOfEventToBeProcessed();
    }
    // use 'last entry' as reference index looks not good but G4Run may be destructed so I have to do so
    const auto iBegin{fEndEntryForCurrentRun - run->GetNumberOfEventToBeProcessed() + event->GetEventID()};

    auto& [reader, t, x, y, z, particlePdgID, momentumX, momentumY, momentumZ, weight]{*fEventData};
    if (reader.IsInvalid()) [[unlikely]] {
        Mustard::PrintError("TTreeReader is invalid");
        return;
    }
    if (reader.GetEntries() == 0) [[unlikely]] {
        Mustard::PrintError("TTreeReader has no entry to read");
        return;
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

        const auto& [pdgID, px, py, pz]{std::tie(*particlePdgID, *momentumX, *momentumY, *momentumZ)};
        if (pdgID.size() != px.size() or pdgID.size() != py.size() or pdgID.size() != pz.size()) {
            Mustard::PrintError(fmt::format("pdgID.size() ({}), px.size() ({}), py.size() ({}), pz.size() ({}) inconsistent, skipping",
                                            pdgID.size(), px.size(), py.size(), pz.size()));
            return;
        }

        // clang-format off
        const auto primaryVertex{new G4PrimaryVertex{*x, *y, *z, *t}}; // clang-format on
        for (gsl::index i{}; i < ssize(pdgID); ++i) {
            primaryVertex->SetPrimary(new G4PrimaryParticle{pdgID[i], px[i], py[i], pz[i]});
        }
        primaryVertex->SetWeight(*weight);
        event->AddPrimaryVertex(primaryVertex);
    }
}

} // namespace Mustard::Geant4X::inline Generator
