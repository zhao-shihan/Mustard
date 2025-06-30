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

#include "Mustard/Extension/Geant4X/Particle/Antimuonium.h++"
#include "Mustard/Extension/Geant4X/Particle/Muonium.h++"
#include "Mustard/Extension/Geant4X/Physics/MuoniumPhysicsMessenger.h++"
#include "Mustard/Extension/Geant4X/Physics/TargetForMuoniumPhysics.h++"
#include "Mustard/Math/Random/Distribution/Exponential.h++"
#include "Mustard/Math/Random/Distribution/Gaussian3DDiagnoal.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256Plus.h++"
#include "Mustard/Utility/LiteralUnit.h++"
#include "Mustard/Utility/PhysicalConstant.h++"

#include "G4Material.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4ParticleChange.hh"
#include "G4ThreeVector.hh"
#include "G4Track.hh"
#include "G4VContinuousProcess.hh"
#include "G4VSolid.hh"
#include "Randomize.hh"

#include "muc/utility"

#include <random>

namespace Mustard::inline Extension::Geant4X::inline Process {

template<TargetForMuoniumPhysics ATarget>
class MuoniumTransport : public G4VContinuousProcess {
public:
    MuoniumTransport();

    auto ManipulateAllSteps(G4bool val) -> void { fManipulateAllSteps = val; }

    virtual auto IsApplicable(const G4ParticleDefinition&) -> G4bool override;
    virtual auto AlongStepDoIt(const G4Track& track, const G4Step&) -> G4VParticleChange* override;

private:
    virtual auto GetContinuousStepLimit(const G4Track& track, G4double, G4double, G4double&) -> G4double override;
    auto ProposeRandomFlight(const G4Track& track) -> void;

private:
    enum struct TransportStatus {
        Unknown = -1,
        Decaying,
        InsideTargetVolume,
        OutsideTargetVolume
    };

private:
    const ATarget* const fTarget;

    G4bool fManipulateAllSteps;

    G4ParticleChange fParticleChange;
    TransportStatus fTransportStatus;
    G4bool fIsExitingTargetVolume;

    Math::Random::Xoshiro256Plus fXoshiro256Plus;
    Math::Random::Gaussian3DDiagnoalFast<G4ThreeVector> fStandardGaussian3D;

    typename MuoniumPhysicsMessenger<ATarget>::template Register<MuoniumTransport<ATarget>> fMessengerRegister;
};

} // namespace Mustard::inline Extension::Geant4X::inline Process

#include "Mustard/Extension/Geant4X/Process/MuoniumTransport.inl"
