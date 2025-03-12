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

#include "Mustard/Extension/Geant4X/Interface/SingletonMessenger.h++"

#include <memory>

class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithADouble;
class G4UIcmdWithAString;
class G4UIcmdWith3VectorAndUnit;
class G4UIdirectory;

namespace Mustard::inline Extension::Geant4X::inline Generator {

class EcoMugCosmicRayMuon;

class EcoMugCosmicRayMuonMessenger final : public Geant4X::SingletonMessenger<EcoMugCosmicRayMuonMessenger,
                                                                              EcoMugCosmicRayMuon> {
    friend Env::Memory::SingletonInstantiator;

private:
    EcoMugCosmicRayMuonMessenger();
    ~EcoMugCosmicRayMuonMessenger();

public:
    auto SetNewValue(G4UIcommand* command, G4String value) -> void override;

private:
    std::unique_ptr<G4UIdirectory> fEcoMugDirectory;
    
    std::unique_ptr<G4UIdirectory> fPosDirectory;
    std::unique_ptr<G4UIcmdWith3VectorAndUnit> fSkyCenterPos;
    std::unique_ptr<G4UIcmdWith3VectorAndUnit> fHSphereCenterPos;
    std::unique_ptr<G4UIcmdWith3VectorAndUnit> fCylinderCenterPos;
    
    std::unique_ptr<G4UIdirectory> fSizeDirectory;
    std::unique_ptr<G4UIcmdWith3VectorAndUnit> fSkySizeXY;
    std::unique_ptr<G4UIdirectory> fSizeHSphereDirectory;
    std::unique_ptr<G4UIcmdWithADoubleAndUnit> fHSphereRadius;
    std::unique_ptr<G4UIdirectory> fSizeCylinderDirectory;
    std::unique_ptr<G4UIcmdWithADoubleAndUnit> fCylinderRadius;
    std::unique_ptr<G4UIcmdWithADoubleAndUnit> fCylinderHeight;

    std::unique_ptr<G4UIdirectory> fEnergyDirectory;
    std::unique_ptr<G4UIcmdWithADoubleAndUnit> fMaxMomentum;
    std::unique_ptr<G4UIcmdWithADoubleAndUnit> fMinMomentum;

    std::unique_ptr<G4UIdirectory> fAngleDirectory;
    std::unique_ptr<G4UIcmdWithADoubleAndUnit> fMaxTheta;
};

} // namespace Mustard::inline Extension::Geant4X::inline Generator
