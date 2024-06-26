#include "AirShower/Action/DetectorConstruction.h++"
#include "AirShower/Detector/Definition/Atmosphere.h++"
#include "AirShower/Detector/Definition/Earth.h++"
#include "AirShower/Detector/Definition/World.h++"
#include "AirShower/Messenger/DetectorMessenger.h++"
#include "AirShower/SD/EarthSD.h++"

#include "Mustard/Detector/Definition/DefinitionBase.h++"
#include "Mustard/Detector/Field/AsG4Field.h++"
#include "Mustard/Utility/LiteralUnit.h++"

#include "G4ChordFinder.hh"
#include "G4EqMagElectricField.hh"
#include "G4InterpolationDriver.hh"
#include "G4NistManager.hh"
#include "G4ProductionCuts.hh"
#include "G4ProductionCutsTable.hh"
#include "G4TDormandPrince45.hh"
#include "G4TMagFieldEquation.hh"

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
    /* {
        using namespace Mustard::LiteralUnit::Length;
        using namespace Mustard::LiteralUnit::MagneticFluxDensity;

        constexpr auto hMin{1_um};
        { // magnetic field
            const auto RegisterMagneticField{
                []<typename AField>(Mustard::Detector::Definition::DefinitionBase& detector, AField* field, bool forceToAllDaughters) {
                    using Equation = G4TMagFieldEquation<AField>;
                    using Stepper = G4TDormandPrince45<Equation, 6>;
                    using Driver = G4InterpolationDriver<Stepper>;
                    const auto equation{new Equation{field}}; // clang-format off
                    const auto stepper{new Stepper{equation, 6}};
                    const auto driver{new Driver{hMin, stepper, 6}}; // clang-format on
                    const auto chordFinder{new G4ChordFinder{driver}};
                    detector.RegisterField(std::make_unique<G4FieldManager>(field, chordFinder), forceToAllDaughters);
                }};
            RegisterMagneticField(mmsField, new Mustard::Detector::Field::AsG4Field<Detector::Field::MMSField>, false);
            RegisterMagneticField(solenoidFieldS1, new Mustard::Detector::Field::AsG4Field<Detector::Field::SolenoidFieldS1>, false);
            RegisterMagneticField(solenoidFieldT1, new Mustard::Detector::Field::AsG4Field<Detector::Field::SolenoidFieldT1>, false);
            RegisterMagneticField(solenoidFieldS2, new Mustard::Detector::Field::AsG4Field<Detector::Field::SolenoidFieldS2>, false);
            RegisterMagneticField(solenoidFieldT2, new Mustard::Detector::Field::AsG4Field<Detector::Field::SolenoidFieldT2>, false);
            RegisterMagneticField(solenoidFieldS3, new Mustard::Detector::Field::AsG4Field<Detector::Field::SolenoidFieldS3>, false);
            RegisterMagneticField(emcField, new Mustard::Detector::Field::AsG4Field<Detector::Field::EMCField>, false);
        }
        { // EM field, must be reigstered after MMS magnetic field! but why?
            using Equation = G4EqMagElectricField;
            using Stepper = G4TDormandPrince45<Equation, 8>;
            using Driver = G4InterpolationDriver<Stepper>;
            const auto field{new Mustard::Detector::Field::AsG4Field<Detector::Field::AcceleratorField>};
            const auto equation{new Equation{field}}; // clang-format off
            const auto stepper{new Stepper{equation, 8}};
            const auto driver{new Driver{hMin, stepper, 8}}; // clang-format on
            const auto chordFinder{new G4ChordFinder{driver}};
            acceleratorField.RegisterField(std::make_unique<G4FieldManager>(field, chordFinder), false);
        }
    } */

    return fWorld->PhysicalVolume();
}

} // namespace AirShower::inline Action
