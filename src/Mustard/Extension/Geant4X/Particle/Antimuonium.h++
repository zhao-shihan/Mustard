#pragma once

#include "Mustard/Env/Memory/Singleton.h++"

#include "G4ParticleDefinition.hh"

namespace Mustard::inline Extension::Geant4X::inline Particle {

class Antimuonium final : public Env::Memory::Singleton<Antimuonium>,
                          public G4ParticleDefinition {
    friend Env::Memory::SingletonInstantiator;

private:
    Antimuonium();

public:
    static auto Definition() -> auto { return &Instance(); }
};

} // namespace Mustard::inline Extension::Geant4X::inline Particle
