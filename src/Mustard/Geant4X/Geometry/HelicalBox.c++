// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

#include "Mustard/Geant4X/Geometry/HelicalBox.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Utility/MathConstant.h++"

#include "G4QuadrangularFacet.hh"
#include "G4TriangularFacet.hh"

#include "Eigen/Core"

#include "muc/math"
#include "muc/numeric"

#include <cmath>
#include <exception>
#include <utility>

namespace Mustard::Geant4X::inline Geometry {

using namespace Mustard::MathConstant;

HelicalBox::HelicalBox(std::string name,
                       double radius,
                       double width,
                       double pitch,
                       double phi0,
                       double phiTotal,
                       bool frontPlanar,
                       bool backPlanar,
                       double tolerance) :
    G4TessellatedSolid{std::move(name)},
    fRadius{radius},
    fWidth{width},
    fPitch{pitch},
    fPhi0{phi0},
    fPhiTotal{phiTotal},
    fFrontPlanar{frontPlanar},
    fBackPlanar{backPlanar},
    fTolerance{tolerance},
    fTotalLength{},
    fZLength{},
    fFrontEndPosition{},
    fFrontEndNormal{},
    fBackEndPosition{},
    fBackEndNormal{} {
    const auto cosA{std::cos(pitch)};
    const auto sinA{std::sin(pitch)};
    const auto tanA{sinA / cosA};
    const auto tanAR{radius * tanA};
    const auto zOffset{(phi0 + phiTotal / 2) * tanAR};
    double tFront;
    double tBack;
    fTotalLength = radius * phiTotal / cosA;
    fZLength = tanAR * phiTotal;
    // prepare uv mesh
    const auto deltaU0{std::sqrt(8 * tolerance) * cosA};
    const auto n{muc::llround(phiTotal / deltaU0) + 2};
    const auto deltaU{phiTotal / (n - 1)};
    Eigen::VectorXd u(n);
    muc::ranges::iota(u, 0);
    u *= deltaU;

    // compute end position and normal
    const auto Helix{
        [&](const auto& u) -> G4Point3D {
            const auto u1{u + phi0};
            return {radius * std::cos(u1),
                    radius * std::sin(u1),
                    u1 * tanAR - zOffset};
        }};
    const auto EndFaceNormal{
        [&](const auto& u) -> G4ThreeVector {
            const auto u1{u + phi0};
            return {-radius * std::sin(u1),
                    radius * std::cos(u1),
                    tanAR};
        }};

    fFrontEndPosition = Helix(0);
    fFrontEndNormal = EndFaceNormal(0).unit();
    fBackEndPosition = Helix(phiTotal);
    fBackEndNormal = EndFaceNormal(phiTotal).unit();

    // parameterized surface
    const auto MainPoint{
        [&](const auto& u, int j) -> G4Point3D {
            const auto u1{u + phi0};
            const auto cosU{std::cos(u1)};
            const auto sinU{std::sin(u1)};
            const auto r{(sqrt2 / 2) * width};
            const auto rCosV{r * std::cos(j * (pi / 2) - (3 * pi / 4))};
            const auto rSinV{r * std::sin(j * (pi / 2) - (3 * pi / 4))};
            const auto rSinVSinA{rSinV * sinA};

            auto x{(radius + rCosV) * cosU + rSinVSinA * sinU};
            auto y{(radius + rCosV) * sinU - rSinVSinA * cosU};
            auto z{u1 * tanAR + rSinV * cosA - zOffset};

            if (u == 0 and frontPlanar) {
                const auto endZ = fFrontEndPosition.z();
                double t{(endZ - z) / fFrontEndNormal.z()};
                x += t * fFrontEndNormal.x();
                y += t * fFrontEndNormal.y();
                z += t * fFrontEndNormal.z();
                tFront = t;
                return {x, y, z};
            } else if (std::abs(u - phiTotal) < 1e-15 and backPlanar) {
                const auto endZ = fBackEndPosition.z();
                double t{(endZ - z) / fBackEndNormal.z()};
                x += t * fBackEndNormal.x();
                y += t * fBackEndNormal.y();
                z += t * fBackEndNormal.z();
                tBack = t;
                return {x, y, z};
            } else {
                return {x, y, z};
            }
        }};
    Eigen::MatrixX<G4Point3D> x(n, 5); // main grid
    for (auto i{0ll}; i < n; ++i) {
        x(i, 0) = MainPoint(u[i], 0);
        x(i, 1) = MainPoint(u[i], 1);
        x(i, 2) = MainPoint(u[i], 2);
        x(i, 3) = MainPoint(u[i], 3);
        x(i, 4) = x(i, 0);
    }

    for (int j{}; j < 4; j++) {
        if (frontPlanar) {
            double t0{(fFrontEndPosition.z() - x(1, j).z()) / fFrontEndNormal.z()};
            if (std::abs(tFront) > std::abs(t0)) {
                Mustard::Throw<std::runtime_error>("the back end can not be planar!");
            }
        } else if (backPlanar) {
            double t0{(fBackEndPosition.z() - x(n - 2, j).z()) / fBackEndNormal.z()};
            if (std::abs(tBack) > std::abs(t0)) {
                Mustard::Throw<std::runtime_error>("the front end can not be planar!");
            }
        }
    }

    const auto AuxillaryPoint{
        [&](const auto& u, int j) -> G4Point3D {
            const auto u1{u + phi0 + deltaU / 2};
            const auto cosU{std::cos(u1)};
            const auto sinU{std::sin(u1)};
            const auto r{width / 2};
            const auto rCosV{r * std::cos(j * pi - (pi / 2))};
            const auto rSinV{r * std::sin(j * pi - (pi / 2))};
            const auto rSinVSinA{rSinV * sinA};
            return {(radius + rCosV) * cosU + rSinVSinA * sinU,
                    (radius + rCosV) * sinU - rSinVSinA * cosU,
                    u1 * tanAR + rSinV * cosA - zOffset};
        }};
    Eigen::MatrixX<G4Point3D> c(n - 1, 2); // auxillary grid
    for (auto i{0ll}; i < n - 1; ++i) {
        c(i, 0) = AuxillaryPoint(u[i], 0);
        c(i, 1) = AuxillaryPoint(u[i], 1);
    }

    // make helical box
    const auto AddSideTwistedFacet{
        [&](int i, int j) {
            const auto i1{i + 1};
            const auto j1{j + 1};
            AddFacet(new G4TriangularFacet{x(i, j), x(i1, j), c(i, j / 2), ABSOLUTE});
            AddFacet(new G4TriangularFacet{x(i1, j), x(i1, j1), c(i, j / 2), ABSOLUTE});
            AddFacet(new G4TriangularFacet{x(i1, j1), x(i, j1), c(i, j / 2), ABSOLUTE});
            AddFacet(new G4TriangularFacet{x(i, j1), x(i, j), c(i, j / 2), ABSOLUTE});
        }};
    const auto AddInOutTwistedFacet{
        [&](int i, int j, bool in) {
            const auto i1{i + 1};
            const auto j1{j + 1};
            if ((tanAR >= 0) xor in) {
                /* tan{\alpha} > 0, these facets are concave:
                // tan{\alpha} < 0, these facets are convex:
                //
                //  (i ,j1)--(i1,j1)
                //    \      /    \
                //     \    /      \
                //    (i ,j )--(i1,j )
                */
                AddFacet(new G4TriangularFacet{x(i1, j1), x(i, j1), x(i, j), ABSOLUTE});
                AddFacet(new G4TriangularFacet{x(i, j), x(i1, j), x(i1, j1), ABSOLUTE});
            } else {
                /* tan{\alpha} < 0, these facets are concave:
                // tan{\alpha} > 0, these facets are convex:
                //
                //    (i1,j2)--(i2,j2)
                //     /    \      /
                //    /      \    /
                //  (i1,j1)--(i2,j1)
                */
                AddFacet(new G4TriangularFacet{x(i, j1), x(i, j), x(i1, j), ABSOLUTE});
                AddFacet(new G4TriangularFacet{x(i1, j), x(i1, j1), x(i, j1), ABSOLUTE});
            }
        }};
    for (auto i{0ll}; i < n - 1; ++i) {
        AddSideTwistedFacet(i, 0);
        AddInOutTwistedFacet(i, 1, false);
        AddSideTwistedFacet(i, 2);
        AddInOutTwistedFacet(i, 3, true);
    }

    // seal front end
    AddFacet(new G4QuadrangularFacet{x(0, 0), x(0, 1), x(0, 2), x(0, 3), ABSOLUTE});
    // seal back end
    AddFacet(new G4QuadrangularFacet{x(n - 1, 3), x(n - 1, 2), x(n - 1, 1), x(n - 1, 0), ABSOLUTE});

    SetSolidClosed(true);
}

} // namespace Mustard::Geant4X::inline Geometry
