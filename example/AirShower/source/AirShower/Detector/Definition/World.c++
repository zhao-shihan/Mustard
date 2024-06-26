#include "AirShower/Detector/Definition/World.h++"
#include "AirShower/Detector/Description/World.h++"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4Transform3D.hh"

namespace AirShower::Detector::Definition {

auto World::Construct(bool checkOverlaps) -> void {
    const auto& world{Description::World::Instance()};

    const auto solid{Make<G4Box>(
        world.Name(),
        world.Width() / 2,
        world.Width() / 2,
        world.MaxHeight())};
    const auto logic{Make<G4LogicalVolume>(
        solid,
        G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic"),
        world.Name())};
    Make<G4PVPlacement>(
        G4Transform3D{},
        logic,
        world.Name(),
        nullptr,
        false,
        0,
        checkOverlaps);
}

} // namespace AirShower::Detector::Definition
