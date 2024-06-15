#pragma once

#include "Mustard/Utility/NonMoveableBase.h++"

#include "G4VPhysicsConstructor.hh"

#include "gsl/gsl"

class G4DecayTable;
class G4ParticleDefinition;
class G4String;

namespace Mustard::inline Extension::Geant4X::inline Physics {

class DecayPhysicsBase : public NonMoveableBase,
                         public G4VPhysicsConstructor {
public:
    using G4VPhysicsConstructor::G4VPhysicsConstructor;

    virtual auto UpdateDecayBR() -> void = 0;
    virtual auto ConstructParticle() -> void = 0;
    virtual auto ConstructProcess() -> void = 0;

protected:
    auto UpdateDecayBRFor(const G4ParticleDefinition* particle) -> void;
    virtual auto InsertDecayChannel(const G4String& parentName, gsl::not_null<G4DecayTable*> decay) -> void = 0;
    virtual auto AssignRareDecayBR(gsl::not_null<G4DecayTable*> decay) -> void = 0;
};

} // namespace Mustard::inline Extension::Geant4X::inline Physics
