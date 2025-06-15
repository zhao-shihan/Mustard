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

#include "Mustard/Extension/Geant4X/DecayChannel/DecayChannelExtension.h++"

#include "CLHEP/Units/SystemOfUnits.h"

#include "G4Decay.hh"
#include "G4DecayProducts.hh"
#include "G4DecayTable.hh"
#include "G4DynamicParticle.hh"
#include "G4ParticleChangeForDecay.hh"
#include "G4PhysicsLogVector.hh"
#include "G4Track.hh"
#include "G4VDecayChannel.hh"
#include "G4VExtDecayer.hh"

#include <concepts>

namespace Mustard::inline Extension::Geant4X::inline Decay::internal {

template<typename Decay>
class ExtendDecayIt : public Decay {
public:
    using Decay::Decay;

    virtual auto DecayIt(const G4Track& aTrack, const G4Step&) -> G4VParticleChange* override;
};

} // namespace Mustard::inline Extension::Geant4X::inline Decay::internal

#include "Mustard/Extension/Geant4X/Decay/internal/ExtendDecayIt.inl"
