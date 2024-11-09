#pragma once

#include "G4Point3D.hh"
#include "G4TessellatedSolid.hh"
#include "G4ThreeVector.hh"

#include <string>

namespace Mustard::inline Extension::Geant4X::inline Geometry {

class HelicalTube : public G4TessellatedSolid {
public:
    HelicalTube(std::string name,
                double majorRadius,             /* mm */
                double minorRadius,             /* mm */
                double pitch,                   /* rad */
                double phi0,                    /* rad */
                double phiTotal,                /* rad */
                double majorTolerance = 0.001,  /* 1 */
                double minorTolerance = 0.001); /* 1 */

    auto MajorRadius() const -> auto { return fMajorRadius; }
    auto MinorRadius() const -> auto { return fMinorRadius; }
    auto Pitch() const -> auto { return fPitch; }
    auto Phi0() const -> auto { return fPhi0; }
    auto PhiTotal() const -> auto { return fPhiTotal; }
    auto MajorTolerance() const -> auto { return fMajorTolerance; }
    auto MinorTolerance() const -> auto { return fMinorTolerance; }

    auto TotalLength() const -> auto { return fTotalLength; }
    auto ZLength() const -> auto { return fZLength; }
    auto FrontEndPosition() const -> auto { return fFrontEndPosition; }
    auto FrontEndNormal() const -> auto { return fFrontEndNormal; }
    auto BackEndPosition() const -> auto { return fBackEndPosition; }
    auto BackEndNormal() const -> auto { return fBackEndNormal; }

    auto GetEntityType() const -> G4GeometryType override { return "G4XHelicalTube"; }

private:
    double fMajorRadius;
    double fMinorRadius;
    double fPitch;
    double fPhi0;
    double fPhiTotal;
    double fMajorTolerance;
    double fMinorTolerance;

    double fTotalLength;
    double fZLength;
    G4Point3D fFrontEndPosition;
    G4ThreeVector fFrontEndNormal;
    G4Point3D fBackEndPosition;
    G4ThreeVector fBackEndNormal;
};

} // namespace Mustard::inline Extension::Geant4X::inline Geometry
