#pragma once

#include "Mustard/Env/Memory/PassiveSingleton.h++"

#include "G4VUserActionInitialization.hh"

namespace AirShower::inline Action {

class ActionInitialization final : public Mustard::Env::Memory::PassiveSingleton<ActionInitialization>,
                                   public G4VUserActionInitialization {
public:
    auto Build() const -> void override;
};

} // namespace AirShower::inline Action
