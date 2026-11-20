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

#include "Mustard/EigenX/AlignedVector3.h++"
#include "Mustard/Geant4X/Management/Exception.h++"
#include "Mustard/Geant4X/Process/ThermalScattering.h++"
#include "Mustard/Math/Random/Distribution/Exponential.h++"
#include "Mustard/Math/Random/Distribution/Gaussian3D.h++"
#include "Mustard/Math/Random/Distribution/Gaussian4D.h++"
#include "Mustard/Math/Vector.h++"
#include "Mustard/Utility/LiteralUnit.h++"
#include "Mustard/Utility/MathConstant.h++"
#include "Mustard/Utility/PhysicalConstant.h++"
#include "Mustard/Utility/VectorCast.h++"

#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4SafetyHelper.hh"
#include "G4SystemOfUnits.hh"
#include "G4Track.hh"
#include "G4TransportationManager.hh"
#include "G4VPhysicalVolume.hh"
#include "Randomize.hh"

#include "muc/numeric"
#include "muc/random"
#include "muc/utility"

#include "fmt/format.h"

#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <source_location>

namespace Mustard::Geant4X::inline Process {

using namespace LiteralUnit::Length;

ThermalScattering::ThermalScattering(G4String meanFreePathMaterialPropertyName,
                                     const G4String& processName, G4ProcessType processType) :
    G4VContinuousProcess{processName, processType},
    fMeanFreePathMaterialPropertyName{std::move(meanFreePathMaterialPropertyName)},
    fDisableCoarseGraining{},
    fMinCoarseGrainingScale{100},
    fFineStructure{},
    fCurrentRun{},
    fRNG{},
    fSafetyHelper{},
    fCurrentMaterial{},
    fCurrentOuterMaterial{},
    fMeanFreePathCache{},
    fStatus{},
    fSigmaVelocity1D{},
    fMeanInverseVelocity{},
    fMultipleScatteringTimeLimit{},
    fMultipleScatteringPathLength{},
    fActiveFineStructure{},
    fOuterMeanFreePath{},
    fOuterSigmaVelocity1D{},
    fParticleChange{},
    fMessengerRegister{this} {
    pParticleChange = &fParticleChange;
}

auto ThermalScattering::AddFineStructure(const FineStructureForThermalScattering* fineStructure) -> void {
    if (fineStructure) {
        fFineStructure.push_back(fineStructure);
    }
}

auto ThermalScattering::MinCoarseGrainingScale(double val) -> void {
    if (constexpr double minScale{30};
        val < minScale) [[unlikely]] {
        Exception<JustWarning>(fmt::format(
            "Minimum coarse-graining scale is too small (got {}, expect >= {}), setting it to {}.", val, minScale, minScale));
        fMinCoarseGrainingScale = minScale;
        return;
    }
    fMinCoarseGrainingScale = val;
}

auto ThermalScattering::IsApplicable(const G4ParticleDefinition& particle) -> bool {
    return particle.GetPDGCharge() == 0; // only neutral particles are applicable
}

auto ThermalScattering::InitializeForNewRun() -> void {
    // Seed once per run to ensure different random number sequences in parallel computing
    fRNG.Seed(std::bit_cast<std::uint64_t>(G4UniformRand()));
    // Get safety helper.
    fSafetyHelper = G4TransportationManager::GetTransportationManager()->GetSafetyHelper();
    fSafetyHelper->InitialiseHelper();
    // Clear cached mean free paths from the previous run
    fMeanFreePathCache.clear();
}

auto ThermalScattering::StartTracking(G4Track* track) -> void {
    if (const auto run{G4RunManager::GetRunManager()->GetCurrentRun()};
        fCurrentRun != std::pair{run, run->GetRunID()}) {
        fCurrentRun = {run, run->GetRunID()};
        InitializeForNewRun();
    }
    G4VContinuousProcess::StartTracking(track);
    fCurrentMaterial = nullptr;           // force trigger update of sigma velocity since particle mass may change
    fCurrentOuterMaterial = std::nullopt; // force trigger update of outer material
    if (not track->GetParticleDefinition()->GetPDGStable() and not fDisableCoarseGraining) {
        // If no other process has pre-assigned the decay proper time, assign one now.
        // Coarse-grained transport requires pre-assigned decay because G4Decay's normal NILL decrement uses a single
        // thermal-velocity sample per step (L/v_endpoint) while the step's proper-time advance uses the thermal-average
        // mean inverse velocity. Pre-assigned decay bypasses this by using proper time directly.
        if (track->GetDynamicParticle()->GetPreAssignedDecayProperTime() < 0) {
            // Cast away constness is safe here because we are in StartTracking and track is not shared.
            const auto dynamicParticle{const_cast<G4DynamicParticle*>(track->GetDynamicParticle())};
            const auto decayProperTime{Random::Exponential{track->GetParticleDefinition()->GetPDGLifeTime()}(fRNG)};
            dynamicParticle->SetPreAssignedDecayProperTime(decayProperTime);
        }
    }
}

auto ThermalScattering::AlongStepDoIt(const G4Track& track, const G4Step& step) -> G4VParticleChange* {
    fParticleChange.Initialize(track);
    if (const auto stepStatus{step.GetPostStepPoint()->GetStepStatus()};
        stepStatus == fGeomBoundary or stepStatus == fWorldBoundary) {
        fStatus = Status::WaitForOtherProcess; // waiting for transportation
    }
    switch (fStatus) {
    case Status::CoarseGrainedThermalScattering:
        CoarseGrainedThermalScattering(track);
        break;
    case Status::SingleThermalScattering:
        SingleThermalScattering(track);
        break;
    case Status::InFineStructureVolume:
        FullThermalScatteringInFineStructureVolume(track);
        break;
    case Status::WaitForOtherProcess:
        // Do nothing
        break;
    case Status::StopButAlive:
        fParticleChange.ProposeTrackStatus(fStopButAlive);
        break;
    }
    return &fParticleChange;
}

auto ThermalScattering::MeanFreePathIn(const G4Material& material) -> double {
    const auto mpt{material.GetMaterialPropertiesTable()};
    if (mpt == nullptr) {
        return 0;
    }
    if (const auto it{fMeanFreePathCache.find(mpt)};
        it != fMeanFreePathCache.end()) [[likely]] {
        return it->second;
    }
    if (not mpt->ConstPropertyExists(fMeanFreePathMaterialPropertyName)) {
        fMeanFreePathCache.emplace(mpt, 0);
        return 0;
    }
    const auto meanFreePath{mpt->GetConstProperty(fMeanFreePathMaterialPropertyName)};
    if (meanFreePath <= fgSpatialTiny) [[unlikely]] {
        Exception<JustWarning>(fmt::format(
            "Mean free path in material '{}' is too small ({} nm), particle will be stopped.",
            material.GetName(), meanFreePath / nm));
    }
    fMeanFreePathCache.emplace(mpt, meanFreePath);
    return meanFreePath;
}

auto ThermalScattering::GetContinuousStepLimit(const G4Track& track, double previousStepSize, double currentMinimumStep, double&) -> double {
    using namespace LiteralUnit::Length;
    using namespace MathConstant;
    using namespace PhysicalConstant;

    // Note: the framework `safety` out-argument is intentionally ignored. Transportation, which fills it, runs
    // AFTER this process in the AlongStep GPIL vector, so on entry it is always DBL_MAX. Where a real safety
    // is needed (coarse-graining) we query the safety helper ourselves; we never write back to `safety` (like
    // G4VMultipleScattering, we let Transportation propose the framework safety).

    if (theNumberOfInteractionLengthLeft > 0 and previousStepSize > 0) {
        theNumberOfInteractionLengthLeft -= previousStepSize / currentInteractionLength;
    }
    if (theNumberOfInteractionLengthLeft <= 0) {
        theNumberOfInteractionLengthLeft = Random::Exponential{}(fRNG);
        theInitialNumberOfInteractionLength = theNumberOfInteractionLengthLeft;
    }

    if (track.GetVelocity() == 0) [[unlikely]] {
        Exception<JustWarning>("Zero initial velocity is not allowed, stopping the track.");
        fStatus = Status::StopButAlive;
        return 0;
    }

    const auto temperature{track.GetMaterial()->GetTemperature()};
    if (const auto meanKineticEnergy{1.5 * k_Boltzmann * temperature};
        track.GetKineticEnergy() > 50 * meanKineticEnergy) [[unlikely]] {
        const auto kineticTemperature{track.GetKineticEnergy() / (1.5 * k_Boltzmann)};
        Exception<JustWarning>(fmt::format(
            "Kinetic energy {} eV (equivalent to {} K) is too high for thermal scattering in material at {} K. "
            "It will be thermalized to the local temperature without recording energy deposition.",
            track.GetKineticEnergy() / eV, kineticTemperature / kelvin, temperature / kelvin));
    }

    if (fCurrentMaterial != track.GetMaterial()) {
        fCurrentMaterial = track.GetMaterial();
        currentInteractionLength = MeanFreePathIn(*fCurrentMaterial);
        if (currentInteractionLength <= fgSpatialTiny) {
            fStatus = Status::StopButAlive;
            return 0;
        }
        fSigmaVelocity1D = SigmaVelocity1D(temperature, *track.GetParticleDefinition());
        fMeanInverseVelocity = sqrt2 * inv_sqrtpi / fSigmaVelocity1D; // <1/v> = √(2m/(πkT)) = √(2/π)/σv
    }
    const auto singleScatteringFreePath{currentInteractionLength * theNumberOfInteractionLengthLeft};

    // If other process will happen between thermal scatterings, let it win
    if (currentMinimumStep <= singleScatteringFreePath) {
        SetGPILSelection(NotCandidateForSelection);
        fStatus = Status::WaitForOtherProcess;
        return std::numeric_limits<double>::max();
    }

    // Track full thermal scattering in target fine structure volume if applicable
    for (auto&& fineStructure : std::as_const(fFineStructure)) {
        auto position{VectorCast<EigenX::AlignedVector3d>(track.GetPosition())};
        fineStructure->TransformPositionFromGlobalFrameToFineStructureLocalFrame(position);
        if (not fineStructure->FineStructureVolumeContains(position)) {
            continue;
        }
        fStatus = Status::InFineStructureVolume;
        fMultipleScatteringTimeLimit = currentMinimumStep / track.GetVelocity(); // e.g. decay process gives path length limit as velocity*(decay time)
        fMultipleScatteringPathLength = currentMinimumStep;                      // true-path limit
        fActiveFineStructure = fineStructure;
        // Get particle mean free path and temperature in grandmother volume of fine structure volume.
        // They are parameters NOT inside the fine structure but inside fine structure volume.
        const auto touchable{track.GetTouchable()};
        const auto outerVolume{touchable->GetHistoryDepth() >= 2 ? touchable->GetVolume(2) : nullptr};
        const auto outerMaterial{outerVolume ? outerVolume->GetLogicalVolume()->GetMaterial() : nullptr};
        auto outerTemperature{temperature};
        if (fCurrentOuterMaterial != outerMaterial) {
            fCurrentOuterMaterial = outerMaterial;
            if (outerMaterial) {
                fOuterMeanFreePath = MeanFreePathIn(*outerMaterial);
                outerTemperature = outerMaterial->GetTemperature();
                fOuterSigmaVelocity1D = SigmaVelocity1D(outerTemperature, *track.GetParticleDefinition());
            } else {
                fOuterMeanFreePath = fgSpatialHuge;
                fOuterSigmaVelocity1D = fSigmaVelocity1D; // not used though
            }
        }
        // See comments in FullThermalScatteringInFineStructureVolume for details.
        if (currentInteractionLength > 10 * fOuterMeanFreePath) [[unlikely]] {
            Exception<JustWarning>(fmt::format(
                "Mean free path in outer material ({:.2f} nm) is much smaller than that in fine structure ({:.2f} nm). "
                "The accuracy will be reduced.",
                fOuterMeanFreePath / nm, currentInteractionLength / nm));
        }
        // Return a tiny (but non-zero) step so that G4Transportation's AlongStepDoIt still runs ComputeStep.
        // The full random flight (which may leave the fine-structure volume by tiny when limited by boundary)
        // is applied by overriding the position in AlongStepDoIt. When that endpoint lands in a *neighbouring*
        // G4 volume, ComputeStep on the next step detects the out-of-volume start point (Step=0, exiting) and
        // Transportation's forced PostStepDoIt performs a full relocation, so the navigator self-recovers to
        // the correct. Returning exactly 0 here would instead trigger Transportation's `currentMinimumStep==0`
        // shortcut, which skips ComputeStep and defeats that self-recovery. (While inside the volume the tiny
        // step is physics-limited, so it is not counted as a stuck "zero step".)
        // The true path length is set via ProposeTrueStepLength.
        return std::numeric_limits<double>::min();
    }

    // Try coarse-grained thermal scattering if not disabled
    while (not fDisableCoarseGraining) {
        // Check that the current minimum step, the coarse-graining time limit, and the safety path length are all
        // larger than the coarse-graining threshold. The two cheap (no-geometry) checks are done first; the safety
        // is queried only if they pass, because ComputeSafety is the expensive part.
        const auto minCoarseGrainingPathLength{fMinCoarseGrainingScale * currentInteractionLength};
        if (currentMinimumStep < minCoarseGrainingPathLength) {
            break;
        }
        // Coarse graining via CLT: path length fixed, elapsed time = Δt ~ N(L*<1/v>, σt), where σt = sqrt(2*λ*L*(1-2/π))/σv
        // Δt > tLimit = L*<1/v> + 4.5*σt is truncated (P(Δt > L*<1/v> + 4.5*σt) = 3.4e-6 so it's good enough)
        // Solve tLimit = L*<1/v> + 4.5*σt w.r.t. to L gives the following.
        fMultipleScatteringTimeLimit = currentMinimumStep / track.GetVelocity(); // e.g. decay process gives path length limit as velocity*(decay time)
        const auto tQuantCoeff{fgCoarseGrainingTimeMaxZ / fSigmaVelocity1D * std::sqrt(2 * currentInteractionLength * (1 - 2 / pi))};
        const auto sqrtPathLengthLimitFromTimeLimit{(std::sqrt(muc::pow(tQuantCoeff, 2) + 4 * fMeanInverseVelocity * fMultipleScatteringTimeLimit) - tQuantCoeff) / (2 * fMeanInverseVelocity)};
        const auto pathLengthLimitFromTimeLimit{muc::pow(sqrtPathLengthLimitFromTimeLimit, 2)};
        if (pathLengthLimitFromTimeLimit < minCoarseGrainingPathLength) {
            break;
        }
        // Coarse graining via CLT: path length fixed, displacement = Δx ~ N^3(0, σx), where σx = sqrt(2/3*λ*L)
        // |Δx| > safety = 5.3*σx is truncated (P(|Δx| > 5.3*σx) = 3.5e-6 so it's good enough)
        // σx = sqrt(2/3*λ*L), safety = 5.3*σx  =>  L = 1.5/λ*(safety/5.3)^2
        // The REAL isotropic safety is required here (the framework `safety` argument is DBL_MAX, see top of
        // this function): it keeps the net diffusion displacement inside the current physical volume so the
        // particle cannot diffuse *through* a volume boundary. The navigator is correctly located at the
        // pre-step point (Transportation relocates it in its forced PostStepDoIt every step), so ComputeSafety
        // here is valid. No safety margin is applied: the isotropic safety is already an in-volume bound and
        // Geant4 tolerates the floating-point boundary edge case.
        const auto safety{fSafetyHelper->ComputeSafety(track.GetPosition())};
        const auto safetyPathLength{1.5 / currentInteractionLength * muc::pow(safety / fgCoarseGrainingSafetyMaxZ, 2)};
        if (safetyPathLength < minCoarseGrainingPathLength) {
            break;
        }
        // Now we can do coarse-grained thermal scattering. The true path length is the minimum of three limits:
        //  - currentMinimumStep: the path length to the next PostStep (discrete) interaction.
        //  - pathLengthLimitFromTimeLimit: keeps the diffusion elapsed time (including its +4.5σ tail) within the
        //    lifetime; more precise than currentMinimumStep alone for the decay time spread.
        //  - safetyPathLength: keeps the net displacement inside the current material (space limit).
        fStatus = Status::CoarseGrainedThermalScattering;
        fMultipleScatteringPathLength = std::min({currentMinimumStep, pathLengthLimitFromTimeLimit, safetyPathLength});
        // Return exactly 0: the displacement is bounded by the real safety so the overridden endpoint is always
        // in-volume (no self-recovery needed), and 0 makes Transportation take its `currentMinimumStep==0`
        // shortcut (no ComputeStep, no truncation of our step) while its forced PostStepDoIt still relocates the
        // navigator within the volume to our overridden position.
        // The true path length is set via ProposeTrueStepLength.
        return 0;
    }

    // Otherwise, do single thermal scattering
    fStatus = Status::SingleThermalScattering;
    return singleScatteringFreePath;
}

auto ThermalScattering::ThermalVelocity(double sigmaVelocity1D) -> std::pair<Vector3D, double> {
    Vector3D direction;
    double velocity;
    do {
        direction = Random::Gaussian3D<Vector3D>{}(fRNG);
        velocity = direction.mag2();
    } while (velocity <= muc::pow(muc::default_abs_tol<double>, 2));
    velocity = std::sqrt(velocity);
    direction *= 1 / velocity; // do not use CLHEP::Hep3Vector::operator/=(double).
    velocity *= sigmaVelocity1D;
    return {direction, velocity};
}

auto ThermalScattering::SingleThermalScattering(const G4Track&) -> void {
    assert(fStatus == Status::SingleThermalScattering);
    // This looks really like a PostStepDoIt action, and one may argue that we should use G4VContinuousDiscreteProcess.
    // However, since we need the interplay between multiple thermal scattering and single thermal scattering, the best choice
    // is to use G4VContinuousProcess and also do the single thermal scattering in AlongStepDoIt though this may looks funny.
    const auto [direction, velocity]{ThermalVelocity(fSigmaVelocity1D)};
    fParticleChange.ProposeVelocityDirection(direction);
    fParticleChange.ProposeVelocity(velocity);
}

auto ThermalScattering::CoarseGrainedThermalScattering(const G4Track& track) -> void {
    using namespace MathConstant;
    using namespace PhysicalConstant;
    assert(fStatus == Status::CoarseGrainedThermalScattering);
    // Coarse graining via CLT: path length fixed, displacement = Δx ~ N^3(0, σx), where σx = sqrt(2/3*λ*L)
    const auto sqrt2LambdaL{std::sqrt(2 * currentInteractionLength * fMultipleScatteringPathLength)};
    const auto sigmaDisplacement1D{sqrt2LambdaL * inv_sqrt3};
    // Coarse graining via CLT: path length fixed, elapsed time = Δt ~ N(L*<1/v>, σt), where σt = sqrt(2*λ*L*(1-2/π))/σv
    const auto meanElapsedTime{fMultipleScatteringPathLength * fMeanInverseVelocity};
    const auto sigmaElapsedTime{std::sqrt(1 - 2 / pi) * sqrt2LambdaL / fSigmaVelocity1D};
    const auto minElapsedTime{fMultipleScatteringPathLength / (0.5 * c_light)};
    const auto elapsedTimeMinZ{(minElapsedTime - meanElapsedTime) / sigmaElapsedTime};
    // Generate normal random vectors and truncate them to ensure that the displacement and elapsed time are within the safety and time limit.
    VectorLor gausXYZT;
    do {
        gausXYZT = Random::Gaussian4D<VectorLor>{}(fRNG);
    } while (gausXYZT.vect().mag2() >= muc::pow(fgCoarseGrainingSafetyMaxZ, 2) or
             gausXYZT.t() <= elapsedTimeMinZ or gausXYZT.t() >= fgCoarseGrainingTimeMaxZ);
    const auto displacement{sigmaDisplacement1D * gausXYZT.vect()};
    const auto elapsedTime{meanElapsedTime + sigmaElapsedTime * gausXYZT.t()};
    // Sample final velocity and direction, and propose the new state of the particle.
    // The position is overridden here; we do NOT call G4SafetyHelper::ReLocateWithinVolume: G4Transportation's
    // forced PostStepDoIt runs every step and relocates the navigator to this overridden position (within-volume,
    // since the displacement is bounded by the real safety). The recorded step length is the true path length.
    const auto [direction, velocity]{ThermalVelocity(fSigmaVelocity1D)};
    fParticleChange.ProposeTrueStepLength(fMultipleScatteringPathLength);
    fParticleChange.ProposeTimeIncrement(elapsedTime);
    fParticleChange.ProposePosition(track.GetPosition() + displacement);
    fParticleChange.ProposeVelocityDirection(direction);
    fParticleChange.ProposeVelocity(velocity);
}

auto ThermalScattering::FullThermalScatteringInFineStructureVolume(const G4Track& track) -> void {
    using namespace LiteralUnit::Length;
    assert(fStatus == Status::InFineStructureVolume);
    assert(fActiveFineStructure);

    // Get initial position and direction in local frame of fine structure
    using Vec3 = EigenX::AlignedVector3d;                                  // A vertorizable 3-vector
    auto initialPosition{VectorCast<Vec3>(track.GetPosition())};           // initial position in local frame
    auto initialDirection{VectorCast<Vec3>(track.GetMomentumDirection())}; // initial momentum direction in local frame
    fActiveFineStructure->TransformPositionFromGlobalFrameToFineStructureLocalFrame(initialPosition);
    fActiveFineStructure->TransformDirectionFromGlobalFrameToFineStructureLocalFrame(initialDirection);
    assert(fActiveFineStructure->FineStructureVolumeContains(initialPosition));

    // Random distributions
    static_assert(Random::Exponential<>::Stateless());
    static_assert(Random::Gaussian3D<Vec3>::Stateless());
    const auto outerMeanFreePathTiny{fOuterMeanFreePath <= fgSpatialTiny};
    const auto outerMeanFreePathHuge{fOuterMeanFreePath >= fgSpatialHuge};

    // Dynamic state variables
    auto freePath{currentInteractionLength * theNumberOfInteractionLengthLeft}; // the free path left for single scattering
    double pathLength{};                                                        // the total flight length in this G4Step
    double elapsedTime{};                                                       // elapsed time of this flight
    Vec3 displacement{0, 0, 0};                                                 // displacement of this flight in local frame (tracking displacement instead of local position for better numeric accuracy)
    Vec3 position;                                                              // current position in local frame
    auto direction{initialDirection};                                           // velocity direction in local frame
    auto velocity{track.GetVelocity()};                                         // velocity magnitude
    auto outerFreePathLeft{Random::Exponential{fOuterMeanFreePath}(fRNG)};      // the free path left at outside of fine structure volume
    bool timeUp;                                                                // flag indicate that the flight was terminated by time limit
    bool pathLengthLimitReached;                                                // flag indicate that the flight was terminated by path length limit
    bool outOfBound;                                                            // flag indicate that the flight was terminated by geometry boundary

    // Do thermal scattering until one of the limit is reached
    const auto stepForward{[&](double step) { // step < 0 means backward
        pathLength += step;
        elapsedTime += step / velocity;
        displacement += step * direction;
        position = initialPosition + displacement;
    }};
    // The final correction to fulfill the limit
    const auto finalCorrection{[&] {
        // Correction from time limit
        double timeFSC{timeUp ? velocity * (elapsedTime - fMultipleScatteringTimeLimit) : 0};
        // Correction from path length limit
        double pathFSC{pathLengthLimitReached ? pathLength - fMultipleScatteringPathLength : 0};
        // Correction from geometry boundary
        double geomFSC{};
        if (outOfBound) {
            auto binaryOut{displacement};
            auto binaryIn{(displacement - freePath * direction).eval()};
            for (auto residual{freePath}; residual > fgSpatialTiny / 2; residual /= 2) {
                auto binaryMid{((binaryOut + binaryIn) / 2).eval()};
                position = initialPosition + binaryMid;
                if (fActiveFineStructure->FineStructureVolumeContains(position)) {
                    binaryIn = binaryMid;
                } else {
                    binaryOut = binaryMid;
                }
            }
            geomFSC = (displacement - binaryOut).norm(); // use binaryOut to ensure the final position is outside the volume
            // limited by geometry, use path length before correction to preserve residual free path for next step
            theNumberOfInteractionLengthLeft = pathLength / currentInteractionLength;
        }
        // Take the largest correction
        if (const auto fsc{std::max({pathFSC, timeFSC, geomFSC})};
            fsc < pathLength) {
            stepForward(-fsc);
        }
    }};
    do {
        // Step forward and check limits
        stepForward(freePath);
        timeUp = (elapsedTime >= fMultipleScatteringTimeLimit);
        pathLengthLimitReached = (pathLength >= fMultipleScatteringPathLength);
        outOfBound = not fActiveFineStructure->FineStructureVolumeContains(position);
        if (timeUp or pathLengthLimitReached or outOfBound) {
            finalCorrection();
            break;
        }
        const auto consumedFreePath{freePath};
        freePath = Random::Exponential{currentInteractionLength}(fRNG);
        // Within limits, check whether the position is inside the fine structure, and handle the case that
        // the position is outside the fine structure but still inside the fine structure volume.
        const auto insideFineStructure{fActiveFineStructure->FineStructureSubterm(position)};
        if (not insideFineStructure) {
            // The strategy here is accurate for the case that mean free path in outer material (fOuterMeanFreePath) is much larger than
            // that of fine structure (currentInteractionLength). E.g. producing muonium in vacuum from laser-ablated silica aerogel.
            // If fOuterMeanFreePath is comparable to or smaller than currentInteractionLength this will work, and the result is
            // as if the denser outer material somewhat diffuse into the fine structure material and both reduces mean free path
            // and changes the temperature near the fine structure boundary.
            outerFreePathLeft -= consumedFreePath;
            if (outerFreePathLeft > 0 or outerMeanFreePathHuge) {
                continue; // no scattering outside the fine structure, just continue
            }
            stepForward(outerFreePathLeft); // move back
            if (outerMeanFreePathTiny) [[unlikely]] {
                fParticleChange.ProposeTrackStatus(fStopButAlive);
                break; // outer mean free path is too small, stop the flight
            }
            outerFreePathLeft = Random::Exponential{fOuterMeanFreePath}(fRNG);
        }
        // Sample thermal velocity
        do {
            direction = Random::Gaussian3D<Vec3>{}(fRNG);
            velocity = direction.squaredNorm();
        } while (velocity <= muc::pow(muc::default_abs_tol<double>, 2));
        velocity = std::sqrt(velocity);
        direction /= velocity;
        velocity *= insideFineStructure ? fSigmaVelocity1D : fOuterSigmaVelocity1D;
    } while (not fDisableCoarseGraining);

    // Transfrom back to global frame
    fActiveFineStructure->TransformPositionFromFineStructureLocalFrameToGlobalFrame(position);
    fActiveFineStructure->TransformDirectionFromFineStructureLocalFrameToGlobalFrame(direction);

    // Propose particle change. As in the coarse-grain case we do NOT call ReLocateWithinVolume: G4Transportation's
    // forced PostStepDoIt relocates the navigator to this overridden position every step. If the flight left the
    // fine-structure volume into a neighbouring G4 volume, ComputeStep on the next step detects the out-of-volume
    // start point and Transportation performs a full relocation (self-recovery) — no geometry buffer is required.
    // Precondition: the fine-structure volume must lie within a single G4 physical volume so this relocation is valid.
    fParticleChange.ProposeTrueStepLength(pathLength);
    fParticleChange.ProposeTimeIncrement(elapsedTime);
    fParticleChange.ProposePosition(VectorCast<Vector3D>(position));
    fParticleChange.ProposeVelocityDirection(VectorCast<Vector3D>(direction));
    fParticleChange.ProposeVelocity(velocity);
}

auto ThermalScattering::SigmaVelocity1D(double temperature, const G4ParticleDefinition& particle) -> double {
    using namespace LiteralUnit::Velocity;
    using namespace PhysicalConstant;
    const auto sigma{std::sqrt(c_squared * k_Boltzmann * temperature / particle.GetPDGMass())}; // sqrt(kT/m) = sqrt(c^2kT/mc^2)
    if (sigma <= muc::default_abs_tol<double>) [[unlikely]] {
        Exception<JustWarning>(fmt::format("Sigma of 1D velocity is too small ({} m/s)", sigma / 1_m_s));
    }
    return sigma;
}

} // namespace Mustard::Geant4X::inline Process
