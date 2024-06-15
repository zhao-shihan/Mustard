#include "Mustard/Extension/Geant4X/Physics/MuoniumTwoBodyDecayPhysics.h++"
#include "Mustard/Extension/Geant4X/Physics/MuoniumTwoBodyDecayPhysicsMessenger.h++"

#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcommand.hh"
#include "G4UIdirectory.hh"

namespace Mustard::inline Extension::Geant4X::inline Physics {

MuoniumTwoBodyDecayPhysicsMessenger::MuoniumTwoBodyDecayPhysicsMessenger() :
    SingletonMessenger{},
    fDirectory{},
    fAnnihilationDecayBR{},
    fM2eeDecayBR{},
    fUpdateDecayBR{} {

    fDirectory = std::make_unique<G4UIdirectory>("/Mustard/Physics/MuoniumDecay/");
    fDirectory->SetGuidance("About muon(ium) decay channel and decay generators.");

    fAnnihilationDecayBR = std::make_unique<G4UIcmdWithADouble>("/Mustard/Physics/MuoniumDecay/AnnihilationDecay/BR", this);
    fAnnihilationDecayBR->SetGuidance("Set branching ratio for muonium annihilation decay channel.");
    fAnnihilationDecayBR->SetParameterName("BR", false);
    fAnnihilationDecayBR->SetRange("0 <= BR && BR <= 1");
    fAnnihilationDecayBR->AvailableForStates(G4State_PreInit, G4State_Idle);

    fM2eeDecayBR = std::make_unique<G4UIcmdWithADouble>("/Mustard/Physics/MuoniumDecay/M2eeDecay/BR", this);
    fM2eeDecayBR->SetGuidance("Set branching ratio for muonium two-body decay channel.");
    fM2eeDecayBR->SetParameterName("BR", false);
    fM2eeDecayBR->SetRange("0 <= BR && BR <= 1");
    fM2eeDecayBR->AvailableForStates(G4State_PreInit, G4State_Idle);

    fUpdateDecayBR = std::make_unique<G4UIcmdWithoutParameter>("/Mustard/Physics/MuoniumDecay/UpdateDecayBR", this);
    fUpdateDecayBR->SetGuidance("Update decay branching ratio.");
    fUpdateDecayBR->AvailableForStates(G4State_Idle);
}

MuoniumTwoBodyDecayPhysicsMessenger::~MuoniumTwoBodyDecayPhysicsMessenger() = default;

auto MuoniumTwoBodyDecayPhysicsMessenger::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fAnnihilationDecayBR.get()) {
        Deliver<MuoniumTwoBodyDecayPhysics>([&](auto&& r) {
            r.AnnihilationDecayBR(fAnnihilationDecayBR->GetNewDoubleValue(value));
        });
    } else if (command == fM2eeDecayBR.get()) {
        Deliver<MuoniumTwoBodyDecayPhysics>([&](auto&& r) {
            r.M2eeDecayBR(fM2eeDecayBR->GetNewDoubleValue(value));
        });
    } else if (command == fUpdateDecayBR.get()) {
        Deliver<MuoniumTwoBodyDecayPhysics>([&](auto&& r) {
            r.UpdateDecayBR();
        });
    }
}

} // namespace Mustard::inline Extension::Geant4X::inline Physics
