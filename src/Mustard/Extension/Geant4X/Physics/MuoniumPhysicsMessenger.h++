#pragma once

#include "Mustard/Extension/Geant4X/Interface/SingletonMessenger.h++"
#include "Mustard/Extension/Geant4X/Physics/TargetForMuoniumPhysics.h++"

#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIdirectory.hh"
#include "G4UImessenger.hh"

#include <memory>

namespace Mustard::inline Extension::Geant4X::inline Physics {

inline namespace Process {

template<TargetForMuoniumPhysics ATarget>
class MuoniumFormation;

template<TargetForMuoniumPhysics ATarget>
class MuoniumTransport;

} // namespace Process

template<TargetForMuoniumPhysics ATarget>
class MuoniumPhysicsMessenger final : public Geant4X::SingletonMessenger<MuoniumPhysicsMessenger<ATarget>,
                                                                         MuoniumFormation<ATarget>,
                                                                         MuoniumTransport<ATarget>> {
    friend Env::Memory::SingletonInstantiator;

private:
    MuoniumPhysicsMessenger();

public:
    auto SetNewValue(G4UIcommand* command, G4String value) -> void override;

private:
    std::unique_ptr<G4UIdirectory> fMuoniumPhysicsDirectory;

    std::unique_ptr<G4UIdirectory> fFormationProcessDirectory;
    std::unique_ptr<G4UIcmdWithADouble> fConversionProbability;

    std::unique_ptr<G4UIdirectory> fTransportProcessDirectory;
    std::unique_ptr<G4UIcmdWithABool> fManipulateAllSteps;
};

} // namespace Mustard::inline Extension::Geant4X::inline Physics

#include "Mustard/Extension/Geant4X/Physics/MuoniumPhysicsMessenger.inl"
