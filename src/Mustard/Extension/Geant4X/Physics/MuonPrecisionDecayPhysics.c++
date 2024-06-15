#include "Mustard/Extension/Geant4X/DecayChannel/MuonInternalPairProductionDecayChannel.h++"
#include "Mustard/Extension/Geant4X/Physics/MuonPrecisionDecayPhysics.h++"

#include "G4DecayTable.hh"
#include "G4DecayWithSpin.hh"
#include "G4EmBuilder.hh"
#include "G4MuonDecayChannelWithSpin.hh"
#include "G4MuonMinus.hh"
#include "G4MuonPlus.hh"
#include "G4MuonRadiativeDecayChannelWithSpin.hh"
#include "G4PionDecayMakeSpin.hh"
#include "G4PionMinus.hh"
#include "G4PionPlus.hh"
#include "G4ProcessTable.hh"

#include <limits>
#include <stdexcept>

namespace Mustard::inline Extension::Geant4X::inline Physics {

MuonPrecisionDecayPhysics::MuonPrecisionDecayPhysics(G4int verbose) :
    DecayPhysicsBase{"MuonPrecisionDecayPhysics", verbose},
    fRadiativeDecayBR{0.014},
    fIPPDecayBR{3.4e-5},
    fMessengerRegister{this} {}

auto MuonPrecisionDecayPhysics::ConstructParticle() -> void {
    G4EmBuilder::ConstructMinimalEmSet();

    const auto NewDecayTableFor{
        [this](G4ParticleDefinition* muon) {
            const auto decay{new G4DecayTable};
            InsertDecayChannel(muon->GetParticleName(), decay);
            delete muon->GetDecayTable();
            muon->SetDecayTable(decay);
        }};
    NewDecayTableFor(G4MuonPlus::Definition());
    NewDecayTableFor(G4MuonMinus::Definition());

    UpdateDecayBR(); // set BR here

    G4PionPlus::Definition();
    G4PionMinus::Definition();
}

auto MuonPrecisionDecayPhysics::ConstructProcess() -> void {
    const auto ReplaceMuonDecayPhysics{
        [decayWithSpin = new G4DecayWithSpin,
         processTable = G4ProcessTable::GetProcessTable()](G4ParticleDefinition* muon) {
            const auto manager{muon->GetProcessManager()};
            if (manager == nullptr) { return; }
            const auto decay{processTable->FindProcess("Decay", muon)};
            if (decay) { manager->RemoveProcess(decay); }
            manager->AddProcess(decayWithSpin);
            // set ordering for PostStepDoIt and AtRestDoIt
            manager->SetProcessOrdering(decayWithSpin, idxPostStep);
            manager->SetProcessOrdering(decayWithSpin, idxAtRest);
        }};
    ReplaceMuonDecayPhysics(G4MuonPlus::Definition());
    ReplaceMuonDecayPhysics(G4MuonMinus::Definition());

    const auto ReplacePionDecayPhysics{
        [decayMakeSpin = new G4PionDecayMakeSpin,
         processTable = G4ProcessTable::GetProcessTable()](G4ParticleDefinition* pion) {
            const auto manager{pion->GetProcessManager()};
            if (manager == nullptr) { return; }
            const auto decay{processTable->FindProcess("Decay", pion)};
            if (decay) { manager->RemoveProcess(decay); }
            manager->AddProcess(decayMakeSpin);
            // set ordering for PostStepDoIt and AtRestDoIt
            manager->SetProcessOrdering(decayMakeSpin, idxPostStep);
            manager->SetProcessOrdering(decayMakeSpin, idxAtRest);
        }};
    ReplacePionDecayPhysics(G4PionPlus::Definition());
    ReplacePionDecayPhysics(G4PionMinus::Definition());
}

auto MuonPrecisionDecayPhysics::UpdateDecayBR() -> void {
    UpdateDecayBRFor(G4MuonPlus::Definition());
    UpdateDecayBRFor(G4MuonMinus::Definition());
}

auto MuonPrecisionDecayPhysics::InsertDecayChannel(const G4String& parentName, gsl::not_null<G4DecayTable*> decay) -> void {
    // sort by initial BR! we firstly write random BRs in decrease order...
    decay->Insert(new G4MuonDecayChannelWithSpin{parentName, 1e-1}), decay->GetDecayChannel(0)->SetVerboseLevel(verboseLevel);
    decay->Insert(new G4MuonRadiativeDecayChannelWithSpin{parentName, 1e-2}), decay->GetDecayChannel(0)->SetVerboseLevel(verboseLevel);
    decay->Insert(new MuonInternalPairProductionDecayChannel{parentName, 1e-3, verboseLevel});
}

auto MuonPrecisionDecayPhysics::AssignRareDecayBR(gsl::not_null<G4DecayTable*> decay) -> void {
    // set BR here
    decay->GetDecayChannel(1)->SetBR(fRadiativeDecayBR);
    decay->GetDecayChannel(2)->SetBR(fIPPDecayBR);
}

} // namespace Mustard::inline Extension::Geant4X::inline Physics
