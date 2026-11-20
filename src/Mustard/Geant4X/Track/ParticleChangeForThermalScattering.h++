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

#pragma once

#include "Mustard/Math/Vector.h++"

#include "G4VParticleChange.hh"

#include <optional>

namespace Mustard::Geant4X::inline Track {

class ParticleChangeForThermalScattering : public G4VParticleChange {
public:
    auto Initialize(const G4Track& track) -> void override;

    auto ProposeTimeIncrement(double dt) -> void { fTimeIncrement = dt; }
    auto ProposePosition(const Vector3D& dx) -> void { fPosition = dx; }
    auto ProposeVelocityDirection(const Vector3D& d) -> void { fVelocityDirection = d; }
    auto ProposeVelocity(double v) -> void { fVelocity = v; }

    auto UpdateStepForAlongStep(G4Step* step) -> G4Step* override;

private:
    double fMass;
    std::optional<double> fTimeIncrement;
    Vector3D fPosition;
    Vector3D fVelocityDirection;
    double fVelocity;
};

} // namespace Mustard::Geant4X::inline Track
