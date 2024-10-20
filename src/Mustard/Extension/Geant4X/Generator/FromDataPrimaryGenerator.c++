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
    TTreeReaderValue<muc::array3f> x{reader, "x"};
    TTreeReaderValue<std::vector<int>> pdgID{reader, "pdgID"};
    TTreeReaderValue<std::vector<muc::array3f>> p{reader, "p"};
};

FromDataPrimaryGenerator::FromDataPrimaryGenerator() :
    fBeamFile{},
    fEventData{std::make_unique_for_overwrite<struct EventData>()},
    fNVertex{1} {}

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
    auto& [reader, t, x, particlePdgID, momentum]{*fEventData};
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

        const auto& [pdgID, p]{std::tie(*particlePdgID, *momentum)};
        if (pdgID.size() != p.size()) {
            throw std::runtime_error{Mustard::PrettyException(fmt::format("pdgID.size() ({}) != p.size() ({})", pdgID.size(), p.size()))};
        }

        // clang-format off
        const auto primaryVertex{new G4PrimaryVertex{Mustard::VectorCast<G4ThreeVector>(*x), *t}}; // clang-format on
        for (gsl::index i{}; i < ssize(pdgID); ++i) {
            primaryVertex->SetPrimary(new G4PrimaryParticle{pdgID[i], p[i][0], p[i][1], p[i][2]});
        }
        event->AddPrimaryVertex(primaryVertex);
    }
}

} // namespace Mustard::inline Extension::Geant4X::inline Generator
