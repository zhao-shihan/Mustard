#include "AirShower/Detector/Definition/Atmosphere.h++"
#include "AirShower/Detector/Description/Atmosphere.h++"
#include "AirShower/Detector/Description/World.h++"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4Transform3D.hh"

#include "gsl/gsl"

namespace AirShower::Detector::Definition {

auto Atmosphere::Construct(bool checkOverlaps) -> void {
    const auto& atmosphere{Description::Atmosphere::Instance()};
    const auto& world{Description::World::Instance()};

    G4LogicalVolume* outerAtmosphere{};
    for (gsl::index i{atmosphere.NPressureSlice() - 1}; i >= 0; --i) {
        const auto solid{Make<G4Box>(
            atmosphere.Name(),
            world.Width() / 2,
            world.Width() / 2,
            atmosphere.AltitudeSlice()[i] / 2)};
        const auto logic{Make<G4LogicalVolume>(
            solid,
            G4NistManager::Instance()->BuildMaterialWithNewDensity(
                fmt::format("AtmosphereSlice{}", i), "G4_AIR",
                atmosphere.StateSlice()[i].density,
                atmosphere.StateSlice()[i].temperature,
                atmosphere.StateSlice()[i].pressure),
            atmosphere.Name())};
        Make<G4PVPlacement>(
            G4TranslateZ3D{outerAtmosphere ? -atmosphere.AltitudeSlice()[i + 1] / 2 + atmosphere.AltitudeSlice()[i] / 2 :
                                             atmosphere.AltitudeSlice()[i] / 2},
            logic,
            atmosphere.Name(),
            outerAtmosphere ? outerAtmosphere :
                              Mother().LogicalVolume(),
            false,
            i,
            checkOverlaps);
        outerAtmosphere = logic;
    }
}

} // namespace AirShower::Detector::Definition
