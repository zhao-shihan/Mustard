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

#pragma once

#include "Mustard/Geant4X/Generator/DataReaderPrimaryGeneratorMessenger.h++"

#include "TTreeReader.h"
#include "TTreeReaderValue.h"

#include "G4VPrimaryGenerator.hh"

#include <filesystem>
#include <memory>
#include <string>
#include <utility>

class G4Run;
class TChain;

namespace Mustard::Geant4X::inline Generator {

/// @brief Geant4 primary generator reading event data from ROOT files
///
/// Generates primary vertices and particles from pregenerated event data stored
/// in ROOT TTrees.
///
/// @note Data format specifications:
///   - TTree with branches:
///        t (double):                Vertex time [ns]
///        x,y,z (float):             Vertex position [mm]
///        pdgID (vector<int>):       Particle PDG codes
///        px,py,pz (vector<float>):  Particle momentum [MeV]
///        w (float):                 Event weight
///
class DataReaderPrimaryGenerator : public G4VPrimaryGenerator {
public:
    /// @brief Default constructor (requires later EventData() and NVertex() call)
    DataReaderPrimaryGenerator();
    /// @brief Construct with data file and tree name
    /// @param file ROOT file path containing event data
    /// @param data TTree name within file
    /// @param nVertex Number of vertices per Geant4 event
    DataReaderPrimaryGenerator(const std::filesystem::path& file, const std::string& data, int nVertex = 1);

    virtual ~DataReaderPrimaryGenerator();

    /// @brief Load new event data source
    /// @param file ROOT file path
    /// @param data TTree name
    /// @throws `std::runtime_error` if called in invalid G4 state
    auto EventData(const std::filesystem::path& file, const std::string& data) -> void;

    /// @brief Get current vertices per event
    auto NVertex() const -> auto { return fNVertex; }
    /// @brief Set vertices per event
    /// @param n Number of vertices (≥0)
    /// @throws `std::runtime_error` if called in invalid G4 state
    auto NVertex(int n) -> void;

    /// @brief Generate primary vertices for current event
    /// @param event Geant4 event being processed
    virtual auto GeneratePrimaryVertex(G4Event* event) -> void override;

protected:
    /// @brief Verify Geant4 state allows configuration changes
    /// @throws std::runtime_error if in active run state
    static auto CheckG4Status() -> void;

protected:
    /// @brief Event data reader structure
    /// @see `Mustard::Data::GeneratedEvent` for a data model
    struct EventData {
        TTreeReader reader;                                        ///< ROOT tree reader
        TTreeReaderValue<float> w{reader, "w"};                    ///< Vertex weight
        TTreeReaderValue<double> t{reader, "t"};                   ///< Vertex time
        TTreeReaderValue<float> x{reader, "x"};                    ///< Vertex X position
        TTreeReaderValue<float> y{reader, "y"};                    ///< Vertex Y position
        TTreeReaderValue<float> z{reader, "z"};                    ///< Vertex Z position
        TTreeReaderValue<std::vector<int>> pdgID{reader, "pdgID"}; ///< Particle PDG IDs
        TTreeReaderValue<std::vector<float>> px{reader, "px"};     ///< Momentum X components
        TTreeReaderValue<std::vector<float>> py{reader, "py"};     ///< Momentum Y components
        TTreeReaderValue<std::vector<float>> pz{reader, "pz"};     ///< Momentum Z components
    };

protected:
    std::unique_ptr<TChain> fChain;           ///< ROOT TChain holding event data
    struct EventData fEventData;              ///< Reader instance for current tree
                                              //
    int fNVertex;                             ///< Vertices per Geant4 event
                                              //
    std::pair<const G4Run*, int> fCurrentRun; ///< Current run
    long long fEndEntryForCurrentRun;         ///< Entry index management

private:
    DataReaderPrimaryGeneratorMessenger::Register<DataReaderPrimaryGenerator> fDataReaderPrimaryGeneratorMessengerRegister;
};

} // namespace Mustard::Geant4X::inline Generator
