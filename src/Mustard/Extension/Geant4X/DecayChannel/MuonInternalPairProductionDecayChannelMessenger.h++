#pragma once

#include "Mustard/Extension/Geant4X/Interface/SingletonMessenger.h++"

#include <memory>

class G4UIcmdWithADouble;
class G4UIcmdWithAnInteger;
class G4UIdirectory;

namespace Mustard::inline Extension::Geant4X::inline DecayChannel {

class MuonInternalPairProductionDecayChannel;

class MuonInternalPairProductionDecayChannelMessenger final : public Geant4X::SingletonMessenger<MuonInternalPairProductionDecayChannelMessenger,
                                                                                                 MuonInternalPairProductionDecayChannel> {
    friend class Env::Memory::SingletonInstantiator;

private:
    MuonInternalPairProductionDecayChannelMessenger();
    ~MuonInternalPairProductionDecayChannelMessenger();

public:
    auto SetNewValue(G4UIcommand* command, G4String value) -> void override;

private:
    std::unique_ptr<G4UIdirectory> fDirectory;
    std::unique_ptr<G4UIcmdWithADouble> fMetropolisDelta;
    std::unique_ptr<G4UIcmdWithAnInteger> fMetropolisDiscard;
};

} // namespace Mustard::inline Extension::Geant4X::inline DecayChannel
