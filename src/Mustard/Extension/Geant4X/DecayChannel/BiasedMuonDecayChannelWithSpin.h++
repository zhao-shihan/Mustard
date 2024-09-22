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

#pragma once

#include "G4MuonDecayChannelWithSpin.hh"

namespace Mustard::inline Extension::Geant4X::inline DecayChannel {

class BiasedMuonDecayChannelWithSpin : public G4MuonDecayChannelWithSpin {
public:
    BiasedMuonDecayChannelWithSpin(const G4String& parentName, G4double br, G4int verbose = 1);

    auto EnergyCut() const -> auto { return fEnergyCut; }
    auto EnergyCut(double cut) -> void { fEnergyCut = cut; }

    auto DecayIt(G4double) -> G4DecayProducts* override;

private:
    double fEnergyCut;
};

} // namespace Mustard::inline Extension::Geant4X::inline DecayChannel
