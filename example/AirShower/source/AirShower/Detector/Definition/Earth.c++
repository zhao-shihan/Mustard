#include "AirShower/Detector/Definition/Earth.h++"
#include "AirShower/Detector/Description/Earth.h++"
#include "AirShower/Detector/Description/World.h++"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4Transform3D.hh"

namespace AirShower::Detector::Definition {

auto Earth::Construct(bool checkOverlaps) -> void {
    const auto& earth{Description::Earth::Instance()};
    const auto& world{Description::World::Instance()};

    const auto solid{Make<G4Box>(
        earth.Name(),
        world.Width() / 2,
        world.Width() / 2,
        earth.Depth() / 2)};
    const auto logic{Make<G4LogicalVolume>(
        solid,
        G4NistManager::Instance()->FindOrBuildMaterial("G4_WATER"),
        earth.Name())};
    Make<G4PVPlacement>(
        G4TranslateZ3D{-earth.Depth() / 2},
        logic,
        earth.Name(),
        Mother().LogicalVolume(),
        false,
        0,
        checkOverlaps);
}

} // namespace AirShower::Detector::Definition
