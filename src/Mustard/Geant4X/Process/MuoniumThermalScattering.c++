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
#include "Mustard/Geant4X/Particle/Antimuonium.h++"
#include "Mustard/Geant4X/Particle/Muonium.h++"
#include "Mustard/Geant4X/Process/MuoniumThermalScattering.h++"
#include "Mustard/IO/PrettyLog.h++"
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
#include "G4NistManager.hh"
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

namespace Mustard::Geant4X::inline Process {

using namespace LiteralUnit::Length;

MuoniumThermalScattering::MuoniumThermalScattering() :
    G4VContinuousProcess{"MuoniumThermalScattering", fUserDefined},
    fFineStructure{},
    fActiveFineStructure{},
    fDisableCoarseGraining{},
    fMinCoarseGrainingScale{30},
    fCurrentRun{},
    fRNG{},
    fSafetyHelper{},
    fCurrentMaterial{},
    fCurrentParticle{},
    fCurrentOuterMaterial{},
    fAirMaterial{},
    fAirComposition{},
    fStatus{},
    fSigmaVelocity1D{},
    fMeanInverseVelocity{},
    fMultipleScatteringTimeLimit{},
    fMultipleScatteringPathLength{},
    fOuterMeanFreePath{fgSpatialHuge}, // outer mean free path needs a default value corresponding to null outer material
    fOuterSigmaVelocity1D{},
    fParticleChange{},
    fMessengerRegister{this} {
    pParticleChange = &fParticleChange;
}

auto MuoniumThermalScattering::AddFineStructure(const FineStructureForAtomThermalScattering* fineStructure) -> void {
    if (fineStructure) {
        fFineStructure.push_back(fineStructure);
    }
}

auto MuoniumThermalScattering::MinCoarseGrainingScale(double val) -> void {
    if (constexpr double minScale{10};
        val < minScale) [[unlikely]] {
        PrintWarning(fmt::format(
            "Minimum coarse-graining scale is too small (got {}, expect >= {}), setting it to {}.", val, minScale, minScale));
        fMinCoarseGrainingScale = minScale;
        return;
    }
    fMinCoarseGrainingScale = val;
}

auto MuoniumThermalScattering::IsApplicable(const G4ParticleDefinition& particle) -> bool {
    if (particle.GetPDGCharge() != 0) {
        return false; // only neutral particles are applicable
    }
    return &particle == Geant4X::Muonium::Definition() or
           &particle == Geant4X::Antimuonium::Definition();
}

auto MuoniumThermalScattering::StartTracking(G4Track* track) -> void {
    if (const auto run{G4RunManager::GetRunManager()->GetCurrentRun()};
        fCurrentRun != run) {
        fCurrentRun = run;
        // Seed once per run to ensure different random number sequences in parallel computing
        fRNG.Seed(std::bit_cast<std::uint64_t>(G4UniformRand()));
        // Get safety helper.
        fSafetyHelper = G4TransportationManager::GetTransportationManager()->GetSafetyHelper();
        fSafetyHelper->InitialiseHelper();
        // Set up air material and composition for SimilarToAir check.
        fAirMaterial = G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");
        fAirComposition.clear();
        const auto fraction{fAirMaterial->GetFractionVector()};
        const auto nAirElement{static_cast<int>(fAirMaterial->GetNumberOfElements())};
        for (int i{}; i < nAirElement; ++i) {
            fAirComposition[fAirMaterial->GetElement(i)->GetZ()] = fraction[i];
        }
    }
    fCurrentMaterial = nullptr; // force trigger update of sigma velocity since particle mass may change
    fCurrentParticle = track->GetParticleDefinition();
    fCurrentOuterMaterial = nullptr;
    G4VContinuousProcess::StartTracking(track);
    if (not fCurrentParticle->GetPDGStable() and not fDisableCoarseGraining) {
        // If no other process has pre-assigned the decay proper time, assign one now.
        // Coarse-grained transport requires pre-assigned decay because G4Decay's normal NILL decrement uses a single
        // thermal-velocity sample per step (L/v_endpoint) while the step's proper-time advance uses the thermal-average
        // mean inverse velocity. Pre-assigned decay bypasses this by using proper time directly.
        if (track->GetDynamicParticle()->GetPreAssignedDecayProperTime() < 0) {
            // Cast away constness is safe here because we are in StartTracking and track is not shared.
            const auto dynamicParticle{const_cast<G4DynamicParticle*>(track->GetDynamicParticle())};
            const auto decayProperTime{Random::Exponential{fCurrentParticle->GetPDGLifeTime()}(fRNG)};
            dynamicParticle->SetPreAssignedDecayProperTime(decayProperTime);
        }
    }
}

auto MuoniumThermalScattering::AlongStepDoIt(const G4Track& track, const G4Step&) -> G4VParticleChange* {
    assert(theNumberOfInteractionLengthLeft >= 0);
    assert(currentInteractionLength > fgSpatialTiny);
    assert(theInitialNumberOfInteractionLength >= 0);
    assert(fSigmaVelocity1D > muc::default_abs_tol<double>);
    assert(fMultipleScatteringTimeLimit >= 0);
    assert(fMultipleScatteringPathLength >= 0);
    if (track.GetVelocity() == 0) {
        Throw<std::runtime_error>("Zero initial velocity is not allowed.");
    }
    fParticleChange.Initialize(track);
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

auto MuoniumThermalScattering::GetContinuousStepLimit(const G4Track& track, double previousStepSize, double currentMinimumStep, double&) -> double {
    using namespace LiteralUnit::Length;
    using namespace MathConstant;
    using namespace PhysicalConstant;

    // NB: the framework `safety` out-argument is intentionally ignored. Transportation, which fills it, runs
    // AFTER this process in the AlongStep GPIL vector, so on entry it is always DBL_MAX. Where a real safety
    // is needed (coarse-graining) we query the safety helper ourselves; we never write back to `safety` (like
    // G4VMultipleScattering, we let Transportation propose the framework safety).

    const auto temperature{track.GetMaterial()->GetTemperature()};
    if (const auto meanKineticEnergy{1.5 * k_Boltzmann * temperature};
        track.GetKineticEnergy() > 50 * meanKineticEnergy) [[unlikely]] {
        const auto kineticTemperature{track.GetKineticEnergy() / (1.5 * k_Boltzmann)};
        PrintWarning(fmt::format(
            "Kinetic energy {} eV (equivalent to {} K) is too high for thermal scattering in material at {} K. "
            "It will be thermalized to the local temperature without recording energy deposition.",
            track.GetKineticEnergy() / eV, kineticTemperature / kelvin, temperature / kelvin));
    }

    if (theNumberOfInteractionLengthLeft > 0 and previousStepSize > 0) {
        theNumberOfInteractionLengthLeft -= previousStepSize / currentInteractionLength;
    }
    if (theNumberOfInteractionLengthLeft <= 0) {
        theNumberOfInteractionLengthLeft = Random::Exponential{}(fRNG);
        theInitialNumberOfInteractionLength = theNumberOfInteractionLengthLeft;
    }

    if (fCurrentMaterial != track.GetMaterial()) {
        fCurrentMaterial = track.GetMaterial();
        currentInteractionLength = MeanFreePathIn(*fCurrentMaterial);
        if (currentInteractionLength <= fgSpatialTiny) {
            fStatus = Status::StopButAlive;
            return 0;
        }
        fSigmaVelocity1D = SigmaVelocity1D(temperature, *fCurrentParticle);
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
        fActiveFineStructure = fineStructure;
        fStatus = Status::InFineStructureVolume;
        fMultipleScatteringTimeLimit = currentMinimumStep / track.GetVelocity(); // e.g. decay process gives path length limit as velocity*(decay time)
        fMultipleScatteringPathLength = currentMinimumStep;                      // true-path limit
        // Get particle mean free path and temperature in grandmother volume of fine structure volume.
        // They are parameters NOT inside the fine structure but inside fine structure volume.
        const auto touchable{track.GetTouchable()};
        const auto outerVolume{touchable->GetHistoryDepth() >= 2 ? touchable->GetVolume(2) : nullptr};
        const auto outerMaterial{outerVolume ? outerVolume->GetLogicalVolume()->GetMaterial() : nullptr};
        auto outerTemperature{temperature};
        if (fCurrentOuterMaterial != outerMaterial) {
            fCurrentOuterMaterial = outerMaterial;
            if (fCurrentOuterMaterial) {
                fOuterMeanFreePath = MeanFreePathIn(*fCurrentOuterMaterial);
                outerTemperature = fCurrentOuterMaterial->GetTemperature();
                fOuterSigmaVelocity1D = SigmaVelocity1D(outerTemperature, *fCurrentParticle);
            } else {
                fOuterMeanFreePath = fgSpatialHuge;
                // fOuterSigmaVelocity1D is not used when fCurrentOuterMaterial is null.
            }
        }
        // See comments in FullThermalScatteringInFineStructureVolume for details.
        if (currentInteractionLength > 10 * fOuterMeanFreePath) [[unlikely]] {
            PrintWarning(fmt::format(
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

auto MuoniumThermalScattering::SingleThermalScattering(const G4Track&) -> void {
    assert(fStatus == Status::SingleThermalScattering);
    // This looks really like a PostStepDoIt action, and one may argue that we should use G4VContinuousDiscreteProcess.
    // However, since we need the interplay between multiple thermal scattering and single thermal scattering, the best choice
    // is to use G4VContinuousProcess and also do the single thermal scattering in AlongStepDoIt though this may looks funny.
    const auto [velocity, direction]{ThermalVelocity(fSigmaVelocity1D)};
    fParticleChange.ProposeVelocityDirection(direction);
    fParticleChange.ProposeVelocity(velocity);
}

auto MuoniumThermalScattering::CoarseGrainedThermalScattering(const G4Track& track) -> void {
    using namespace MathConstant;
    assert(fStatus == Status::CoarseGrainedThermalScattering);
    // Generate normal random vectors and truncate them to ensure that the displacement and elapsed time are within the safety and time limit.
    auto gausXYZT{Random::Gaussian4D<VectorLor>{}(fRNG)};
    while (gausXYZT.vect().mag2() >= muc::pow(fgCoarseGrainingSafetyMaxZ, 2) or
           gausXYZT.t() >= fgCoarseGrainingTimeMaxZ) [[unlikely]] {
        gausXYZT = Random::Gaussian4D<VectorLor>{}(fRNG);
    }
    const auto sqrt2LambdaL{std::sqrt(2 * currentInteractionLength * fMultipleScatteringPathLength)};
    // Coarse graining via CLT: path length fixed, elapsed time = Δt ~ N(L*<1/v>, σt), where σt = sqrt(2*λ*L*(1-2/π))/σv
    const auto meanElapsedTime{fMultipleScatteringPathLength * fMeanInverseVelocity};
    const auto sigmaElapsedTime{sqrt2LambdaL * std::sqrt(1 - 2 / pi) / fSigmaVelocity1D};
    auto elapsedTime{meanElapsedTime + sigmaElapsedTime * gausXYZT.t()};
    // Coarse graining via CLT: path length fixed, displacement = Δx ~ N^3(0, σx), where σx = sqrt(2/3*λ*L)
    const auto displacement{sqrt2LambdaL * inv_sqrt3 * gausXYZT.vect()};
    // Sample final velocity and direction, and propose the new state of the particle.
    // The position is overridden here; we do NOT call G4SafetyHelper::ReLocateWithinVolume: G4Transportation's
    // forced PostStepDoIt runs every step and relocates the navigator to this overridden position (within-volume,
    // since the displacement is bounded by the real safety). The recorded step length is the true path length.
    const auto [velocity, direction]{ThermalVelocity(fSigmaVelocity1D)};
    fParticleChange.ProposeTrueStepLength(fMultipleScatteringPathLength);
    fParticleChange.ProposeTimeIncrement(elapsedTime);
    fParticleChange.ProposePosition(track.GetPosition() + displacement);
    fParticleChange.ProposeVelocityDirection(direction);
    fParticleChange.ProposeVelocity(velocity);
}

auto MuoniumThermalScattering::FullThermalScatteringInFineStructureVolume(const G4Track& track) -> void {
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
    Random::Exponential expo;
    Random::Gaussian3D<Vec3> gaus3D;
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
    auto outerFreePathLeft{expo(fRNG, fOuterMeanFreePath)};                     // the free path left at outside of fine structure volume
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
            geomFSC = (displacement - binaryOut).norm();
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
        // Within limits, check whether the position is inside the fine structure, and handle the case that
        // the position is outside the fine structure but still inside the fine structure volume.
        const auto insideFineStructure{fActiveFineStructure->FineStructureSubterm(position)};
        if (insideFineStructure) {
            freePath = expo(fRNG, currentInteractionLength);
        } else {
            // The strategy here is accurate for the case that mean free path in outer material (fOuterMeanFreePath) is much larger than
            // that of fine structure (currentInteractionLength). E.g. producing muonium in vacuum from laser-ablated silica aerogel.
            // If fOuterMeanFreePath is comparable to or smaller than currentInteractionLength this will work, and the result is
            // as if the denser outer material somewhat diffuse into the fine structure material and both reduces mean free path
            // and changes the temperature near the fine structure boundary.
            outerFreePathLeft -= freePath;
            if (outerFreePathLeft > 0 or outerMeanFreePathHuge) {
                continue; // no scattering outside the fine structure, just continue
            }
            stepForward(outerFreePathLeft); // move back
            if (outerMeanFreePathTiny) [[unlikely]] {
                fParticleChange.ProposeTrackStatus(fStopButAlive);
                break; // outer mean free path is too small, stop the flight
            }
            outerFreePathLeft = expo(fRNG, fOuterMeanFreePath);
        }
        // Sample thermal velocity
        do {
            direction = gaus3D(fRNG);
            velocity = direction.squaredNorm();
        } while (velocity == 0);
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

auto MuoniumThermalScattering::ThermalVelocity(double sigmaVelocity1D) -> std::pair<double, Vector3D> {
    double velocity;
    Vector3D direction;
    do {
        direction = Random::Gaussian3D<Vector3D>{}(fRNG);
        velocity = direction.mag2();
    } while (velocity == 0);
    velocity = std::sqrt(velocity);
    direction *= 1 / velocity; // do not use CLHEP::Hep3Vector::operator/=(double).
    velocity *= sigmaVelocity1D;
    return {velocity, direction};
}

auto MuoniumThermalScattering::MeanFreePathIn(const G4Material& material) const -> double {
    if (const auto mpt{material.GetMaterialPropertiesTable()};
        mpt and mpt->ConstPropertyExists("MUONIUM_MFP")) {
        const auto mfp{mpt->GetConstProperty("MUONIUM_MFP")};
        if (mfp <= fgSpatialTiny) [[unlikely]] {
            PrintWarning(fmt::format("Mean free path is too small ({} nm), particle will be stopped.", mfp / nm));
        }
        return mfp;
    } else if (SimilarToAir(material)) {
        using namespace LiteralUnit;
        return 135_nm * 1.2_kg_m3 / material.GetDensity();
    }
    return 0;
}

auto MuoniumThermalScattering::SimilarToAir(const G4Material& material) const -> bool {
    if (&material == fAirMaterial) {
        return true;
    }
    if (material.GetState() != kStateGas) {
        return false;
    }
    int nAirElementMatched{}; // material elements are distinct in Z, so counting matches suffices
    const auto fractionVector{material.GetFractionVector()};
    const auto nMaterialElement{static_cast<int>(material.GetNumberOfElements())};
    for (int i{}; i < nMaterialElement; ++i) {
        const auto fraction{fractionVector[i]};
        if (const auto found{fAirComposition.find(material.GetElement(i)->GetZ())};
            found != fAirComposition.end()) {
            if (not muc::isclose(fraction, found->second, {.rel = 0.01})) {
                return false; // an air element with mismatched fraction
            }
            ++nAirElementMatched;
        } else if (fraction >= 0.001) {
            return false; // a non-trace element absent from air
        }
    }
    return nAirElementMatched == ssize(fAirComposition); // all air elements must be present
}

auto MuoniumThermalScattering::SigmaVelocity1D(double temperature, const G4ParticleDefinition& particle) -> double {
    using namespace LiteralUnit::Velocity;
    using namespace PhysicalConstant;
    const auto sigma{std::sqrt(c_squared * k_Boltzmann * temperature / particle.GetPDGMass())}; // sqrt(kT/m) = sqrt(c^2kT/mc^2)
    if (sigma <= muc::default_abs_tol<double>) [[unlikely]] {
        Throw<std::runtime_error>(fmt::format("Sigma of 1D velocity is too small ({} m/s)", sigma / 1_m_s));
    }
    return sigma;
}

} // namespace Mustard::Geant4X::inline Process
