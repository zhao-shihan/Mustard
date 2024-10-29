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

#pragma once

#include "Mustard/Extension/Geant4X/Generator/FromDataPrimaryGeneratorMessenger.h++"

#include "G4VPrimaryGenerator.hh"

#include <filesystem>
#include <memory>
#include <string>

class TFile;

namespace Mustard::inline Extension::Geant4X::inline Generator {

class FromDataPrimaryGenerator : public G4VPrimaryGenerator {
public:
    FromDataPrimaryGenerator();
    FromDataPrimaryGenerator(const std::filesystem::path& file, const std::string& data);
    ~FromDataPrimaryGenerator();

    auto EventData(const std::filesystem::path& file, const std::string& data) -> void;

    auto NVertex() const -> auto { return fNVertex; }
    auto NVertex(int n) -> void { fNVertex = std::max(0, n); }

    auto GeneratePrimaryVertex(G4Event* event) -> void override;

protected:
    struct EventData;

protected:
    std::unique_ptr<TFile> fBeamFile;
    std::unique_ptr<struct EventData> fEventData;

    int fNVertex; // Do not change this within a G4 run

private:
    FromDataPrimaryGeneratorMessenger::Register<FromDataPrimaryGenerator> fFromDataPrimaryGeneratorMessengerRegister;
};

} // namespace Mustard::inline Extension::Geant4X::inline Generator
