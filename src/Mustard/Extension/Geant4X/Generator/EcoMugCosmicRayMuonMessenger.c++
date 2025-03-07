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

#include "Mustard/Extension/Geant4X/Generator/EcoMugCosmicRayMuon.h++"
#include "Mustard/Extension/Geant4X/Generator/EcoMugCosmicRayMuonMessenger.h++"
#include "Mustard/Utility/LiteralUnit.h++"

#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAString.hh"

namespace Mustard::inline Extension::Geant4X::inline Generator {
using namespace LiteralUnit;
EcoMugCosmicRayMuonMessenger::EcoMugCosmicRayMuonMessenger() :
    SingletonMessenger{},
    fEcoMugDir{},
    fGeneratorShape{},
    fPosDir{},
    fSkyCenterPos{},
    fCylinderCenterPos{},
    fHSphereCenterPos{},
    fSizeDir{},
    fSkySizeXY{},
    fSizeHSphereDir{},
    fHSphereRadius{},
    fSizeCylinderDir{},
    fCylinderRadius{},
    fCylinderHeight{},
    fEnergyDir{},
    fMaxMomentum{},
    fMinMomentum{},
    fAngleDir{},
    fMaxTheta{} {

    fEcoMugDir = std::make_unique<G4UIdirectory>("/EcoMug/");
    fEcoMugDir->SetGuidance("EcoMug cosmic ray generator extension.");

    // generator shape configuration
    fGeneratorShape = std::make_unique<G4UIcmdWithAString>("EcoMug/gen_shape", this);
    fGeneratorShape->SetGuidance("Set the shape type of EcoMug generator.\
                                \nEither cylinder, h_sphere(default) or sky.");
    fGeneratorShape->SetParameterName("Str", false);
   fGeneratorShape->SetCandidates("cylinder h_sphere sky");
    fGeneratorShape->AvailableForStates(G4State_Idle);

    // position
    fPosDir = std::make_unique<G4UIdirectory>("EcoMug/position/");
    fPosDir->SetGuidance("generator posiotion directory");

    fSkyCenterPos = std::make_unique<G4UIcmdWith3VectorAndUnit>("EcoMug/position/sky", this);
    fSkyCenterPos->SetGuidance("Set sky center position.");
    fSkyCenterPos->SetParameterName("X", "Y", "Z", false, false);
    fSkyCenterPos->SetUnitCategory("Length");
    fSkyCenterPos->AvailableForStates(G4State_Idle);

    fCylinderCenterPos = std::make_unique<G4UIcmdWith3VectorAndUnit>("EcoMug/position/cylinder", this);
    fCylinderCenterPos->SetGuidance("Set cylinder center position.");
    fCylinderCenterPos->SetParameterName("X", "Y", "Z", false, false);
    fCylinderCenterPos->SetUnitCategory("Length");
    fCylinderCenterPos->AvailableForStates(G4State_Idle);

    fHSphereCenterPos = std::make_unique<G4UIcmdWith3VectorAndUnit>("EcoMug/position/h_sphere", this);
    fHSphereCenterPos->SetGuidance("Set h_sphere center position.");
    fHSphereCenterPos->SetParameterName("X", "Y", "Z", false, false);
    fHSphereCenterPos->SetUnitCategory("Length");
    fHSphereCenterPos->AvailableForStates(G4State_Idle);

    // size
    fSizeDir = std::make_unique<G4UIdirectory>("EcoMug/size/");
    fSizeDir->SetGuidance("generator size directory");

    fSkySizeXY = std::make_unique<G4UIcmdWith3VectorAndUnit>("EcoMug/size/skyXY", this);
    fSkySizeXY->SetGuidance("Set the XY size of sky if generation shape is sky.\
                            \nPlease enter a 3vector x y z. Only x and y works.\
                            \nNote that: Native(xyz)->Beam(zxy)");
    fSkySizeXY->SetParameterName("X", "Y", "Z", false);
    fSkySizeXY->SetUnitCategory("Length");
    fSkySizeXY->SetRange("X >= 0 || Y >=0 || Z >= 0");
    fSkySizeXY->AvailableForStates(G4State_Idle);

    fSizeHSphereDir = std::make_unique<G4UIdirectory>("EcoMug/size/h_sphere/");
    fSizeHSphereDir->SetGuidance("h_sphere generator size directory");

    fHSphereRadius = std::make_unique<G4UIcmdWithADoubleAndUnit>("EcoMug/size/h_sphere/radius", this);
    fHSphereRadius->SetGuidance("Set the generation h_sphere radius if used.");
    fHSphereRadius->SetParameterName("R", false);
    fHSphereRadius->SetUnitCategory("Length");
    fHSphereRadius->SetRange("R >= 0");
    fHSphereRadius->AvailableForStates(G4State_Idle);

    fSizeCylinderDir = std::make_unique<G4UIdirectory>("EcoMug/size/cylinder/");
    fSizeCylinderDir->SetGuidance("cylinder generator size directory");

    fCylinderRadius = std::make_unique<G4UIcmdWithADoubleAndUnit>("EcoMug/size/cylinder/radius", this);
    fCylinderRadius->SetGuidance("Set the generation cylinder radius if used.");
    fCylinderRadius->SetParameterName("R", false);
    fCylinderRadius->SetUnitCategory("Length");
    fCylinderRadius->SetRange("R >= 0");
    fCylinderRadius->AvailableForStates(G4State_Idle);

    fCylinderHeight = std::make_unique<G4UIcmdWithADoubleAndUnit>("EcoMug/size/cylinder/height", this);
    fCylinderHeight->SetGuidance("Set the generation cylinder height if used.");
    fCylinderHeight->SetParameterName("H", false);
    fCylinderHeight->SetUnitCategory("Length");
    fCylinderHeight->SetRange("H >= 0");
    fCylinderHeight->AvailableForStates(G4State_Idle);
    // energy
    fEnergyDir = std::make_unique<G4UIdirectory>("EcoMug/engergy/");
    fEnergyDir->SetGuidance("muon energy directory");

    fMaxMomentum = std::make_unique<G4UIcmdWithADoubleAndUnit>("EcoMug/energy/max_momentum", this);
    fMaxMomentum->SetGuidance("Set the max momentum of a muon.");
    fMaxMomentum->SetParameterName("EkMax", false);
    fMaxMomentum->SetUnitCategory("Energy");
    fMaxMomentum->SetRange("EkMax >= 0");
    fMaxMomentum->AvailableForStates(G4State_Idle);

    fMinMomentum = std::make_unique<G4UIcmdWithADoubleAndUnit>("EcoMug/energy/min_momentum", this);
    fMinMomentum->SetGuidance("Set the minimun momentum of a muon.");
    fMinMomentum->SetParameterName("EkMin", false);
    fMinMomentum->SetUnitCategory("Energy");
    fMinMomentum->SetRange("EkMin >= 0");
    fMinMomentum->AvailableForStates(G4State_Idle);
    // angle
    fAngleDir = std::make_unique<G4UIdirectory>("EcoMug/angle/");
    fAngleDir->SetGuidance("muon angle directory");

    fMaxTheta = std::make_unique<G4UIcmdWithADoubleAndUnit>("EcoMug/angle/max_theta", this);
    fMaxTheta->SetGuidance("Set the max theta angle of a muon.");
    fMaxTheta->SetParameterName("theta", false);
    fMaxTheta->SetUnitCategory("Angle");
    fMaxTheta->SetRange("theta >= 0");
    fMaxTheta->AvailableForStates(G4State_Idle);
}

EcoMugCosmicRayMuonMessenger::~EcoMugCosmicRayMuonMessenger() = default;

auto EcoMugCosmicRayMuonMessenger::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fGeneratorShape.get()) {
        Deliver<EcoMugCosmicRayMuon>([&](auto&& r) {
            if (value == "cylinder") {
                r.UseCylinder();
            } else if (value == "sky") {
                r.UseSky();
            } else if (value == "h_sphere") {
                r.UseHSphere();
            }
        });
    } else if (command == fSkyCenterPos.get()) {
        Deliver<EcoMugCosmicRayMuon>([&](auto&& r) {
            r.SkyCenterPosition(fSkyCenterPos->GetNew3VectorValue(value));
        });
    } else if (command == fCylinderCenterPos.get()) {
        Deliver<EcoMugCosmicRayMuon>([&](auto&& r) {
            r.CylinderCenterPosition(fCylinderCenterPos->GetNew3VectorValue(value));
        });
    } else if (command == fHSphereCenterPos.get()) {
        Deliver<EcoMugCosmicRayMuon>([&](auto&& r) {
            r.HSphereCenterPosition(fHSphereCenterPos->GetNew3VectorValue(value));
        });
    } else if (command == fSkySizeXY.get()) {
        Deliver<EcoMugCosmicRayMuon>([&](auto&& r) {
            auto XYZ{fSkySizeXY->GetNew3VectorValue(value)};
            r.SkySize(XYZ.getX(), XYZ.getY());
        });
    } else if (command == fCylinderRadius.get()) {
        Deliver<EcoMugCosmicRayMuon>([&](auto&& r) {
            r.CylinderRadius(fCylinderRadius->GetNewDoubleValue(value));
        });
    } else if (command == fCylinderHeight.get()) {
        Deliver<EcoMugCosmicRayMuon>([&](auto&& r) {
            r.CylinderHeight(fCylinderHeight->GetNewDoubleValue(value));
        });
    } else if (command == fHSphereRadius.get()) {
        Deliver<EcoMugCosmicRayMuon>([&](auto&& r) {
            r.HSphereRadius(fHSphereRadius->GetNewDoubleValue(value));
        });
    } else if (command == fMaxMomentum.get()) {
        Deliver<EcoMugCosmicRayMuon>([&](auto&& r) {
            r.MaxMomentum(fMaxMomentum->GetNewDoubleValue(value));
        });
    } else if (command == fMinMomentum.get()) {
        Deliver<EcoMugCosmicRayMuon>([&](auto&& r) {
            r.MinMomentum(fMinMomentum->GetNewDoubleValue(value));
        });
    } else if (command == fMaxTheta.get()) {
        Deliver<EcoMugCosmicRayMuon>([&](auto&& r) {
            r.MaxTheta(fMaxTheta->GetNewDoubleValue(value));
        });
    }
}

} // namespace Mustard::inline Extension::Geant4X::inline Generator
