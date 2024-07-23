#pragma once

#include "Mustard/Extension/Geant4X/Interface/SingletonMessenger.h++"

#include "G4UIcmdWithAString.hh"
#include "G4UIdirectory.hh"

#include "muc/concepts"

#include <concepts>
#include <memory>
#include <string_view>

namespace Mustard::Simulation {

template<typename Analysis>
class AnalysisBaseMessenger final : public Geant4X::SingletonMessenger<AnalysisBaseMessenger<Analysis>,
                                                                       Analysis> {
    friend Mustard::Env::Memory::SingletonInstantiator;

private:
    AnalysisBaseMessenger();
    ~AnalysisBaseMessenger() = default;

public:
    auto SetNewValue(G4UIcommand* command, G4String value) -> void override;

private:
    std::unique_ptr<G4UIdirectory> fDirectory;
    std::unique_ptr<G4UIcmdWithAString> fFilePath;
    std::unique_ptr<G4UIcmdWithAString> fFileMode;
};

} // namespace Mustard::Simulation

#include "Mustard/Simulation/AnalysisBaseMessenger.inl"
