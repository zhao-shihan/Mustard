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

#include "Mustard/Extension/Geant4X/DecayChannel/BiasedMuonDecayChannelWithSpin.h++"
#include "Mustard/Utility/PhysicalConstant.h++"

#include "G4DecayProducts.hh"
#include "G4MuonDecayChannelWithSpin.hh"

namespace Mustard::inline Extension::Geant4X::inline DecayChannel {

using namespace PhysicalConstant;

BiasedMuonDecayChannelWithSpin::BiasedMuonDecayChannelWithSpin(const G4String& parentName, G4double br, G4int verbose) :
    G4MuonDecayChannelWithSpin{parentName, br},
    fEnergyCut{40.} {}

auto BiasedMuonDecayChannelWithSpin::DecayIt(G4double) -> G4DecayProducts* {

    G4DecayProducts* products;
    G4DynamicParticle* e;
    G4DynamicParticle* v1;
    G4DynamicParticle* v2;

    double positronEnergy{};

    while (positronEnergy < fEnergyCut) {
        products = G4MuonDecayChannelWithSpin::DecayIt(muon_mass_c2);
        v2 = products->PopProducts();
        v1 = products->PopProducts();
        e = products->PopProducts();

        positronEnergy = e->GetKineticEnergy();
    }

    products->PushProducts(e);
    products->PushProducts(v1);
    products->PushProducts(v2);

    return products;
}

} // namespace Mustard::inline Extension::Geant4X::inline DecayChannel
