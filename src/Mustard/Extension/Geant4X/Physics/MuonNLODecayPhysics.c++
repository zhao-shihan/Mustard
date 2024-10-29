// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
//
// This file is part of Mustard, an offline software framework for HEP experiments.
//
// Mustard is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// Mustard is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// Mustard. If not, see <https://www.gnu.org/licenses/>.

#include "Mustard/Extension/Geant4X/Decay/ExtendedDecayWithSpin.h++"
#include "Mustard/Extension/Geant4X/DecayChannel/MuonInternalConversionDecayChannel.h++"
#include "Mustard/Extension/Geant4X/Physics/MuonNLODecayPhysics.h++"

#include "G4DecayTable.hh"
#include "G4EmBuilder.hh"
#include "G4MuonDecayChannelWithSpin.hh"
#include "G4MuonMinus.hh"
#include "G4MuonPlus.hh"
#include "G4MuonRadiativeDecayChannelWithSpin.hh"
#include "G4ProcessTable.hh"

#include <limits>
#include <stdexcept>

namespace Mustard::inline Extension::Geant4X::inline Physics {

MuonNLODecayPhysics::MuonNLODecayPhysics(G4int verbose) :
    DecayPhysicsBase{"MuonNLODecayPhysics"},
    fRadiativeDecayBR{0.014},
    fICDecayBR{3.6054e-5}, // QED leading-order
    fMessengerRegister{this} {
    verboseLevel = verbose;
}

auto MuonNLODecayPhysics::UpdateDecayBR() -> void {
    UpdateDecayBRFor(G4MuonPlus::Definition());
    UpdateDecayBRFor(G4MuonMinus::Definition());
}

auto MuonNLODecayPhysics::ResetDecayBR() -> void {
    ResetDecayBRFor(G4MuonPlus::Definition());
    ResetDecayBRFor(G4MuonMinus::Definition());
}

auto MuonNLODecayPhysics::ConstructParticle() -> void {
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
}

auto MuonNLODecayPhysics::ConstructProcess() -> void {
    const auto ReplaceMuonDecayPhysics{
        [decayWithSpin = new ExtendedDecayWithSpin,
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
}

auto MuonNLODecayPhysics::InsertDecayChannel(const G4String& parentName, gsl::not_null<G4DecayTable*> decay) -> void {
    // sort by initial BR! we firstly write random BRs in decrease order...
    decay->Insert(new G4MuonDecayChannelWithSpin{parentName, 1e-1}), decay->GetDecayChannel(0)->SetVerboseLevel(verboseLevel);
    decay->Insert(new G4MuonRadiativeDecayChannelWithSpin{parentName, 1e-2}), decay->GetDecayChannel(0)->SetVerboseLevel(verboseLevel);
    decay->Insert(new MuonInternalConversionDecayChannel{parentName, 1e-3, verboseLevel});
}

auto MuonNLODecayPhysics::AssignMinorDecayBR(gsl::not_null<G4DecayTable*> decay) -> void {
    // set BR here
    decay->GetDecayChannel(1)->SetBR(fRadiativeDecayBR);
    decay->GetDecayChannel(2)->SetBR(fICDecayBR);
}

auto MuonNLODecayPhysics::ResetMinorDecayBR(gsl::not_null<G4DecayTable*> decay) -> void {
    // reset BR here
    decay->GetDecayChannel(1)->SetBR(0.014);
    decay->GetDecayChannel(2)->SetBR(3.6054e-5);
}

} // namespace Mustard::inline Extension::Geant4X::inline Physics
