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

#include "Mustard/Extension/Geant4X/DecayChannel/MuoniumDecayChannelWithSpin.h++"
#include "Mustard/Extension/Geant4X/DecayChannel/MuoniumInternalConversionDecayChannel.h++"
#include "Mustard/Extension/Geant4X/DecayChannel/MuoniumRadiativeDecayChannelWithSpin.h++"
#include "Mustard/Extension/Geant4X/Particle/Muonium.h++"
#include "Mustard/Extension/Geant4X/Physics/MuoniumNLODecayPhysics.h++"
#include "Mustard/Extension/Geant4X/Physics/MuoniumSMAndLFVDecayPhysics.h++"

#include "G4DecayTable.hh"
#include "G4PhaseSpaceDecayChannel.hh"

namespace Mustard::inline Extension::Geant4X::inline Physics {

MuoniumSMAndLFVDecayPhysics::MuoniumSMAndLFVDecayPhysics(G4int verbose) :
    MuoniumNLODecayPhysics{verbose},
    fDoubleRadiativeDecayBR{},
    fElectronPairDecayBR{},
    fMessengerRegister{this} {}

auto MuoniumSMAndLFVDecayPhysics::InsertDecayChannel(const G4String& parentName, gsl::not_null<G4DecayTable*> decay) -> void {
    MuoniumNLODecayPhysics::InsertDecayChannel(parentName, decay);
    decay->Insert(new G4PhaseSpaceDecayChannel{parentName, 1e-4, 2, "gamma", "gamma"});
    decay->Insert(new G4PhaseSpaceDecayChannel{parentName, 1e-5, 2, "e+", "e-"});
}

auto MuoniumSMAndLFVDecayPhysics::AssignRareDecayBR(gsl::not_null<G4DecayTable*> decay) -> void {
    MuoniumNLODecayPhysics::AssignRareDecayBR(decay);
    decay->GetDecayChannel(3)->SetBR(fDoubleRadiativeDecayBR);
    decay->GetDecayChannel(4)->SetBR(fElectronPairDecayBR);
}

} // namespace Mustard::inline Extension::Geant4X::inline Physics
