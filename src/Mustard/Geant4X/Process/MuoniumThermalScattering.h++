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

#include "Mustard/EigenX/AlignedVector3.h++"
#include "Mustard/Geant4X/Particle/Antimuonium.h++"
#include "Mustard/Geant4X/Particle/Muonium.h++"
#include "Mustard/Geant4X/Physics/MuoniumPhysicsMessenger.h++"
#include "Mustard/Geant4X/Physics/TargetForMuoniumPhysics.h++"
#include "Mustard/Geant4X/Track/ParticleChangeForThermalScattering.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Math/Random/Distribution/Exponential.h++"
#include "Mustard/Math/Random/Distribution/Gaussian3D.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256Plus.h++"
#include "Mustard/Math/Vector.h++"
#include "Mustard/Utility/LiteralUnit.h++"
#include "Mustard/Utility/MathConstant.h++"
#include "Mustard/Utility/PhysicalConstant.h++"
#include "Mustard/Utility/VectorCast.h++"

#include "CLHEP/Units/SystemOfUnits.h"

#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4NistManager.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4SafetyHelper.hh"
#include "G4Track.hh"
#include "G4TransportationManager.hh"
#include "G4VContinuousProcess.hh"
#include "G4VPhysicalVolume.hh"
#include "Randomize.hh"

#include "gtl/phmap.hpp"

#include "muc/numeric"
#include "muc/random"
#include "muc/utility"

#include "fmt/format.h"

#include <array>
#include <bit>
#include <cassert>
#include <cstdint>

namespace Mustard::Geant4X::inline Process {

template<TargetForMuoniumPhysics ATarget>
class MuoniumThermalScattering : public G4VContinuousProcess {
public:
    MuoniumThermalScattering();

    auto DisableCoarseGraining(bool val) -> void { fDisableCoarseGraining = val; }
    auto MinCoarseGrainingScale(double val) -> void;

    auto StartTracking(G4Track* track) -> void override;
    auto IsApplicable(const G4ParticleDefinition& particle) -> bool override;
    auto AlongStepDoIt(const G4Track& track, const G4Step&) -> G4VParticleChange* override;

protected:
    auto GetContinuousStepLimit(const G4Track& track, double previousStepSize, double currentMinimumStep, double&) -> double override;

private:
    auto SingleThermalScattering(const G4Track& track) -> void;
    auto CoarseGrainedThermalScattering(const G4Track& track) -> void;
    auto FullThermalScatteringInFineStructureVolume(const G4Track& track) -> void;

    auto MeanFreePathIn(const G4Material& material) const -> double;
    auto SimilarToAir(const G4Material& material) const -> bool;

    static auto SigmaVelocity1D(double temperature, double mass) -> double;
    static auto ThermalVelocity(double sigmaVelocity1D) -> std::pair<double, Vector3D>;

private:
    enum struct Status {
        SingleThermalScattering,
        CoarseGrainedThermalScattering,
        InFineStructureVolume,
        WaitForOtherProcess,
        StopButAlive
    };

private:
    const ATarget* fFineStructure;

    bool fDisableCoarseGraining;
    double fMinCoarseGrainingScale;

    const G4Run* fCurrentRun;
    Random::Xoshiro256Plus fRNG;
    G4SafetyHelper* fSafetyHelper;
    const G4Material* fCurrentMaterial;
    const G4ParticleDefinition* fCurrentParticle;
    const G4Material* fCurrentOuterMaterial;
    const G4Material* fAirMaterial;
    gtl::flat_hash_map<int, double> fAirComposition;

    Status fStatus;
    double fSigmaVelocity1D;
    double fMeanInverseVelocity;
    double fMultipleScatteringTimeLimit;
    double fMultipleScatteringPathLength;
    double fOuterMeanFreePath;
    double fOuterSigmaVelocity1D;
    ParticleChangeForThermalScattering fParticleChange;

    typename MuoniumPhysicsMessenger<ATarget>::template Register<MuoniumThermalScattering<ATarget>> fMessengerRegister;

    static constexpr auto fgSpatialTiny{CLHEP::Bohr_radius};     // a tiny space scale that is physically meaningless but large enough for double
    static constexpr auto fgSpatialHuge{1.4e10 * CLHEP::parsec}; // a huge space scale that is physically meaningless but small enough for double
    static constexpr auto fgCoarseGrainingSafetyMaxZ{5.3};
    static constexpr auto fgCoarseGrainingTimeMaxZ{4.5};
};

} // namespace Mustard::Geant4X::inline Process

#include "Mustard/Geant4X/Process/MuoniumThermalScattering.inl"
