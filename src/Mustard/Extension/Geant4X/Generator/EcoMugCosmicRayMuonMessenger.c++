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
    fEcoMugDirectory{},
    fPosDirectory{},
    fSkyCenterPos{},
    fHSphereCenterPos{},
    fCylinderCenterPos{},
    fSizeDirectory{},
    fSkySizeXY{},
    fSizeHSphereDirectory{},
    fHSphereRadius{},
    fSizeCylinderDirectory{},
    fCylinderRadius{},
    fCylinderHeight{},
    fEnergyDirectory{},
    fMaxMomentum{},
    fMinMomentum{},
    fAngleDirectory{},
    fMaxTheta{}{

    fEcoMugDirectory = std::make_unique<G4UIdirectory>("/Mustard/EcoMug/");
    fEcoMugDirectory->SetGuidance("EcoMug cosmic ray generator extension.");

    // position and genetator shape
    fPosDirectory = std::make_unique<G4UIdirectory>("/Mustard/EcoMug/Position/");
    fPosDirectory->SetGuidance("generator posiotion(with shape information) directory\
                                \nAvailable shape: cylinder, hsphere, sky.");

    fSkyCenterPos = std::make_unique<G4UIcmdWith3VectorAndUnit>("/Mustard/EcoMug/Position/Sky", this);
    fSkyCenterPos->SetGuidance("Set sky center position.");
    fSkyCenterPos->SetParameterName("X", "Y", "Z", false, true);
    fSkyCenterPos->SetUnitCategory("Length");
    fSkyCenterPos->AvailableForStates(G4State_Idle);

    fHSphereCenterPos = std::make_unique<G4UIcmdWith3VectorAndUnit>("/Mustard/EcoMug/Position/Hsphere", this);
    fHSphereCenterPos->SetGuidance("Set hsphere center position.");
    fHSphereCenterPos->SetParameterName("X", "Y", "Z", false, true);
    fHSphereCenterPos->SetUnitCategory("Length");
    fHSphereCenterPos->AvailableForStates(G4State_Idle);

    fCylinderCenterPos = std::make_unique<G4UIcmdWith3VectorAndUnit>("/Mustard/EcoMug/Position/Cylinder", this);
    fCylinderCenterPos->SetGuidance("Set cylinder center position.");
    fCylinderCenterPos->SetParameterName("X", "Y", "Z", false, true);
    fCylinderCenterPos->SetUnitCategory("Length");
    fCylinderCenterPos->AvailableForStates(G4State_Idle);

    // size
    fSizeDirectory = std::make_unique<G4UIdirectory>("/Mustard/EcoMug/Size/");
    fSizeDirectory->SetGuidance("generator size directory");

    fSkySizeXY = std::make_unique<G4UIcmdWith3VectorAndUnit>("/Mustard/EcoMug/Size/SkyXY", this);
    fSkySizeXY->SetGuidance("Set the XY size of sky if generation shape is sky.\
                            \nPlease enter a 3vector x y z. Only x and y works.\
                            \nNote that: Native(xyz)->Beam(zxy)");
    fSkySizeXY->SetParameterName("X", "Y", "Z", false);
    fSkySizeXY->SetUnitCategory("Length");
    fSkySizeXY->SetRange("X >= 0 || Y >=0 || Z >= 0");
    fSkySizeXY->AvailableForStates(G4State_Idle);

    fSizeHSphereDirectory = std::make_unique<G4UIdirectory>("/Mustard/EcoMug/Size/Hsphere/");
    fSizeHSphereDirectory->SetGuidance("hsphere generator size directory");

    fHSphereRadius = std::make_unique<G4UIcmdWithADoubleAndUnit>("/Mustard/EcoMug/Size/Hsphere/Radius", this);
    fHSphereRadius->SetGuidance("Set the generation hsphere radius if used.");
    fHSphereRadius->SetParameterName("R", false);
    fHSphereRadius->SetUnitCategory("Length");
    fHSphereRadius->SetRange("R >= 0");
    fHSphereRadius->AvailableForStates(G4State_Idle);

    fSizeCylinderDirectory = std::make_unique<G4UIdirectory>("/Mustard/EcoMug/Size/Cylinder/");
    fSizeCylinderDirectory->SetGuidance("cylinder generator size directory");

    fCylinderRadius = std::make_unique<G4UIcmdWithADoubleAndUnit>("/Mustard/EcoMug/Size/Cylinder/Radius", this);
    fCylinderRadius->SetGuidance("Set the generation cylinder radius if used.");
    fCylinderRadius->SetParameterName("R", false);
    fCylinderRadius->SetUnitCategory("Length");
    fCylinderRadius->SetRange("R >= 0");
    fCylinderRadius->AvailableForStates(G4State_Idle);

    fCylinderHeight = std::make_unique<G4UIcmdWithADoubleAndUnit>("/Mustard/EcoMug/Size/Cylinder/Height", this);
    fCylinderHeight->SetGuidance("Set the generation cylinder height if used.");
    fCylinderHeight->SetParameterName("H", false);
    fCylinderHeight->SetUnitCategory("Length");
    fCylinderHeight->SetRange("H >= 0");
    fCylinderHeight->AvailableForStates(G4State_Idle);
    // energy
    fEnergyDirectory = std::make_unique<G4UIdirectory>("/Mustard/EcoMug/Energy/");
    fEnergyDirectory->SetGuidance("muon energy directory");

    fMaxMomentum = std::make_unique<G4UIcmdWithADoubleAndUnit>("/Mustard/EcoMug/Energy/Max_momentum", this);
    fMaxMomentum->SetGuidance("Set the max momentum of a muon.");
    fMaxMomentum->SetParameterName("EkMax", false);
    fMaxMomentum->SetUnitCategory("Energy");
    fMaxMomentum->SetRange("EkMax >= 0");
    fMaxMomentum->AvailableForStates(G4State_Idle);

    fMinMomentum = std::make_unique<G4UIcmdWithADoubleAndUnit>("/Mustard/EcoMug/Energy/Min_momentum", this);
    fMinMomentum->SetGuidance("Set the minimun momentum of a muon.");
    fMinMomentum->SetParameterName("EkMin", false);
    fMinMomentum->SetUnitCategory("Energy");
    fMinMomentum->SetRange("EkMin >= 0");
    fMinMomentum->AvailableForStates(G4State_Idle);
    // angle
    fAngleDirectory = std::make_unique<G4UIdirectory>("/Mustard/EcoMug/Angle/");
    fAngleDirectory->SetGuidance("muon angle directory");

    fMaxTheta = std::make_unique<G4UIcmdWithADoubleAndUnit>("/Mustard/EcoMug/Angle/Max_theta", this);
    fMaxTheta->SetGuidance("Set the max theta angle of a muon.");
    fMaxTheta->SetParameterName("theta", false);
    fMaxTheta->SetUnitCategory("Angle");
    fMaxTheta->SetRange("theta >= 0");
    fMaxTheta->AvailableForStates(G4State_Idle);
}

EcoMugCosmicRayMuonMessenger::~EcoMugCosmicRayMuonMessenger() = default;

auto EcoMugCosmicRayMuonMessenger::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fSkyCenterPos.get()) {
        Deliver<EcoMugCosmicRayMuon>([&](auto&& r) {
            r.UseSky();
            r.SkyCenterPosition(fSkyCenterPos->GetNew3VectorValue(value));
        });
    } else if (command == fHSphereCenterPos.get()) {
        Deliver<EcoMugCosmicRayMuon>([&](auto&& r) {
            r.UseHSphere();
            r.HSphereCenterPosition(fHSphereCenterPos->GetNew3VectorValue(value));
        });
    } else if (command == fCylinderCenterPos.get()) {
        Deliver<EcoMugCosmicRayMuon>([&](auto&& r) {
            r.UseCylinder();
            r.CylinderCenterPosition(fCylinderCenterPos->GetNew3VectorValue(value));
        });
    } else if (command == fSkySizeXY.get()) {
        Deliver<EcoMugCosmicRayMuon>([&](auto&& r) {
            auto XYZ{fSkySizeXY->GetNew3VectorValue(value)};
            r.SkySize(XYZ.getX(), XYZ.getY());
        });
    } else if (command == fHSphereRadius.get()) {
        Deliver<EcoMugCosmicRayMuon>([&](auto&& r) {
            r.HSphereRadius(fHSphereRadius->GetNewDoubleValue(value));
        });
    } else if (command == fCylinderRadius.get()) {
        Deliver<EcoMugCosmicRayMuon>([&](auto&& r) {
            r.CylinderRadius(fCylinderRadius->GetNewDoubleValue(value));
        });
    } else if (command == fCylinderHeight.get()) {
        Deliver<EcoMugCosmicRayMuon>([&](auto&& r) {
            r.CylinderHeight(fCylinderHeight->GetNewDoubleValue(value));
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
