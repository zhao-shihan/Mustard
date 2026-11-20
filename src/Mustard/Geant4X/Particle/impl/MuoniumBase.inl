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

namespace Mustard::Geant4X::inline Particle::impl {

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
    // In weak-field limit, muon decay from muonium looks as if lost half of the polarization,
    // and have a gyromagnetic ratio of gamma_M = (gamma_e + gamma_mu) / 2.
    // Note that in G4 a neutral particle feels its magnetic moment in tracking only when
    // G4Transportation::EnableMagneticMoment() is enabled and the field manager uses a
    // spin equation of motion (G4Mag_SpinEqRhs); that equation takes |mu| and therefore
    // can only precess in the positive sense, whereas gamma_M is negative here.
    using namespace PhysicalConstant;
    SetPDGMagneticMoment(magneticMomentSign * muonium_magnetic_moment);

    const auto table{new G4DecayTable};
    table->Insert(new MuoniumDecayChannel{name, 1});
    SetDecayTable(table);
}

} // namespace Mustard::Geant4X::inline Particle
