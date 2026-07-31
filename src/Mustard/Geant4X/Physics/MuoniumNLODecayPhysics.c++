// -*- C++ -*-
//
// Copyright (C) 2020-2026  Shihan Zhao and contributors
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

#include "Mustard/Geant4X/DecayChannel/MuoniumDecayChannelWithSpin.h++"
#include "Mustard/Geant4X/DecayChannel/MuoniumRadiativeDecayChannelWithSpin.h++"
#include "Mustard/Geant4X/Particle/OrthoAntimuonium.h++"
#include "Mustard/Geant4X/Particle/OrthoMuonium.h++"
#include "Mustard/Geant4X/Particle/ParaAntimuonium.h++"
#include "Mustard/Geant4X/Particle/ParaMuonium.h++"
#include "Mustard/Geant4X/Physics/MuonNLODecayPhysics.h++"
#include "Mustard/Geant4X/Physics/MuoniumNLODecayPhysics.h++"

#include "G4DecayTable.hh"
#include "G4DecayWithSpin.hh"
#include "G4EmBuilder.hh"
#include "G4ProcessTable.hh"

namespace Mustard::Geant4X::inline Physics {

MuoniumNLODecayPhysics::MuoniumNLODecayPhysics(int verbose) :
    DecayPhysicsBase{"MuoniumNLODecayPhysics"},
    fRadiativeDecayBR{0.014},
    fMessengerRegister{this} {
    verboseLevel = verbose;
}

auto MuoniumNLODecayPhysics::UpdateDecayBR() -> void {
    UpdateDecayBRFor(ParaMuonium::Definition());
    UpdateDecayBRFor(OrthoMuonium::Definition());
    UpdateDecayBRFor(ParaAntimuonium::Definition());
    UpdateDecayBRFor(OrthoAntimuonium::Definition());
}

auto MuoniumNLODecayPhysics::ResetDecayBR() -> void {
    ResetDecayBRFor(ParaMuonium::Definition());
    ResetDecayBRFor(OrthoMuonium::Definition());
    ResetDecayBRFor(ParaAntimuonium::Definition());
    ResetDecayBRFor(OrthoAntimuonium::Definition());
}

auto MuoniumNLODecayPhysics::ConstructParticle() -> void {
    G4EmBuilder::ConstructMinimalEmSet();

    const auto NewDecayTableFor{
        [this](G4ParticleDefinition* muonium) {
            const auto decay{new G4DecayTable};
            InsertDecayChannel(muonium->GetParticleName(), decay);
            delete muonium->GetDecayTable();
            muonium->SetDecayTable(decay);
        }};
    NewDecayTableFor(ParaMuonium::Definition());
    NewDecayTableFor(OrthoMuonium::Definition());
    NewDecayTableFor(ParaAntimuonium::Definition());
    NewDecayTableFor(OrthoAntimuonium::Definition());

    UpdateDecayBR(); // set BR here
}

auto MuoniumNLODecayPhysics::ConstructProcess() -> void {
    const auto ReplaceDecayPhysics{
        [decayWithSpin = new G4DecayWithSpin,
         processTable = G4ProcessTable::GetProcessTable()](G4ParticleDefinition* muonium) {
            const auto manager{muonium->GetProcessManager()};
            if (manager == nullptr) {
                return;
            }
            const auto decay{processTable->FindProcess("Decay", muonium)};
            if (decay) {
                manager->RemoveProcess(decay);
            }
            manager->AddProcess(decayWithSpin);
            // set ordering for PostStepDoIt and AtRestDoIt
            manager->SetProcessOrdering(decayWithSpin, idxPostStep);
            manager->SetProcessOrdering(decayWithSpin, idxAtRest);
        }};
    ReplaceDecayPhysics(ParaMuonium::Definition());
    ReplaceDecayPhysics(OrthoMuonium::Definition());
    ReplaceDecayPhysics(ParaAntimuonium::Definition());
    ReplaceDecayPhysics(OrthoAntimuonium::Definition());
}

auto MuoniumNLODecayPhysics::InsertDecayChannel(const G4String& parentName, gsl::not_null<G4DecayTable*> decay) -> void {
    if (parentName != "para_muonium" and parentName != "ortho_muonium" and
        parentName != "para_anti_muonium" and parentName != "ortho_anti_muonium") {
        Throw<std::invalid_argument>(fmt::format("Parent should be a muonium-family particle, got '{}'.", parentName));
    }
    // sort by initial BR! we firstly write random BRs in decrease order...
    decay->Insert(new MuoniumDecayChannelWithSpin{parentName, 1e-1, verboseLevel});
    decay->Insert(new MuoniumRadiativeDecayChannelWithSpin{parentName, 1e-2, verboseLevel});
}

auto MuoniumNLODecayPhysics::ResetMinorDecayBR() -> void {
    // reset BR here
    fRadiativeDecayBR = 0.014;
}

auto MuoniumNLODecayPhysics::AssignMinorDecayBR(gsl::not_null<G4DecayTable*> decay) -> void {
    // set BR here
    decay->GetDecayChannel(1)->SetBR(fRadiativeDecayBR);
}

} // namespace Mustard::Geant4X::inline Physics
