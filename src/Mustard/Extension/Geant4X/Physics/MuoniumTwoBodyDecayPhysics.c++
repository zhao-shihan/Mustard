#include "Mustard/Extension/Geant4X/DecayChannel/MuoniumDecayChannelWithSpin.h++"
#include "Mustard/Extension/Geant4X/DecayChannel/MuoniumInternalPairProductionDecayChannel.h++"
#include "Mustard/Extension/Geant4X/DecayChannel/MuoniumRadiativeDecayChannelWithSpin.h++"
#include "Mustard/Extension/Geant4X/Particle/Antimuonium.h++"
#include "Mustard/Extension/Geant4X/Particle/Muonium.h++"
#include "Mustard/Extension/Geant4X/Physics/MuoniumTwoBodyDecayPhysics.h++"

#include "G4DecayTable.hh"
#include "G4DecayWithSpin.hh"
#include "G4EmBuilder.hh"
#include "G4PhaseSpaceDecayChannel.hh"
#include "G4ProcessTable.hh"

namespace Mustard::inline Extension::Geant4X::inline Physics {

MuoniumTwoBodyDecayPhysics::MuoniumTwoBodyDecayPhysics(G4int verbose) :
    DecayPhysicsBase{"MuoniumTwoBodyDecayPhysics", verbose},
    fRadiativeDecayBR{},
    fIPPDecayBR{},
    fAnnihilationDecayBR{},
    fM2eeDecayBR{},
    fMessengerRegister{this} {}

auto MuoniumTwoBodyDecayPhysics::ConstructParticle() -> void {
    G4EmBuilder::ConstructMinimalEmSet();

    const auto NewDecayTableFor{
        [this](G4ParticleDefinition* muonium) {
            const auto decay{new G4DecayTable};
            InsertDecayChannel(muonium->GetParticleName(), decay);
            delete muonium->GetDecayTable();
            muonium->SetDecayTable(decay);
        }};
    NewDecayTableFor(Muonium::Definition());
    NewDecayTableFor(Antimuonium::Definition());

    UpdateDecayBR(); // set BR here
}

auto MuoniumTwoBodyDecayPhysics::ConstructProcess() -> void {
    const auto ReplaceDecayPhysics{
        [decayWithSpin = new G4DecayWithSpin,
         processTable = G4ProcessTable::GetProcessTable()](G4ParticleDefinition* muonium) {
            const auto manager{muonium->GetProcessManager()};
            if (manager == nullptr) { return; }
            const auto decay{processTable->FindProcess("Decay", muonium)};
            if (decay) { manager->RemoveProcess(decay); }
            manager->AddProcess(decayWithSpin);
            // set ordering for PostStepDoIt and AtRestDoIt
            manager->SetProcessOrdering(decayWithSpin, idxPostStep);
            manager->SetProcessOrdering(decayWithSpin, idxAtRest);
        }};
    ReplaceDecayPhysics(Muonium::Definition());
    ReplaceDecayPhysics(Antimuonium::Definition());
}

auto MuoniumTwoBodyDecayPhysics::UpdateDecayBR() -> void {
    UpdateDecayBRFor(Muonium::Definition());
    UpdateDecayBRFor(Antimuonium::Definition());
}

auto MuoniumTwoBodyDecayPhysics::InsertDecayChannel(const G4String& parentName, gsl::not_null<G4DecayTable*> decay) -> void {
    // sort by initial BR! we firstly write random BRs in decrease order...
    decay->Insert(new MuoniumDecayChannelWithSpin{parentName, 1e-1, verboseLevel});
    decay->Insert(new MuoniumRadiativeDecayChannelWithSpin{parentName, 1e-2, verboseLevel});
    decay->Insert(new MuoniumInternalPairProductionDecayChannel{parentName, 1e-3, verboseLevel});
    decay->Insert(new G4PhaseSpaceDecayChannel{parentName, 1e-4, 2, "gamma", "gamma"});
    decay->Insert(new G4PhaseSpaceDecayChannel{parentName, 1e-5, 2, "e+", "e-"});
}

auto MuoniumTwoBodyDecayPhysics::AssignRareDecayBR(gsl::not_null<G4DecayTable*> decay) -> void {
    // set BR here
    decay->GetDecayChannel(1)->SetBR(fRadiativeDecayBR);
    decay->GetDecayChannel(2)->SetBR(fIPPDecayBR);
    decay->GetDecayChannel(3)->SetBR(fAnnihilationDecayBR);
    decay->GetDecayChannel(4)->SetBR(fM2eeDecayBR);
}

} // namespace Mustard::inline Extension::Geant4X::inline Physics
