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

#include "Mustard/Geant4X/Track/ParticleChangeForThermalScattering.h++"

#include "G4Step.hh"
#include "G4StepPoint.hh"

#include "muc/math"

namespace Mustard::Geant4X::inline Track {

auto ParticleChangeForThermalScattering::Initialize(const G4Track& track) -> void {
    G4VParticleChange::Initialize(track);
    fMass = track.GetParticleDefinition()->GetPDGMass();
    fTimeIncrement = std::nullopt;
    const auto post{track.GetStep()->GetPostStepPoint()}; // already updated by G4Transportation
    fPosition = post->GetPosition();
    fVelocityDirection = post->GetMomentumDirection();
    fVelocity = post->GetVelocity();
}

auto ParticleChangeForThermalScattering::UpdateStepForAlongStep(G4Step* step) -> G4Step* {
    const auto post{step->GetPostStepPoint()};
    if (fTimeIncrement.has_value()) {
        const auto pre{step->GetPreStepPoint()};
        post->SetGlobalTime(pre->GetGlobalTime() + *fTimeIncrement);
        post->SetLocalTime(pre->GetLocalTime() + *fTimeIncrement);
        post->SetProperTime(pre->GetProperTime() + *fTimeIncrement);
    }
    post->SetPosition(fPosition);
    post->SetMomentumDirection(fVelocityDirection);
    post->SetVelocity(fVelocity);
    post->SetKineticEnergy(1. / 2 * fMass * muc::pow(fVelocity, 2));
    return G4VParticleChange::UpdateStepForAlongStep(step);
}

} // namespace Mustard::Geant4X::inline Track
