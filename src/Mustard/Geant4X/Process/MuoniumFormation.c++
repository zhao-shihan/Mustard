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

#include "Mustard/Geant4X/Process/MuoniumFormation.h++"

#include "G4DynamicParticle.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4VPhysicalVolume.hh"

#include "fmt/format.h"

namespace Mustard::Geant4X::inline Process {

MuoniumFormation::MuoniumFormation() :
    G4VRestProcess{"MuoniumFormation", fUserDefined},
    fDefaultFormationProbability{0.5},
    fConversionProbability{},
    fParticleChange{},
    fMessengerRegister{this} {
    pParticleChange = &fParticleChange;
}

auto MuoniumFormation::DefaultFormationProbability(double p) -> void {
    if (p < 0 or p > 1) [[unlikely]] {
        PrintError(fmt::format("Invalid probability (got {}), not setting it.", p));
        return;
    }
    fDefaultFormationProbability = p;
}

auto MuoniumFormation::IsApplicable(const G4ParticleDefinition& particle) -> G4bool {
    return &particle == G4MuonPlus::Definition();
}

auto MuoniumFormation::AtRestDoIt(const G4Track& track, const G4Step&) -> G4VParticleChange* {
    using namespace PhysicalConstant;

    fParticleChange.Initialize(track);
    // The dynamic particle
    const auto muoniumDynamicParticle{new G4DynamicParticle{*track.GetDynamicParticle()}};
    // Determine whether the transition can be observed
    auto& rng{*G4Random::getTheEngine()};
    const auto conversion{rng.flat() < fConversionProbability};
    // Use antimuonium if it will be a conversion... not exactly true since it is not pure antimuonium but mix state. OK for now though.
    // FIXME: Try always using muonium but behave like mix state in each interaction.
    muoniumDynamicParticle->SetDefinition(conversion ? static_cast<G4ParticleDefinition*>(Geant4X::Antimuonium::Definition()) :
                                                       static_cast<G4ParticleDefinition*>(Geant4X::Muonium::Definition()));
    // Must pre-assign the decay time to ensure correct behaviour of transport and decay
    // (transport process use this to determine when to stop flight,
    //  instead of relying on G4 tracking mechanism. See MuoniumThermalScattering process for detail.)
    if (conversion) {
        // use the muonium conversion time spectrum (prop. to t^2*exp(-t/tau))
        const auto [tStar, converged]{muc::find_root::secant(
            // CDF - x
            [x = rng.flat()](const auto t) {
                const auto cdf{1 - std::exp(-t) * muc::polynomial({2, 2, 1}, t) / 2};
                return cdf - x;
            },
            // most probable t*
            2.)};
        if (not converged) {
            PrintError("Antimuonium decay time disconverged.");
        }
        muoniumDynamicParticle->SetPreAssignedDecayProperTime(tStar * muonium_lifetime);
    } else {
        // use standard exp decay
        muoniumDynamicParticle->SetPreAssignedDecayProperTime(G4RandExponential::shoot(&rng, muonium_lifetime));
    }
    // Sampling momentum according to boltzmann distribution
    const auto temperature{track.GetVolume()->GetLogicalVolume()->GetMaterial()->GetTemperature()}; // clang-format off
    const auto momentum{std::sqrt(muonium_mass_c2 * k_Boltzmann * temperature) *
                        G4ThreeVector{G4RandGauss::shoot(&rng),
                                      G4RandGauss::shoot(&rng),
                                      G4RandGauss::shoot(&rng)}}; // clang-format on
    // Set momentum and energy
    muoniumDynamicParticle->SetMomentum(momentum);
    muoniumDynamicParticle->SetKineticEnergy(momentum.mag2() / (2 * muonium_mass_c2));
    // Kill the muon, form the (anti-)muonium
    fParticleChange.ProposeTrackStatus(fStopAndKill);
    fParticleChange.SetNumberOfSecondaries(1);
    fParticleChange.AddSecondary(muoniumDynamicParticle);
    // Clean
    ClearNumberOfInteractionLengthLeft();
    return &fParticleChange;
}

auto MuoniumFormation::GetMeanLifeTime(const G4Track& track, G4ForceCondition*) -> double {
    auto formationProb{fDefaultFormationProbability};
    if (const auto mpt{track.GetMaterial()->GetMaterialPropertiesTable()};
        mpt and mpt->ConstPropertyExists("MUONIUM_FORMATION_PROB")) {
        formationProb = mpt->GetConstProperty("MUONIUM_FORMATION_PROB");
    }
    return G4UniformRand() < formationProb ? 0 : std::numeric_limits<double>::max();
}

} // namespace Mustard::Geant4X::inline Process
