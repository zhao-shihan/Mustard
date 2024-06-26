#pragma once

#include "Mustard/Env/Memory/PassiveSingleton.h++"

#include "G4UserRunAction.hh"

namespace AirShower::inline Action {

class RunAction final : public Mustard::Env::Memory::PassiveSingleton<RunAction>,
                        public G4UserRunAction {
public:
    auto BeginOfRunAction(const G4Run* run) -> void override;
    auto EndOfRunAction(const G4Run*) -> void override;
};

} // namespace AirShower::inline Action
