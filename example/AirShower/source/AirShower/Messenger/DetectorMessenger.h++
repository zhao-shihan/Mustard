#pragma once

#include "AirShower/Action/DetectorConstruction.h++"

#include "Mustard/Extension/Geant4X/Interface/DetectorMessenger.h++"

namespace AirShower::inline Messenger {

class DetectorMessenger final : public Mustard::Geant4X::DetectorMessenger<DetectorMessenger,
                                                                           DetectorConstruction,
                                                                           "AirShower"> {
    friend Mustard::Env::Memory::SingletonInstantiator;

private:
    DetectorMessenger() = default;
    ~DetectorMessenger() = default;
};

} // namespace AirShower::inline Messenger
