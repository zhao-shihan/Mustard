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

#include "Mustard/Geant4X/Physics/MuoniumPhysicsMessenger.h++"
#include "Mustard/Geant4X/Process/FineStructureForAtomThermalScattering.h++"
#include "Mustard/Geant4X/Track/ParticleChangeForThermalScattering.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256Plus.h++"
#include "Mustard/Math/Vector.h++"

#include "CLHEP/Units/PhysicalConstants.h"
#include "CLHEP/Units/SystemOfUnits.h"

#include "G4VContinuousProcess.hh"

#include "gtl/phmap.hpp"

#include <utility>
#include <vector>

class G4Material;
class G4ParticleDefinition;
class G4Run;
class G4SafetyHelper;
class G4Step;
class G4Track;
class G4VParticleChange;

namespace Mustard::Geant4X::inline Process {

class MuoniumThermalScattering : public G4VContinuousProcess {
public:
    MuoniumThermalScattering();

    auto AddFineStructure(const FineStructureForAtomThermalScattering* fineStructure) -> void;
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

    auto ThermalVelocity(double sigmaVelocity1D) -> std::pair<double, Vector3D>;
    auto MeanFreePathIn(const G4Material& material) const -> double;
    auto SimilarToAir(const G4Material& material) const -> bool;

    static auto SigmaVelocity1D(double temperature, const G4ParticleDefinition& particle) -> double;

private:
    enum struct Status {
        SingleThermalScattering,
        CoarseGrainedThermalScattering,
        InFineStructureVolume,
        WaitForOtherProcess,
        StopButAlive
    };

private:
    std::vector<const FineStructureForAtomThermalScattering*> fFineStructure;
    const FineStructureForAtomThermalScattering* fActiveFineStructure;

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

    MuoniumPhysicsMessenger::Register<MuoniumThermalScattering> fMessengerRegister;

    static constexpr auto fgSpatialTiny{CLHEP::Bohr_radius};     // a tiny space scale that is physically meaningless but large enough for double
    static constexpr auto fgSpatialHuge{1.4e10 * CLHEP::parsec}; // a huge space scale that is physically meaningless but small enough for double
    static constexpr auto fgCoarseGrainingSafetyMaxZ{5.3};
    static constexpr auto fgCoarseGrainingTimeMaxZ{4.5};
};

} // namespace Mustard::Geant4X::inline Process
