#include "Mustard/Extension/Geant4X/Geometry/HelicalTube.h++"
#include "Mustard/Math/Parity.h++"
#include "Mustard/Utility/MathConstant.h++"

#include "G4TriangularFacet.hh"

#include "Eigen/Core"

#include "muc/math"
#include "muc/numeric"

#include <cmath>
#include <utility>

namespace Mustard::inline Extension::Geant4X::inline Geometry {

using namespace Mustard::MathConstant;

HelicalTube::HelicalTube(std::string name,
                         double majorRadius,
                         double minorRadius,
                         double pitch,
                         double phi0,
                         double phiTotal,
                         double majorTolerance,
                         double minorTolerance) :
    G4TessellatedSolid{std::move(name)},
    fMajorRadius{majorRadius},
    fMinorRadius{minorRadius},
    fPitch{pitch},
    fPhi0{phi0},
    fPhiTotal{phiTotal},
    fMajorTolerance{majorTolerance},
    fMinorTolerance{minorTolerance},
    fTotalLength{},
    fZLength{},
    fFrontEndPosition{},
    fFrontEndNormal{},
    fBackEndPosition{},
    fBackEndNormal{} {
    const auto cosA{std::cos(pitch)};
    const auto sinA{std::sin(pitch)};
    const auto tanA{sinA / cosA};
    const auto tanAR{majorRadius * tanA};
    const auto zOffset{(phi0 + phiTotal / 2) * tanAR};
    fTotalLength = majorRadius * phiTotal / cosA;
    fZLength = tanAR * phiTotal;

    // prepare uv mesh
    const auto deltaU0{std::sqrt(8 * majorTolerance) * cosA};
    const auto deltaV0{std::sqrt(8 * minorTolerance)};
    const auto nU{muc::llround(phiTotal / deltaU0) + 2};
    const auto nV{muc::llround(2 * pi / deltaV0) + 3};
    const auto deltaU{phiTotal / (nU - 1)};
    const auto deltaV{2 * pi / (nV - 1)};
    Eigen::VectorXd u(nU);
    Eigen::VectorXd v(nV);
    muc::ranges::iota(u, 0);
    muc::ranges::iota(v, 0);
    u *= deltaU;
    v *= deltaV;
    v[nV - 1] = 0; // rollback

    // parameterized surface
    const auto Surface{
        [&](const auto& u, const auto& v) -> G4Point3D {
            const auto u1{u + phi0};
            const auto cosU{std::cos(u1)};
            const auto sinU{std::sin(u1)};
            const auto rCosV{minorRadius * std::cos(v)};
            const auto rSinV{minorRadius * std::sin(v)};
            const auto rSinVSinA{rSinV * sinA};
            return {(majorRadius + rCosV) * cosU + rSinVSinA * sinU,
                    (majorRadius + rCosV) * sinU - rSinVSinA * cosU,
                    u1 * tanAR + rSinV * cosA - zOffset};
        }};
    Eigen::MatrixX<G4Point3D> x(nU, nV);
    for (auto i{0ll}; i < nU; ++i) {
        for (auto j{0ll}; j < nV; ++j) {
            x(i, j) = Surface(u[i], Mustard::Math::IsEven(j) ? v[j] :
                                                               v[j] + deltaV / 2);
        }
    }

    // make main tube
    const auto AddDoubleTwistedFacet{
        [&](int i, int j) {
            const auto i1{i + 1};
            const auto j1{j + 1};
            if (tanAR >= 0) {
                /* tan{\alpha} < 0, these facets are concave:
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
                //
                //    (i ,j1)--(i1,j1)
                //     /    \      /
                //    /      \    /
                //  (i ,j )--(i1,j )
                */
                AddFacet(new G4TriangularFacet{x(i, j1), x(i, j), x(i1, j), ABSOLUTE});
                AddFacet(new G4TriangularFacet{x(i1, j), x(i1, j1), x(i, j1), ABSOLUTE});
            }
        }};
    for (auto i{0ll}; i < nU - 1; ++i) {
        for (auto j{0ll}; j < nV - 1; ++j) {
            AddDoubleTwistedFacet(i, j);
        }
    }

    // compute end position and normal
    const auto Helix{
        [&](const auto& u) -> G4Point3D {
            const auto u1{u + phi0};
            return {majorRadius * std::cos(u1),
                    majorRadius * std::sin(u1),
                    u1 * tanAR - zOffset};
        }};
    const auto EndFaceNormal{
        [&](const auto& u) -> G4ThreeVector {
            const auto u1{u + phi0};
            return {-majorRadius * std::sin(u1),
                    majorRadius * std::cos(u1),
                    tanAR};
        }};
    fFrontEndPosition = Helix(0);
    fFrontEndNormal = EndFaceNormal(0).unit();
    fBackEndPosition = Helix(phiTotal);
    fBackEndNormal = EndFaceNormal(phiTotal).unit();

    // seal front end
    for (auto j{0ll}; j < nV - 1; ++j) {
        AddFacet(new G4TriangularFacet{fFrontEndPosition, x(0, j), x(0, j + 1), ABSOLUTE});
    }
    // seal back end
    for (auto j{nV - 1}; j > 0; --j) {
        AddFacet(new G4TriangularFacet{fBackEndPosition, x(nU - 1, j), x(nU - 1, j - 1), ABSOLUTE});
    }

    SetSolidClosed(true);
}

} // namespace Mustard::inline Extension::Geant4X::inline Geometry