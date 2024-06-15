#pragma once

#include "Mustard/Extension/Geant4X/Interface/SingletonMessenger.h++"

#include <memory>

class G4UIcmdWithADouble;
class G4UIcmdWithoutParameter;
class G4UIdirectory;

namespace Mustard::inline Extension::Geant4X::inline Physics {

class MuoniumTwoBodyDecayPhysics;

class MuoniumTwoBodyDecayPhysicsMessenger final : public Geant4X::SingletonMessenger<MuoniumTwoBodyDecayPhysicsMessenger,
                                                                                     MuoniumTwoBodyDecayPhysics> {
    friend Env::Memory::SingletonInstantiator;

private:
    MuoniumTwoBodyDecayPhysicsMessenger();
    ~MuoniumTwoBodyDecayPhysicsMessenger();

public:
    auto SetNewValue(G4UIcommand* command, G4String value) -> void override;

private:
    std::unique_ptr<G4UIdirectory> fDirectory;
    std::unique_ptr<G4UIcmdWithADouble> fAnnihilationDecayBR;
    std::unique_ptr<G4UIcmdWithADouble> fM2eeDecayBR;
    std::unique_ptr<G4UIcmdWithoutParameter> fUpdateDecayBR;
};

} // namespace Mustard::inline Extension::Geant4X::inline Physics
