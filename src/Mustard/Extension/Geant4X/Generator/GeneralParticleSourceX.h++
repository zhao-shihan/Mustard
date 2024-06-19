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

#include "Mustard/Extension/Geant4X/Generator/GeneralParticleSourceXMessenger.h++"

#include "G4GeneralParticleSource.hh"

namespace Mustard::inline Extension::Geant4X::inline Generator {

class GeneralParticleSourceX : public G4GeneralParticleSource {
public:
    GeneralParticleSourceX();

    auto NVertex() const -> auto { return fNVertex; }
    auto PulseWidth() const -> auto { return fPulseWidth; }

    auto NVertex(int n) -> void { fNVertex = std::max(0, n); }
    auto PulseWidth(double val) -> void { fPulseWidth = val; }

    auto GeneratePrimaryVertex(G4Event*) -> void override;

private:
    int fNVertex;
    double fPulseWidth;

    GeneralParticleSourceXMessenger::Register<GeneralParticleSourceX> fMessengerRegister;
};

} // namespace Mustard::inline Extension::Geant4X::inline Generator
