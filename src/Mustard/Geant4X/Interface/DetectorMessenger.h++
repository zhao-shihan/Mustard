// -*- C++ -*-
//
// Copyright (C) 2020-2025  Mustard developers
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

#include "Mustard/Detector/Description/DescriptionIO.h++"
#include "Mustard/Geant4X/Interface/SingletonMessenger.h++"

#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIdirectory.hh"
#include "G4VUserDetectorConstruction.hh"

#include "muc/ceta_string"

#include "fmt/format.h"

#include <concepts>
#include <memory>
#include <string_view>

namespace Mustard::Geant4X::inline Interface {

template<typename ADerived, std::derived_from<G4VUserDetectorConstruction> ADetectorConstruction, muc::ceta_string AAppName = nullptr>
    requires requires { typename ADetectorConstruction::ProminentDescription; }
class DetectorMessenger : public Geant4X::SingletonMessenger<ADerived> {
protected:
    DetectorMessenger();

public:
    auto SetNewValue(G4UIcommand* command, G4String value) -> void override;

private:
    std::unique_ptr<G4UIdirectory> fDirectory;
    std::unique_ptr<G4UIcmdWithAString> fImportDescription;
    std::unique_ptr<G4UIcmdWithAString> fExportDescription;
    std::unique_ptr<G4UIcmdWithAString> fEmportDescription;
};

} // namespace Mustard::Geant4X::inline Interface

#include "Mustard/Geant4X/Interface/DetectorMessenger.inl"
