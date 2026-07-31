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

#include "Mustard/Geant4X/DecayChannel/MuoniumDecayChannel.h++"
#include "Mustard/Utility/PhysicalConstant.h++"

#include "G4DecayTable.hh"

namespace Mustard::Geant4X::inline Particle {

template<typename ADerived>
MuoniumBase<ADerived>::MuoniumBase(const G4String& name, int pdgEncoding, int iSpin, int magneticMomentSign) :
    Env::Singleton<ADerived>{},
    G4ParticleDefinition{name,                                                               // name
                         PhysicalConstant::muonium_mass_c2,                                  // mass
                         PhysicalConstant::hbar_Planck / PhysicalConstant::muonium_lifetime, // width
                         0,                                                                  // charge
                         iSpin,                                                              // spin (x2)
                         -1,                                                                 // parity
                         0,                                                                  // conjugation
                         0,                                                                  // isospin
                         0,                                                                  // isospin3
                         0,                                                                  // G parity
                         "lepton",                                                           // particle type
                         0,                                                                  // lepton number
                         0,                                                                  // baryon number
                         pdgEncoding,                                                        // PDG encoding
                         false,                                                              // stable
                         PhysicalConstant::muonium_lifetime,                                 // lifetime
                         nullptr,                                                            // decay table
                         false,                                                              // shortlived
                         "muonium"} {                                                        // subtype
    using namespace PhysicalConstant;
    constexpr auto muBohrMu{0.5 * eplus * hbar_Planck / (muon_mass_c2 / c_squared)};
    constexpr auto muBohrE{-0.5 * eplus * hbar_Planck / (electron_mass_c2 / c_squared)};
    constexpr auto muBohrM{1.0011659208 * muBohrMu + 1.0011596521859 * muBohrE};
    SetPDGMagneticMoment(magneticMomentSign * muBohrM);

    const auto table{new G4DecayTable};
    table->Insert(new MuoniumDecayChannel{name, 1});
    SetDecayTable(table);
}

} // namespace Mustard::Geant4X::inline Particle
