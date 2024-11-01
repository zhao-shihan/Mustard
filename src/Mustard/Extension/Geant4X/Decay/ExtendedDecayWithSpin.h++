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

#include "Mustard/Extension/Geant4X/Decay/internal/ExtendDecayIt.h++"

#include "G4DecayWithSpin.hh"

namespace Mustard::inline Extension::Geant4X::inline Decay {

class ExtendedDecayWithSpin : public internal::ExtendDecayIt<G4DecayWithSpin> {
public:
    ExtendedDecayWithSpin(const G4String& processName = "ExtendedDecayWithSpin");

protected:
    virtual auto PostStepDoIt(const G4Track& aTrack, const G4Step& aStep) -> G4VParticleChange* override;
    virtual auto AtRestDoIt(const G4Track& aTrack, const G4Step& aStep) -> G4VParticleChange* override;
};

} // namespace Mustard::inline Extension::Geant4X::inline Decay