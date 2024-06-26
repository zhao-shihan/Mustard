#pragma once

#include "AirShower/Detector/Description/Atmosphere.h++"
#include "AirShower/Detector/Description/Earth.h++"
#include "AirShower/Detector/Description/World.h++"

#include "Mustard/Env/Memory/PassiveSingleton.h++"

#include "G4VUserDetectorConstruction.hh"

#include <memory>

namespace Mustard::Detector::Definition {
class DefinitionBase;
} // namespace Mustard::Detector::Definition

namespace AirShower {

inline namespace SD {
class EarthSD;
} // namespace SD

inline namespace Action {

class DetectorConstruction final : public Mustard::Env::Memory::PassiveSingleton<DetectorConstruction>,
                                   public G4VUserDetectorConstruction {
public:
    DetectorConstruction(bool checkOverlap);

    auto Construct() -> G4VPhysicalVolume* override;

public:
    using DescriptionInUse = std::tuple<Detector::Description::Atmosphere,
                                        Detector::Description::Earth,
                                        Detector::Description::World>;

private:
    bool fCheckOverlap;

    std::unique_ptr<Mustard::Detector::Definition::DefinitionBase> fWorld;
};

} // namespace Action

} // namespace AirShower
