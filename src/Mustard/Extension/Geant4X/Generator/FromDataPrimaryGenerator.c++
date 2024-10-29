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
#include "Mustard/Utility/PrettyLog.h++"
#include "Mustard/Utility/VectorCast.h++"

#include "TFile.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"

#include "G4Event.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"
#include "G4ThreeVector.hh"

#include "muc/array"

#include "gsl/gsl"

#include "fmt/core.h"

#include <stdexcept>
#include <tuple>
#include <vector>

namespace Mustard::inline Extension::Geant4X::inline Generator {

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
};

FromDataPrimaryGenerator::FromDataPrimaryGenerator() :
    fBeamFile{},
    fEventData{std::make_unique_for_overwrite<struct EventData>()},
    fNVertex{1},
    fFromDataPrimaryGeneratorMessengerRegister{this} {}

FromDataPrimaryGenerator::FromDataPrimaryGenerator(const std::filesystem::path& file, const std::string& data) :
    FromDataPrimaryGenerator{} {
    EventData(file, data);
}

FromDataPrimaryGenerator::~FromDataPrimaryGenerator() = default;

auto FromDataPrimaryGenerator::EventData(const std::filesystem::path& file, const std::string& data) -> void {
    fBeamFile = std::unique_ptr<TFile>{TFile::Open(file.generic_string().c_str())};
    if (fBeamFile == nullptr) {
        throw std::runtime_error{Mustard::PrettyException(fmt::format("Cannot open '{}'", file.c_str()))};
    }
    fEventData->reader.SetTree(data.c_str(), fBeamFile.get());
    if (fEventData->reader.IsInvalid()) {
        throw std::runtime_error{Mustard::PrettyException(fmt::format("Cannot read '{}' from '{}'", data, file.c_str()))};
    }
}

auto FromDataPrimaryGenerator::GeneratePrimaryVertex(G4Event* event) -> void {
    auto& [reader, t, x, y, z, particlePdgID, momentumX, momentumY, momentumZ]{*fEventData};
    if (reader.IsInvalid()) {
        throw std::runtime_error{Mustard::PrettyException(fmt::format("TTreeReader is invalid", reader.GetTree()->GetName()))};
    }
    if (reader.GetEntries() == 0) {
        throw std::runtime_error{Mustard::PrettyException(fmt::format("'{}' has no entry", reader.GetTree()->GetName()))};
    }
    reader.SetEntry((event->GetEventID() * fNVertex) % reader.GetEntries());

    for (int iVertex{}; iVertex < fNVertex; ++iVertex) {
        if (not reader.Next()) {
            reader.Restart();
            reader.Next();
        }

        const auto& [pdgID, px, py, pz]{std::tie(*particlePdgID, *momentumX, *momentumY, *momentumZ)};
        if (pdgID.size() != px.size() or pdgID.size() != py.size() or pdgID.size() != pz.size()) {
            throw std::runtime_error{Mustard::PrettyException(fmt::format("pdgID.size() ({}), px.size() ({}), py.size() ({}), pz.size() ({}) inconsistent",
                                                                          pdgID.size(), px.size(), py.size(), pz.size()))};
        }

        // clang-format off
        const auto primaryVertex{new G4PrimaryVertex{*x, *y, *z, *t}}; // clang-format on
        for (gsl::index i{}; i < ssize(pdgID); ++i) {
            primaryVertex->SetPrimary(new G4PrimaryParticle{pdgID[i], px[i], py[i], pz[i]});
        }
        event->AddPrimaryVertex(primaryVertex);
    }
}

} // namespace Mustard::inline Extension::Geant4X::inline Generator
