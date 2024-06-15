#pragma once

#include "Mustard/Extension/Geant4X/Physics/DecayPhysicsBase.h++"
#include "Mustard/Extension/Geant4X/Physics/MuoniumTwoBodyDecayPhysicsMessenger.h++"

#include "muc/math"

#include "gsl/gsl"

class G4DecayTable;
class G4String;

namespace Mustard::inline Extension::Geant4X::inline Physics {

class MuoniumTwoBodyDecayPhysics : public DecayPhysicsBase {

public:
    MuoniumTwoBodyDecayPhysics(G4int verbose);

    auto RadiativeDecayBR(double br) -> void { fRadiativeDecayBR = muc::clamp<"[]">(br, 0., 1.); }
    auto IPPDecayBR(double br) -> void { fIPPDecayBR = muc::clamp<"[]">(br, 0., 1.); }
    auto AnnihilationDecayBR(double br) -> void { fAnnihilationDecayBR = muc::clamp<"[]">(br, 0., 1.); }
    auto M2eeDecayBR(double br) -> void { fM2eeDecayBR = muc::clamp<"[]">(br, 0., 1.); }

    virtual auto ConstructParticle() -> void override;
    virtual auto ConstructProcess() -> void override;
    virtual auto UpdateDecayBR() -> void override;

protected:
    virtual auto InsertDecayChannel(const G4String& parentName, gsl::not_null<G4DecayTable*> decay) -> void override;
    virtual auto AssignRareDecayBR(gsl::not_null<G4DecayTable*> decay) -> void override;

protected:
    double fRadiativeDecayBR;
    double fIPPDecayBR;
    double fAnnihilationDecayBR;
    double fM2eeDecayBR;

private:
    MuoniumTwoBodyDecayPhysicsMessenger::Register<MuoniumTwoBodyDecayPhysics> fMessengerRegister;
};

} // namespace Mustard::inline Extension::Geant4X::inline Physics
