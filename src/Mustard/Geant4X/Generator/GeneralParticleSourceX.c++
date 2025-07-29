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

#include "Mustard/Geant4X/Generator/GeneralParticleSourceX.h++"

#include "G4Event.hh"
#include "G4PrimaryVertex.hh"
#include "Randomize.hh"

namespace Mustard::Geant4X::inline Generator {

GeneralParticleSourceX::GeneralParticleSourceX() :
    G4GeneralParticleSource{},
    fNVertex{1},
    fPulseWidth{},
    fMessengerRegister{this} {}

auto GeneralParticleSourceX::GeneratePrimaryVertex(G4Event* event) -> void {
    auto& rand{*G4Random::getTheEngine()};
    const auto t0{GetParticleTime()};
    for (int i{}; i < fNVertex; ++i) {
        SetParticleTime(t0 + rand.flat() * fPulseWidth);
        G4GeneralParticleSource::GeneratePrimaryVertex(event);
    }
    SetParticleTime(t0);
}

} // namespace Mustard::Geant4X::inline Generator
