#pragma once

#include "Mustard/Env/Memory/PassiveSingleton.h++"

#include "G4UserEventAction.hh"

namespace AirShower::inline Action {

class EventAction final : public Mustard::Env::Memory::PassiveSingleton<EventAction>,
                          public G4UserEventAction {
public:
    auto BeginOfEventAction(const G4Event*) -> void override;
    auto EndOfEventAction(const G4Event*) -> void override;
};

} // namespace AirShower::inline Action
