#pragma once

#include "Mustard/Extension/Geant4X/Interface/SingletonMessenger.h++"

#include <memory>

class G4UIcmdWithABool;
class G4UIcmdWithAnInteger;
class G4UIcommand;
class G4UIdirectory;

namespace Mustard::inline Extension::Geant4X::inline Run {

class MPIRunManager;

class MPIRunMessenger final : public Geant4X::SingletonMessenger<MPIRunMessenger,
                                                                 MPIRunManager> {
    friend Env::Memory::SingletonInstantiator;

private:
    MPIRunMessenger();
    ~MPIRunMessenger();

public:
    auto SetNewValue(G4UIcommand* command, G4String value) -> void override;

private:
    std::unique_ptr<G4UIdirectory> fDirectory;
    std::unique_ptr<G4UIcmdWithABool> fPrintProgress;
    std::unique_ptr<G4UIcmdWithAnInteger> fPrintProgressModulo;
    std::unique_ptr<G4UIcommand> fPrintRunSummary;
};

} // namespace Mustard::inline Extension::Geant4X::inline Run
