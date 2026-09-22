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

#include "Mustard/Geant4X/Management/Exception.h++"
#include "Mustard/Geant4X/Particle/OrthoMuonium.h++"
#include "Mustard/Geant4X/Particle/ParaMuonium.h++"
#include "Mustard/Geant4X/Process/MuoniumFormation.h++"
#include "Mustard/Math/Random/Distribution/Gaussian3D.h++"
#include "Mustard/Math/Vector.h++"

#include "G4DynamicParticle.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4VPhysicalVolume.hh"

#include "muc/math"
#include "muc/numeric"

#include "fmt/format.h"

#include <algorithm>
#include <limits>

namespace Mustard::Geant4X::inline Process {

MuoniumFormation::MuoniumFormation() :
    G4VRestProcess{"MuoniumFormation", fGeneral},
    fDefaultParaMuoniumFraction{0.25},
    fCurrentRun{},
    fFormationFractionCache{},
    fParaMuoniumFractionCache{},
    fParticleChange{},
    fMessengerRegister{this} {
    pParticleChange = &fParticleChange;
}

auto MuoniumFormation::DefaultParaMuoniumFraction(double val) -> void {
    if (val < 0 or 1 < val) [[unlikely]] {
        const auto clamped{std::clamp(val, 0., 1.)};
        Exception<JustWarning>(fmt::format(
            "Default para-muonium fraction is out of range [0, 1] (got {}), clamping to {}.", val, clamped));
        fDefaultParaMuoniumFraction = clamped;
        return;
    }
    fDefaultParaMuoniumFraction = val;
}

auto MuoniumFormation::IsApplicable(const G4ParticleDefinition& particle) -> bool {
    return &particle == G4MuonPlus::Definition();
}

auto MuoniumFormation::StartTracking(G4Track* track) -> void {
    if (const auto run{G4RunManager::GetRunManager()->GetCurrentRun()};
        fCurrentRun != std::pair{run, run->GetRunID()}) {
        fCurrentRun = {run, run->GetRunID()};
        fFormationFractionCache.clear();
        fParaMuoniumFractionCache.clear();
    }
    G4VRestProcess::StartTracking(track);
}

auto MuoniumFormation::AtRestDoIt(const G4Track& track, const G4Step&) -> G4VParticleChange* {
    using namespace PhysicalConstant;
    fParticleChange.Initialize(track);
    // Sampling velocity according to local temperature
    auto& rng{*G4Random::getTheEngine()};
    Vector3D direction;
    double kineticEnergy;
    do {
        direction = Random::Gaussian3D<Vector3D>{}(rng);
        kineticEnergy = direction.mag2();
    } while (kineticEnergy <= muc::pow(muc::default_abs_tol<double>, 2));
    direction *= 1 / std::sqrt(kineticEnergy); // do not use CLHEP::Hep3Vector::operator/=(double).
    kineticEnergy *= 0.5 * k_Boltzmann * track.GetMaterial()->GetTemperature();
    // The dynamic particle
    const auto muoniumDynamicParticle{new G4DynamicParticle{*track.GetDynamicParticle()}};
    if (rng.flat() < ParaMuoniumFractionIn(*track.GetMaterial())) {
        muoniumDynamicParticle->SetDefinition(ParaMuonium::Definition());
        muoniumDynamicParticle->SetPolarization({});
    } else {
        muoniumDynamicParticle->SetDefinition(OrthoMuonium::Definition());
    }
    muoniumDynamicParticle->SetMomentumDirection(direction);
    muoniumDynamicParticle->SetKineticEnergy(kineticEnergy);
    // Kill the muon, form the muonium
    fParticleChange.ProposeTrackStatus(fStopAndKill);
    fParticleChange.SetNumberOfSecondaries(1);
    fParticleChange.AddSecondary(muoniumDynamicParticle);
    ClearNumberOfInteractionLengthLeft();
    return &fParticleChange;
}

auto MuoniumFormation::FormationFractionIn(const G4Material& material) -> double {
    const auto mpt{material.GetMaterialPropertiesTable()};
    if (mpt == nullptr) {
        return 0;
    }
    if (const auto it{fFormationFractionCache.find(mpt)};
        it != fFormationFractionCache.end()) [[likely]] {
        return it->second;
    }
    constexpr auto propertyName{"MUONIUM_FORMATION_FRACTION"};
    if (not mpt->ConstPropertyExists(propertyName)) {
        fFormationFractionCache.emplace(mpt, 0);
        return 0;
    }
    const auto val{mpt->GetConstProperty(propertyName)};
    if (val < 0 or 1 < val) [[unlikely]] {
        const auto clamped{std::clamp(val, 0., 1.)};
        Exception<JustWarning>(fmt::format(
            "{} for material '{}' is out of range [0, 1] (got {}), clamping to {}.",
            propertyName, material.GetName(), val, clamped));
        fFormationFractionCache.emplace(mpt, clamped);
        return clamped;
    }
    fFormationFractionCache.emplace(mpt, val);
    return val;
}

auto MuoniumFormation::ParaMuoniumFractionIn(const G4Material& material) -> double {
    const auto mpt{material.GetMaterialPropertiesTable()};
    if (mpt == nullptr) {
        return fDefaultParaMuoniumFraction;
    }
    if (const auto it{fParaMuoniumFractionCache.find(mpt)};
        it != fParaMuoniumFractionCache.end()) [[likely]] {
        return it->second;
    }
    constexpr auto propertyName{"PARA_MUONIUM_FORMATION_FRACTION"};
    if (not mpt->ConstPropertyExists(propertyName)) {
        fParaMuoniumFractionCache.emplace(mpt, fDefaultParaMuoniumFraction);
        return fDefaultParaMuoniumFraction;
    }
    const auto val{mpt->GetConstProperty(propertyName)};
    if (val < 0 or 1 < val) [[unlikely]] {
        const auto clamped{std::clamp(val, 0., 1.)};
        Exception<JustWarning>(fmt::format(
            "{} for material '{}' is out of range [0, 1] (got {}), clamping to {}.",
            propertyName, material.GetName(), val, clamped));
        fParaMuoniumFractionCache.emplace(mpt, clamped);
        return clamped;
    }
    fParaMuoniumFractionCache.emplace(mpt, val);
    return val;
}

auto MuoniumFormation::GetMeanLifeTime(const G4Track& track, G4ForceCondition*) -> double {
    const auto prob{FormationFractionIn(*track.GetMaterial())};
    if (prob <= 0) {
        return std::numeric_limits<double>::max();
    }
    if (prob >= 1) {
        return 0;
    }
    return (G4UniformRand() >= prob) * std::numeric_limits<double>::max();
}

} // namespace Mustard::Geant4X::inline Process
