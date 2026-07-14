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

#pragma once

#include "Mustard/Geant4X/Interface/SingletonMessenger.h++"

#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIdirectory.hh"
#include "G4UImessenger.hh"

#include <memory>

namespace Mustard::Geant4X {

inline namespace Process {
class MuoniumFormation;
class MuoniumThermalScattering;
} // namespace Process

inline namespace Physics {

class MuoniumPhysicsMessenger final : public SingletonMessenger<MuoniumPhysicsMessenger,
                                                                MuoniumFormation,
                                                                MuoniumThermalScattering> {
    friend Env::SingletonFactory;

private:
    MuoniumPhysicsMessenger();

public:
    auto SetNewValue(G4UIcommand* command, G4String value) -> void override;

private:
    std::unique_ptr<G4UIdirectory> fMuoniumPhysicsDirectory;

    std::unique_ptr<G4UIdirectory> fFormationProcessDirectory;
    std::unique_ptr<G4UIcmdWithADouble> fConversionProbability;

    std::unique_ptr<G4UIdirectory> fTransportProcessDirectory;
    std::unique_ptr<G4UIcmdWithABool> fManipulateAllSteps;
};

} // namespace Physics

} // namespace Mustard::Geant4X
