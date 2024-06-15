#pragma once

#include "Mustard/Detector/Description/Description.h++"

#include "G4ThreeVector.hh"

#include <concepts>

namespace Mustard::inline Extension::Geant4X::inline Physics {

template<typename T>
concept TargetForMuoniumPhysics =
    requires(T target, G4ThreeVector x) {
        requires Detector::Description::Description<T>;
        { target.VolumeContain(x) } -> std::convertible_to<bool>;
        { target.Contain(x) } -> std::convertible_to<bool>;
    };

} // namespace Mustard::inline Extension::Geant4X::inline Physics
