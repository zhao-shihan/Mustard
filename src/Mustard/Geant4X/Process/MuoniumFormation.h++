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

#include "Mustard/Geant4X/Particle/Antimuonium.h++"
#include "Mustard/Geant4X/Particle/Muonium.h++"
#include "Mustard/Geant4X/Physics/MuoniumPhysicsMessenger.h++"
#include "Mustard/Geant4X/Physics/TargetForMuoniumPhysics.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Utility/LiteralUnit.h++"
#include "Mustard/Utility/PhysicalConstant.h++"

#include "G4MuonPlus.hh"
#include "G4ParticleChange.hh"
#include "G4VRestProcess.hh"
#include "Randomize.hh"

#include "muc/math"
#include "muc/numeric"

#include "gsl/gsl"

#include <limits>

namespace Mustard::Geant4X::inline Process {

template<TargetForMuoniumPhysics ATarget>
class MuoniumFormation : public G4VRestProcess {
public:
    MuoniumFormation();

    auto DefaultFormationProbability(double p) -> void;
    auto ConversionProbability(double p) -> void { fConversionProbability = muc::clamp<"[]">(p, 0., 1.); }

    auto IsApplicable(const G4ParticleDefinition&) -> G4bool override;
    auto AtRestDoIt(const G4Track& track, const G4Step&) -> G4VParticleChange* override;

private:
    auto GetMeanLifeTime(const G4Track& track, G4ForceCondition*) -> double override;

private:
    const ATarget* fFineStructure;

    double fDefaultFormationProbability;
    double fConversionProbability;

    G4ParticleChange fParticleChange;

    typename MuoniumPhysicsMessenger<ATarget>::template Register<MuoniumFormation<ATarget>> fMessengerRegister;
};

} // namespace Mustard::Geant4X::inline Process

#include "Mustard/Geant4X/Process/MuoniumFormation.inl"
