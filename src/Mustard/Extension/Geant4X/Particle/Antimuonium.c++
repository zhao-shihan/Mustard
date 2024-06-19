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

#include "Mustard/Extension/Geant4X/DecayChannel/MuoniumDecayChannel.h++"
#include "Mustard/Extension/Geant4X/Particle/Antimuonium.h++"
#include "Mustard/Utility/PhysicalConstant.h++"

#include "G4DecayTable.hh"
#include "G4ParticleTable.hh"

namespace Mustard::inline Extension::Geant4X::inline Particle {

using namespace PhysicalConstant;

Antimuonium::Antimuonium() :
    Singleton{},
    G4ParticleDefinition{"anti_muonium",                 // name
                         muonium_mass_c2,                // mass
                         hbar_Planck / muonium_lifetime, // width
                         0,                              // charge
                         1,                              // spin
                         0,                              // parity
                         0,                              // conjugation
                         0,                              // isospin
                         0,                              // isospin3
                         0,                              // G parity
                         "lepton",                       // particle type
                         0,                              // lepton number
                         0,                              // baryon number
                         -990013111,                     // PDG encoding
                         false,                          // stable
                         muonium_lifetime,               // lifetime
                         nullptr,                        // decay table
                         false,                          // shortlived
                         "muonium"} {                    // subtype
    // Bohr magnetron of Muonium - T. Shiroka
    // The magnetic moment of Mu is the sum of those of mu+ and e- with
    // the respective gyromagnetic ratio anomalies as coefficients
    constexpr auto muBohrMu{0.5 * eplus * hbar_Planck / (muon_mass_c2 / c_squared)};
    constexpr auto muBohrE{-0.5 * eplus * hbar_Planck / (electron_mass_c2 / c_squared)};
    constexpr auto muBohrM{1.0011659208 * muBohrMu + 1.0011596521859 * muBohrE};
    SetPDGMagneticMoment(-muBohrM); // Negative for anti-muonium

    // create Decay Table
    const auto table{new G4DecayTable};
    // create a decay channel
    table->Insert(new MuoniumDecayChannel{"anti_muonium", 1});
    SetDecayTable(table);
}

} // namespace Mustard::inline Extension::Geant4X::inline Particle
