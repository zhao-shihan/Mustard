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

#include "Mustard/Geant4X/Physics/ThermalScatteringMessenger.h++"
#include "Mustard/Geant4X/Process/FineStructureForThermalScattering.h++"
#include "Mustard/Geant4X/Track/ParticleChangeForThermalScattering.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256Plus.h++"
#include "Mustard/Math/Vector.h++"

#include "CLHEP/Units/PhysicalConstants.h"
#include "CLHEP/Units/SystemOfUnits.h"

#include "G4VContinuousProcess.hh"

#include "gtl/phmap.hpp"

#include <optional>
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

class ThermalScattering : public G4VContinuousProcess {
public:
    ThermalScattering(G4String meanFreePathMaterialPropertyName,
                      const G4String& processName = "ThermalScattering", G4ProcessType processType = fGeneral);

    auto DisableCoarseGraining(bool val) -> void { fDisableCoarseGraining = val; }
    auto MinCoarseGrainingScale(double val) -> void;
    auto AddFineStructure(const FineStructureForThermalScattering* fineStructure) -> void;

    auto IsApplicable(const G4ParticleDefinition& particle) -> bool override;
    virtual auto InitializeForNewRun() -> void;
    auto StartTracking(G4Track* track) -> void override;
    auto AlongStepDoIt(const G4Track& track, const G4Step&) -> G4VParticleChange* override;

protected:
    virtual auto MeanFreePathIn(const G4Material& material) -> double;
    auto GetContinuousStepLimit(const G4Track& track, double previousStepSize, double currentMinimumStep, double&) -> double override;

    auto ThermalVelocity(double sigmaVelocity1D) -> std::pair<Vector3D, double>;
    auto SingleThermalScattering(const G4Track& track) -> void;
    auto CoarseGrainedThermalScattering(const G4Track& track) -> void;
    auto FullThermalScatteringInFineStructureVolume(const G4Track& track) -> void;

    static auto SigmaVelocity1D(double temperature, const G4ParticleDefinition& particle) -> double;

protected:
    enum struct Status {
        SingleThermalScattering,
        CoarseGrainedThermalScattering,
        InFineStructureVolume,
        WaitForOtherProcess,
        StopButAlive
    };

protected:
    G4String fMeanFreePathMaterialPropertyName;

    bool fDisableCoarseGraining;
    double fMinCoarseGrainingScale;
    std::vector<const FineStructureForThermalScattering*> fFineStructure;

    std::pair<const G4Run*, int> fCurrentRun;
    Random::Xoshiro256Plus fRNG;
    G4SafetyHelper* fSafetyHelper;
    const G4Material* fCurrentMaterial;
    std::optional<const G4Material*> fCurrentOuterMaterial;
    gtl::flat_hash_map<const G4MaterialPropertiesTable*, double> fMeanFreePathCache;

    Status fStatus;
    double fSigmaVelocity1D;
    double fMeanInverseVelocity;
    double fMultipleScatteringTimeLimit;
    double fMultipleScatteringPathLength;
    const FineStructureForThermalScattering* fActiveFineStructure;
    double fOuterMeanFreePath;
    double fOuterSigmaVelocity1D;
    ParticleChangeForThermalScattering fParticleChange;

    ThermalScatteringMessenger::Register<ThermalScattering> fMessengerRegister;

    static constexpr auto fgCoarseGrainingSafetyMaxZ{5.3};
    static constexpr auto fgCoarseGrainingTimeMaxZ{4.5};
    static constexpr auto fgSpatialTiny{CLHEP::Bohr_radius};     // a tiny space scale that is physically meaningless but large enough for double
    static constexpr auto fgSpatialHuge{1.4e10 * CLHEP::parsec}; // a huge space scale that is physically meaningless but small enough for double
};

} // namespace Mustard::Geant4X::inline Process
