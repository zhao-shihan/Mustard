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

#include "Mustard/Geant4X/Generator/EcoMugCosmicRayMuonMessenger.h++"
#include "Mustard/Utility/LiteralUnit.h++"

#include "G4ThreeVector.hh"
#include "G4VPrimaryGenerator.hh"

#include "muc/array"

namespace Mustard::Geant4X::inline Generator {

using namespace Mustard::LiteralUnit::NumberFlux;

/// @brief EcoMug: Efficient COsmic MUon Generator
/// Cite: Pagano, D., Bonomi, G., Donzella, A., Zenoni, A., Zumerle, G., & Zurlo, N. (2021).
/// EcoMug: An Efficient COsmic MUon Generator for cosmic-ray muon applications. NIMA, 1014, 165732.
/// Code: https://github.com/dr4kan/EcoMug
class EcoMugCosmicRayMuon : public G4VPrimaryGenerator {
public:
    enum struct Coordinate {
        /*      z
         *      |
         *      +-- y
         *     /
         *    x         */
        Native,
        /*      y
         *      |
         *      +-- x
         *     /
         *    z         */
        Beam
    };

public:
    EcoMugCosmicRayMuon(Coordinate c = Coordinate::Native);
    ~EcoMugCosmicRayMuon();

    auto UseSky() -> void;
    auto SkySize(double x, double y) -> void;
    auto SkyCenterPosition(G4ThreeVector x0) -> void;

    auto UseCylinder() -> void;
    auto CylinderRadius(double r) -> void;
    auto CylinderHeight(double h) -> void;
    auto CylinderCenterPosition(G4ThreeVector x0) -> void;

    auto UseHSphere() -> void;
    auto HSphereRadius(double r) -> void;
    auto HSphereCenterPosition(G4ThreeVector x0) -> void;

    auto MaxTheta(double thetaMax) -> void;
    auto MinMomentum(double pMin) -> void;
    auto MaxMomentum(double pMax) -> void;

    auto GeneratePrimaryVertex(G4Event* event) -> void override;

    auto EstimatedTime(double nMuon, double horizontalFlux = 129_m_2_s_1) -> double;

private:
    auto EcoMuG() const -> const auto&;
    auto EcoMuG() -> auto&;

    auto ToEcoMug(G4ThreeVector x) -> muc::array3d;
    auto ToGeant4(muc::array3d x) -> G4ThreeVector;

private:
    void* fEcoMug;
    enum Coordinate fCoordinate;
    unsigned int fReseedCounter : 8;

    EcoMugCosmicRayMuonMessenger::Register<EcoMugCosmicRayMuon> fMessengerRegister;
};

} // namespace Mustard::Geant4X::inline Generator
