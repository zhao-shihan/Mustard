#pragma once

#include "G4Point3D.hh"
#include "G4TessellatedSolid.hh"
#include "G4ThreeVector.hh"

#include <string>

namespace Mustard::inline Extension::Geant4X::inline Geometry {

class HelicalTube : public G4TessellatedSolid {
public:
    HelicalTube(std::string name,
                double majorRadius,        /* mm */
                double minorRadius,        /* mm */
                double pitch,              /* rad */
                double phi0,               /* rad */
                double phiTotal,           /* rad */
                double tolerance = 0.001); /* 1 */

    auto GetEntityType() const -> G4GeometryType override { return "G4XHelicalTube"; }

    auto FrontEndPosition() const -> G4Point3D;
    auto FrontEndNormal() const -> G4ThreeVector;
    auto BackEndPosition() const -> G4Point3D;
    auto BackEndNormal() const -> G4ThreeVector;

private:
    G4Point3D fFrontEndPosition;
    G4ThreeVector fFrontEndNormal;
    G4Point3D fBackEndPosition;
    G4ThreeVector fBackEndNormal;
};

} // namespace Mustard::inline Extension::Geant4X::inline Geometry
