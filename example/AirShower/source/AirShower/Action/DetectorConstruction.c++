#include "AirShower/Action/DetectorConstruction.h++"
#include "AirShower/Detector/Definition/Atmosphere.h++"
#include "AirShower/Detector/Definition/Earth.h++"
#include "AirShower/Detector/Definition/World.h++"
#include "AirShower/Messenger/DetectorMessenger.h++"
#include "AirShower/SD/EarthSD.h++"

#include "Mustard/Detector/Definition/DefinitionBase.h++"
#include "Mustard/Detector/Field/AsG4Field.h++"
#include "Mustard/Detector/Field/UniformMagneticField.h++"
#include "Mustard/Utility/LiteralUnit.h++"
#include "Mustard/Utility/VectorCast.h++"

#include "G4ChordFinder.hh"
#include "G4InterpolationDriver.hh"
#include "G4TDormandPrince45.hh"
#include "G4TMagFieldEquation.hh"
#include "G4ThreeVector.hh"

#include "gsl/gsl"

namespace AirShower::inline Action {

DetectorConstruction::DetectorConstruction(bool checkOverlap) :
    PassiveSingleton{},
    G4VUserDetectorConstruction{},
    fCheckOverlap{checkOverlap},
    fWorld{} {
    DetectorMessenger::EnsureInstantiation();
}

auto DetectorConstruction::Construct() -> G4VPhysicalVolume* {
    // Construct volumes

    fWorld = std::make_unique<Detector::Definition::World>();
    auto& earth{fWorld->NewDaughter<Detector::Definition::Earth>(fCheckOverlap)};
    auto& atmosphere{fWorld->NewDaughter<Detector::Definition::Atmosphere>(fCheckOverlap)};

    // Register SD

    earth.RegisterSD(new SD::EarthSD{"EarthSD"});

    // Register field
    using namespace Mustard::LiteralUnit::Length;
    constexpr auto hMin{1_m};
    using Field = Mustard::Detector::Field::AsG4Field<Mustard::Detector::Field::UniformMagneticField>;
    using Equation = G4TMagFieldEquation<Field>;
    using Stepper = G4TDormandPrince45<Equation, 6>;
    using Driver = G4InterpolationDriver<Stepper>;
    const auto field{new Field{Detector::Description::Field::Instance().MagneticField()}};
    const auto equation{new Equation{field}}; // clang-format off
    const auto stepper{new Stepper{equation, 6}};
    const auto driver{new Driver{hMin, stepper, 6}}; // clang-format on
    const auto chordFinder{new G4ChordFinder{driver}};
    fWorld->RegisterField(std::make_unique<G4FieldManager>(field, chordFinder), false);

    return fWorld->PhysicalVolume();
}

} // namespace AirShower::inline Action
