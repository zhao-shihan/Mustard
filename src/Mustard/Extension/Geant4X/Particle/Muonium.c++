#include "Mustard/Extension/Geant4X/DecayChannel/MuoniumDecayChannel.h++"
#include "Mustard/Extension/Geant4X/Particle/Muonium.h++"
#include "Mustard/Utility/PhysicalConstant.h++"

#include "G4DecayTable.hh"
#include "G4ParticleTable.hh"

namespace Mustard::inline Extension::Geant4X::inline Particle {

using namespace PhysicalConstant;

Muonium::Muonium() :
    Singleton{},
    G4ParticleDefinition{"muonium",                      // name
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
                         990013111,                      // PDG encoding
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
    SetPDGMagneticMoment(muBohrM);

    // create Decay Table
    const auto table{new G4DecayTable};
    // create a decay channel
    table->Insert(new MuoniumDecayChannel{"muonium", 1});
    SetDecayTable(table);
}

} // namespace Mustard::inline Extension::Geant4X::inline Particle
