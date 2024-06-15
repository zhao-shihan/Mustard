#pragma once

#include "G4ThreeVector.hh"
#include "G4TwoVector.hh"
#include "G4VPrimaryGenerator.hh"

#include <any>

namespace Mustard::inline Extension::Geant4X::inline Generator {

/// @brief EcoMug: Efficient COsmic MUon Generator
/// Cite: Pagano, D., Bonomi, G., Donzella, A., Zenoni, A., Zumerle, G., & Zurlo, N. (2021).
/// EcoMug: An Efficient COsmic MUon Generator for cosmic-ray muon applications. NIMA, 1014, 165732.
/// Code: https://github.com/dr4kan/EcoMug
class EcoMugCosmicRayMuon final : public G4VPrimaryGenerator {
public:
    EcoMugCosmicRayMuon();
    ~EcoMugCosmicRayMuon();

    auto UseSky() -> void;
    auto SkySize(G4TwoVector xy) -> void;
    auto SkyCenterPosition(G4ThreeVector x0) -> void;

    auto UseCylinder() -> void;
    auto CylinderRadius(double r) -> void;
    auto CylinderHeight(double h) -> void;
    auto CylinderCenterPosition(G4ThreeVector x0) -> void;

    auto UseHSphere() -> void;
    auto HSphereRadius(double r) -> void;
    auto HSphereCenterPosition(G4ThreeVector x0) -> void;

    auto GeneratePrimaryVertex(G4Event* event) -> void override;

private:
    std::any fEcoMug;
};

} // namespace Mustard::inline Extension::Geant4X::inline Generator
