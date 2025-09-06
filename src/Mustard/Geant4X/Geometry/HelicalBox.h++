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

#pragma once

#include "G4Point3D.hh"
#include "G4TessellatedSolid.hh"
#include "G4ThreeVector.hh"

#include <string>

namespace Mustard::Geant4X::inline Geometry {

class HelicalBox : public G4TessellatedSolid {
public:
    HelicalBox(std::string name,
               double radius,   /* mm */
               double width,    /* mm */
               double pitch,    /* rad */
               double phi0,     /* rad */
               double phiTotal, /* rad */
               bool frontPlanar = false,
               bool backPlanar = false,
               double tolerance = 0.001); /* 1 */

    auto Radius() const -> auto { return fRadius; }
    auto Width() const -> auto { return fWidth; }
    auto Pitch() const -> auto { return fPitch; }
    auto Phi0() const -> auto { return fPhi0; }
    auto PhiTotal() const -> auto { return fPhiTotal; }
    auto FrontPlanar() const -> auto { return fFrontPlanar; }
    auto BackPlanar() const -> auto { return fBackPlanar; }
    auto Tolerance() const -> auto { return fTolerance; }

    auto TotalLength() const -> auto { return fTotalLength; }
    auto ZLength() const -> auto { return fZLength; }
    auto FrontEndPosition() const -> auto { return fFrontEndPosition; }
    auto FrontEndNormal() const -> auto { return fFrontEndNormal; }
    auto BackEndPosition() const -> auto { return fBackEndPosition; }
    auto BackEndNormal() const -> auto { return fBackEndNormal; }

private:
    double fRadius;
    double fWidth;
    double fPitch;
    double fPhi0;
    double fPhiTotal;
    bool fFrontPlanar;
    bool fBackPlanar;
    double fTolerance;

    double fTotalLength;
    double fZLength;
    G4Point3D fFrontEndPosition;
    G4ThreeVector fFrontEndNormal;
    G4Point3D fBackEndPosition;
    G4ThreeVector fBackEndNormal;
};

} // namespace Mustard::Geant4X::inline Geometry
