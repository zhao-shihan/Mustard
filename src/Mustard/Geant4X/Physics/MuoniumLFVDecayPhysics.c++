// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

#include "Mustard/Geant4X/DecayChannel/MuoniumNeutrinolessDoubleRadiativeDecayChannel.h++"
#include "Mustard/Geant4X/Physics/MuoniumLFVDecayPhysics.h++"

#include "G4DecayTable.hh"
#include "G4PhaseSpaceDecayChannel.hh"

namespace Mustard::Geant4X::inline Physics {

MuoniumLFVDecayPhysics::MuoniumLFVDecayPhysics(G4int verbose) :
    MuoniumNLODecayPhysics{verbose},
    fDoubleRadiativeDecayBR{},
    fAnnihilativeDecayBR{},
    fElectronPairDecayBR{},
    fMuonLFVDecayPhysicsMessengerRegister{this},
    fMuoniumLFVDecayPhysicsMessengerRegister{this} {}

auto MuoniumLFVDecayPhysics::InsertDecayChannel(const G4String& parentName, gsl::not_null<G4DecayTable*> decay) -> void {
    MuoniumNLODecayPhysics::InsertDecayChannel(parentName, decay);
    decay->Insert(new MuoniumNeutrinolessDoubleRadiativeDecayChannel{parentName, 1e-3, verboseLevel});
    decay->Insert(new G4PhaseSpaceDecayChannel{parentName, 1e-4, 2, "gamma", "gamma"});
    decay->Insert(new G4PhaseSpaceDecayChannel{parentName, 1e-5, 2, "e+", "e-"});
}

auto MuoniumLFVDecayPhysics::ResetMinorDecayBR() -> void {
    MuoniumNLODecayPhysics::ResetMinorDecayBR();
    fDoubleRadiativeDecayBR = 0;
    fAnnihilativeDecayBR = 0;
    fElectronPairDecayBR = 0;
}

auto MuoniumLFVDecayPhysics::AssignMinorDecayBR(gsl::not_null<G4DecayTable*> decay) -> void {
    MuoniumNLODecayPhysics::AssignMinorDecayBR(decay);
    decay->GetDecayChannel(2)->SetBR(fDoubleRadiativeDecayBR);
    decay->GetDecayChannel(3)->SetBR(fAnnihilativeDecayBR);
    decay->GetDecayChannel(4)->SetBR(fElectronPairDecayBR);
}

} // namespace Mustard::Geant4X::inline Physics
