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

#include "Mustard/Geant4X/Generator/DataReaderPrimaryGenerator.h++"
#include "Mustard/Geant4X/Management/Exception.h++"
#include "Mustard/IO/PrettyLog.h++"

#include "G4Event.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4StateManager.hh"

#include "gsl/gsl"

#include <algorithm>

namespace Mustard::Geant4X::inline Generator {

DataReaderPrimaryGenerator::DataReaderPrimaryGenerator() :
    fReader{},
    fNVertex{},
    fCurrentRun{},
    fAccumulatedEventToBeProcessed{},
    fDataReaderPrimaryGeneratorMessengerRegister{this} {}

DataReaderPrimaryGenerator::DataReaderPrimaryGenerator(ROOT::RDF::RNode rdf, int nVertex) :
    DataReaderPrimaryGenerator{} {
    EventData(rdf);
    NVertex(nVertex);
}

DataReaderPrimaryGenerator::DataReaderPrimaryGenerator(const std::filesystem::path& file, const std::string& data, int nVertex) :
    DataReaderPrimaryGenerator{} {
    EventData(file, data);
    NVertex(nVertex);
}

DataReaderPrimaryGenerator::~DataReaderPrimaryGenerator() = default;

auto DataReaderPrimaryGenerator::EventData(ROOT::RDF::RNode rdf) -> void {
    if (not CheckG4State()) {
        Exception<JustWarning>("Invalid G4 application state. Not setting it.");
        return;
    }
    fReader.emplace(rdf);
}

auto DataReaderPrimaryGenerator::EventData(const std::filesystem::path& file, const std::string& data) -> void {
    EventData(ROOT::RDataFrame{data, file.generic_string()});
}

auto DataReaderPrimaryGenerator::NVertex(int n) -> void {
    if (not CheckG4State()) {
        Exception<JustWarning>("Invalid G4 application state. Not setting it.");
        return;
    }
    if (n < 0) [[unlikely]] {
        Exception<JustWarning>("Number of vertices per event is negative. Setting to 0.");
        fNVertex = 0;
        return;
    }
    fNVertex = n;
}

auto DataReaderPrimaryGenerator::GeneratePrimaryVertex(G4Event* event) -> void {
    const auto run{G4RunManager::GetRunManager()->GetCurrentRun()};
    if (fCurrentRun != std::pair{run, run->GetRunID()}) {
        fCurrentRun = {run, run->GetRunID()};
        fAccumulatedEventToBeProcessed += run->GetNumberOfEventToBeProcessed();
    }

    if (fNVertex == 0) {
        return;
    }
    if (not fReader) [[unlikely]] {
        Exception<RunMustBeAborted>("No event data reader configured.");
        return;
    }
    const auto nEntry{fReader->NEntry()};
    if (nEntry == 0) [[unlikely]] {
        Exception<RunMustBeAborted>("No entries in data source.");
        return;
    }
    if (nEntry < fNVertex) [[unlikely]] {
        Exception<RunMustBeAborted>(fmt::format(
            "Not enough entries in data source: expected at least {} (number of vertices) entries, got {} entries", fNVertex, nEntry));
        return;
    }

    const auto numberOfEventRemaining{run->GetNumberOfEventToBeProcessed() - event->GetEventID()};
    const auto iFirst{((fAccumulatedEventToBeProcessed - numberOfEventRemaining) * fNVertex) % nEntry};
    const auto iLast{iFirst + fNVertex};

    int nGenerated{};
    auto generateChunk{[&](const auto& chunk) {
        if (chunk.empty()) [[unlikely]] {
            Exception<FatalException>("Failed to read vertex data.");
            return;
        }
        for (auto&& vertex : chunk) {
            const auto primaryVertex(new G4PrimaryVertex{
                F<"x">(*vertex),
                F<"y">(*vertex),
                F<"z">(*vertex),
                F<"t0">(*vertex)});
            const auto nParticle{ssize(*F<"pdgID">(*vertex))};
            Ensures(ssize(*F<"px">(*vertex)) == nParticle);
            Ensures(ssize(*F<"py">(*vertex)) == nParticle);
            Ensures(ssize(*F<"pz">(*vertex)) == nParticle);
            Ensures(ssize(*F<"E">(*vertex)) == nParticle);
            for (gsl::index i{}; i < nParticle; ++i) {
                primaryVertex->SetPrimary(new G4PrimaryParticle{
                    F<"pdgID">(*vertex)[i],
                    F<"px">(*vertex)[i],
                    F<"py">(*vertex)[i],
                    F<"pz">(*vertex)[i],
                    F<"E">(*vertex)[i]});
            }
            primaryVertex->SetWeight(F<"w">(*vertex));
            event->AddPrimaryVertex(primaryVertex);
        }
        nGenerated += chunk.size();
    }};
    auto resetReader{[this] {
        Exception<JustWarning>("Data source exhausted. Resetting reader.");
        fReader->Reset();
    }};
    if (fReader->Exhausted()) {
        resetReader();
    }
    if (iLast <= nEntry) {
        generateChunk(fReader->Read(iFirst, iLast));
    } else {
        generateChunk(fReader->Read(iFirst, nEntry));
        resetReader();
        generateChunk(fReader->Read(0, iLast - nEntry));
    }
    Ensures(nGenerated == fNVertex);
}

auto DataReaderPrimaryGenerator::CheckG4State() -> bool {
    switch (G4StateManager::GetStateManager()->GetCurrentState()) {
    case G4State_PreInit:
    case G4State_Init:
    case G4State_Idle:
        return true;
    case G4State_GeomClosed:
    case G4State_EventProc:
    case G4State_Quit:
    case G4State_Abort:
    default:
        return false;
    }
}

} // namespace Mustard::Geant4X::inline Generator
