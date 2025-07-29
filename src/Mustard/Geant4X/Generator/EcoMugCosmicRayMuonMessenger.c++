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

#include "Mustard/Geant4X/Generator/EcoMugCosmicRayMuon.h++"
#include "Mustard/Geant4X/Generator/EcoMugCosmicRayMuonMessenger.h++"
#include "Mustard/Utility/LiteralUnit.h++"

#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAString.hh"

namespace Mustard::Geant4X::inline Generator {

using namespace LiteralUnit;

EcoMugCosmicRayMuonMessenger::EcoMugCosmicRayMuonMessenger() :
    SingletonMessenger{},
    fEcoMugDirectory{},
    fPositionDirectory{},
    fSkyCenterPosition{},
    fHSphereCenterPosition{},
    fCylinderCenterPosition{},
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
    fMaxTheta{} {

    fEcoMugDirectory = std::make_unique<G4UIdirectory>("/Mustard/EcoMug/");
    fEcoMugDirectory->SetGuidance("EcoMug cosmic ray generator extension.");

    // position and genetator shape
    fPositionDirectory = std::make_unique<G4UIdirectory>("/Mustard/EcoMug/Position/");
    fPositionDirectory->SetGuidance("Generator generation surface and position directory\n"
                                    "Available shape: sky, half sphere, cylinder.");

    fSkyCenterPosition = std::make_unique<G4UIcmdWith3VectorAndUnit>("/Mustard/EcoMug/Position/Sky", this);
    fSkyCenterPosition->SetGuidance("Set sky center position.");
    fSkyCenterPosition->SetParameterName("X", "Y", "Z", false);
    fSkyCenterPosition->SetUnitCategory("Length");
    fSkyCenterPosition->AvailableForStates(G4State_Idle);

    fHSphereCenterPosition = std::make_unique<G4UIcmdWith3VectorAndUnit>("/Mustard/EcoMug/Position/HSphere", this);
    fHSphereCenterPosition->SetGuidance("Set half phere center position.");
    fHSphereCenterPosition->SetParameterName("X", "Y", "Z", false);
    fHSphereCenterPosition->SetUnitCategory("Length");
    fHSphereCenterPosition->AvailableForStates(G4State_Idle);

    fCylinderCenterPosition = std::make_unique<G4UIcmdWith3VectorAndUnit>("/Mustard/EcoMug/Position/Cylinder", this);
    fCylinderCenterPosition->SetGuidance("Set cylinder center position.");
    fCylinderCenterPosition->SetParameterName("X", "Y", "Z", false);
    fCylinderCenterPosition->SetUnitCategory("Length");
    fCylinderCenterPosition->AvailableForStates(G4State_Idle);

    // size
    fSizeDirectory = std::make_unique<G4UIdirectory>("/Mustard/EcoMug/Size/");
    fSizeDirectory->SetGuidance("generator size directory");

    fSkySizeXY = std::make_unique<G4UIcmdWith3VectorAndUnit>("/Mustard/EcoMug/Size/SkyXY", this);
    fSkySizeXY->SetGuidance("Set the XY size of sky if generation shape is sky.\n"
                            "Please enter a 3vector x y z. Only x and y works.\n"
                            "Note that: Native(xyz)->Beam(zxy)");
    fSkySizeXY->SetParameterName("X", "Y", "Z", false);
    fSkySizeXY->SetUnitCategory("Length");
    fSkySizeXY->SetRange("X > 0 || Y > 0");
    fSkySizeXY->AvailableForStates(G4State_Idle);

    fSizeHSphereDirectory = std::make_unique<G4UIdirectory>("/Mustard/EcoMug/Size/HSphere/");
    fSizeHSphereDirectory->SetGuidance("hsphere generator size directory");

    fHSphereRadius = std::make_unique<G4UIcmdWithADoubleAndUnit>("/Mustard/EcoMug/Size/HSphere/Radius", this);
    fHSphereRadius->SetGuidance("Set the generation hsphere radius if used.");
    fHSphereRadius->SetParameterName("R", false);
    fHSphereRadius->SetUnitCategory("Length");
    fHSphereRadius->SetRange("R > 0");
    fHSphereRadius->AvailableForStates(G4State_Idle);

    fSizeCylinderDirectory = std::make_unique<G4UIdirectory>("/Mustard/EcoMug/Size/Cylinder/");
    fSizeCylinderDirectory->SetGuidance("Cylinder generator size directory");

    fCylinderRadius = std::make_unique<G4UIcmdWithADoubleAndUnit>("/Mustard/EcoMug/Size/Cylinder/Radius", this);
    fCylinderRadius->SetGuidance("Set the generation cylinder radius if used.");
    fCylinderRadius->SetParameterName("R", false);
    fCylinderRadius->SetUnitCategory("Length");
    fCylinderRadius->SetRange("R > 0");
    fCylinderRadius->AvailableForStates(G4State_Idle);

    fCylinderHeight = std::make_unique<G4UIcmdWithADoubleAndUnit>("/Mustard/EcoMug/Size/Cylinder/Height", this);
    fCylinderHeight->SetGuidance("Set the generation cylinder height if used.");
    fCylinderHeight->SetParameterName("H", false);
    fCylinderHeight->SetUnitCategory("Length");
    fCylinderHeight->SetRange("H > 0");
    fCylinderHeight->AvailableForStates(G4State_Idle);

    // energy
    fEnergyDirectory = std::make_unique<G4UIdirectory>("/Mustard/EcoMug/Energy/");
    fEnergyDirectory->SetGuidance("muon energy directory");

    fMaxMomentum = std::make_unique<G4UIcmdWithADoubleAndUnit>("/Mustard/EcoMug/Energy/MaxMomentum", this);
    fMaxMomentum->SetGuidance("Set the max momentum of a muon.");
    fMaxMomentum->SetParameterName("pMax", false);
    fMaxMomentum->SetUnitCategory("Energy");
    fMaxMomentum->SetRange("pMax > 0");
    fMaxMomentum->AvailableForStates(G4State_Idle);

    fMinMomentum = std::make_unique<G4UIcmdWithADoubleAndUnit>("/Mustard/EcoMug/Energy/MinMomentum", this);
    fMinMomentum->SetGuidance("Set the minimun momentum of a muon.");
    fMinMomentum->SetParameterName("pMin", false);
    fMinMomentum->SetUnitCategory("Energy");
    fMinMomentum->SetRange("pMin >= 0");
    fMinMomentum->AvailableForStates(G4State_Idle);

    // angle
    fAngleDirectory = std::make_unique<G4UIdirectory>("/Mustard/EcoMug/Angle/");
    fAngleDirectory->SetGuidance("muon angle directory");

    fMaxTheta = std::make_unique<G4UIcmdWithADoubleAndUnit>("/Mustard/EcoMug/Angle/MaxTheta", this);
    fMaxTheta->SetGuidance("Set the max theta angle of a muon.");
    fMaxTheta->SetParameterName("theta", false);
    fMaxTheta->SetUnitCategory("Angle");
    fMaxTheta->SetRange("theta > 0");
    fMaxTheta->AvailableForStates(G4State_Idle);
}

EcoMugCosmicRayMuonMessenger::~EcoMugCosmicRayMuonMessenger() = default;

auto EcoMugCosmicRayMuonMessenger::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fSkyCenterPosition.get()) {
        Deliver<EcoMugCosmicRayMuon>([&](auto&& r) {
            r.UseSky();
            r.SkyCenterPosition(fSkyCenterPosition->GetNew3VectorValue(value));
        });
    } else if (command == fHSphereCenterPosition.get()) {
        Deliver<EcoMugCosmicRayMuon>([&](auto&& r) {
            r.UseHSphere();
            r.HSphereCenterPosition(fHSphereCenterPosition->GetNew3VectorValue(value));
        });
    } else if (command == fCylinderCenterPosition.get()) {
        Deliver<EcoMugCosmicRayMuon>([&](auto&& r) {
            r.UseCylinder();
            r.CylinderCenterPosition(fCylinderCenterPosition->GetNew3VectorValue(value));
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

} // namespace Mustard::Geant4X::inline Generator
