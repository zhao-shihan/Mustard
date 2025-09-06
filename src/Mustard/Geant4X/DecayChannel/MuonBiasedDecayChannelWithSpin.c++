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

#include "Mustard/Geant4X/DecayChannel/MuonBiasedDecayChannelWithSpin.h++"
#include "Mustard/Utility/PhysicalConstant.h++"

#include "G4DecayProducts.hh"
#include "G4MuonDecayChannelWithSpin.hh"

namespace Mustard::Geant4X::inline DecayChannel {

using namespace PhysicalConstant;

MuonBiasedDecayChannelWithSpin::MuonBiasedDecayChannelWithSpin(const G4String& parentName, G4double br) :
    G4MuonDecayChannelWithSpin{parentName, br},
    fEnergyCut{0.},
    fMessengerRegister{this} {}

auto MuonBiasedDecayChannelWithSpin::DecayIt(G4double mass) -> G4DecayProducts* {
    while (true) {
        const auto products{G4MuonDecayChannelWithSpin::DecayIt(mass)};
        const auto positron{(*products)[0]};
        if (positron->GetKineticEnergy() > fEnergyCut) {
            return products;
        }
        delete products;
    }
}

} // namespace Mustard::Geant4X::inline DecayChannel
