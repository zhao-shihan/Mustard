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

#include "Mustard/Extension/Geant4X/Decay/ExtendedDecayWithSpin.h++"
#include "Mustard/Utility/MathConstant.h++"

#include "G4DecayTable.hh"
#include "G4Field.hh"
#include "G4FieldManager.hh"
#include "G4PropagatorInField.hh"
#include "G4Step.hh"
#include "G4SystemOfUnits.hh"
#include "G4Track.hh"
#include "G4Transform3D.hh"
#include "G4TransportationManager.hh"
#include "G4Vector3D.hh"

namespace Mustard::inline Extension::Geant4X::inline Decay {

using namespace Mustard::MathConstant;

ExtendedDecayWithSpin::ExtendedDecayWithSpin(const G4String& processName) :
    ExtendDecayIt{processName} {}

// Since G4DecayWithSpin explicitly use G4Decay::DecayIt instead of virtual DecayIt call,
// we have no choice but copy the whole implementation, to properly bias the decay.
// Lines marked with /* ++ */ are added or modified by us.
auto ExtendedDecayWithSpin::PostStepDoIt(const G4Track& aTrack, const G4Step& aStep) -> G4VParticleChange* {
    // ********************************************************************
    // * License and Disclaimer                                           *
    // *                                                                  *
    // * The  Geant4 software  is  copyright of the Copyright Holders  of *
    // * the Geant4 Collaboration.  It is provided  under  the terms  and *
    // * conditions of the Geant4 Software License,  included in the file *
    // * LICENSE and available at  http://cern.ch/geant4/license .  These *
    // * include a list of copyright holders.                             *
    // *                                                                  *
    // * Neither the authors of this software system, nor their employing *
    // * institutes,nor the agencies providing financial support for this *
    // * work  make  any representation or  warranty, express or implied, *
    // * regarding  this  software system or assume any liability for its *
    // * use.  Please see the license in the file  LICENSE  and URL above *
    // * for the full disclaimer and the limitation of liability.         *
    // *                                                                  *
    // * This  code  implementation is the result of  the  scientific and *
    // * technical work of the GEANT4 collaboration.                      *
    // * By using,  copying,  modifying or  distributing the software (or *
    // * any work based  on the software)  you  agree  to acknowledge its *
    // * use  in  resulting  scientific  publications,  and indicate your *
    // * acceptance of all terms of the Geant4 Software license.          *
    // ********************************************************************

    if ((aTrack.GetTrackStatus() == fStopButAlive) ||
        (aTrack.GetTrackStatus() == fStopAndKill)) {
        fParticleChangeForDecay.Initialize(aTrack);
        return &fParticleChangeForDecay;
    }

    // get particle
    const G4DynamicParticle* aParticle = aTrack.GetDynamicParticle();
    const G4ParticleDefinition* aParticleDef = aParticle->GetDefinition();

    // get parent_polarization
    G4ThreeVector parent_polarization = aParticle->GetPolarization();

    if (parent_polarization == G4ThreeVector(0, 0, 0)) {
        // Generate random polarization direction
        G4double cost = 1. - 2. * G4UniformRand();
        G4double sint = std::sqrt((1. - cost) * (1. + cost));

        G4double phi = 2 * pi * G4UniformRand(); /* ++ */
        G4double sinp = std::sin(phi);
        G4double cosp = std::cos(phi);

        G4double px = sint * cosp;
        G4double py = sint * sinp;
        G4double pz = cost;

        parent_polarization.setX(px);
        parent_polarization.setY(py);
        parent_polarization.setZ(pz);
    }

    // decay table
    G4DecayTable* decaytable = aParticleDef->GetDecayTable();
    if (decaytable != nullptr) {
        for (G4int ip = 0; ip < decaytable->entries(); ip++) {
            decaytable->GetDecayChannel(ip)->SetPolarization(parent_polarization);
        }
    }

    auto pParticleChangeForDecay{static_cast<G4ParticleChangeForDecay*>(DecayIt(aTrack, aStep))}; /* ++ */
    pParticleChangeForDecay->ProposePolarization(parent_polarization);

    return pParticleChangeForDecay;
}

namespace internal {
namespace {

// A crazy access to private G4DecayWithSpin::Spin_Precession
template<auto (G4DecayWithSpin::*F)(const G4Step&, G4ThreeVector, G4double)->G4ThreeVector>
struct StealSpinPrecession {
    friend auto SpinPrecession(G4DecayWithSpin& self, const G4Step& step, G4ThreeVector B, G4double dt) -> G4ThreeVector {
        return (self.*F)(step, B, dt);
    }
};
template struct StealSpinPrecession<&G4DecayWithSpin::Spin_Precession>;
auto SpinPrecession(G4DecayWithSpin& self, const G4Step& step, G4ThreeVector B, G4double dt) -> G4ThreeVector;

} // namespace
} // namespace internal

// Since G4DecayWithSpin explicitly use G4Decay::DecayIt instead of virtual DecayIt call,
// we have no choice but copy the whole implementation, to properly bias the decay.
// Lines marked with /* ++ */ are added or modified by us.
auto ExtendedDecayWithSpin::AtRestDoIt(const G4Track& aTrack, const G4Step& aStep) -> G4VParticleChange* {
    // ********************************************************************
    // * License and Disclaimer                                           *
    // *                                                                  *
    // * The  Geant4 software  is  copyright of the Copyright Holders  of *
    // * the Geant4 Collaboration.  It is provided  under  the terms  and *
    // * conditions of the Geant4 Software License,  included in the file *
    // * LICENSE and available at  http://cern.ch/geant4/license .  These *
    // * include a list of copyright holders.                             *
    // *                                                                  *
    // * Neither the authors of this software system, nor their employing *
    // * institutes,nor the agencies providing financial support for this *
    // * work  make  any representation or  warranty, express or implied, *
    // * regarding  this  software system or assume any liability for its *
    // * use.  Please see the license in the file  LICENSE  and URL above *
    // * for the full disclaimer and the limitation of liability.         *
    // *                                                                  *
    // * This  code  implementation is the result of  the  scientific and *
    // * technical work of the GEANT4 collaboration.                      *
    // * By using,  copying,  modifying or  distributing the software (or *
    // * any work based  on the software)  you  agree  to acknowledge its *
    // * use  in  resulting  scientific  publications,  and indicate your *
    // * acceptance of all terms of the Geant4 Software license.          *
    // ********************************************************************

    // get particle
    const G4DynamicParticle* aParticle = aTrack.GetDynamicParticle();
    const G4ParticleDefinition* aParticleDef = aParticle->GetDefinition();

    // get parent_polarization
    G4ThreeVector parent_polarization = aParticle->GetPolarization();

    if (parent_polarization == G4ThreeVector(0, 0, 0)) {
        // Generate random polarization direction
        G4double cost = 1. - 2. * G4UniformRand();
        G4double sint = std::sqrt((1. - cost) * (1. + cost));

        G4double phi = 2 * pi * G4UniformRand(); /* ++ */
        G4double sinp = std::sin(phi);
        G4double cosp = std::cos(phi);

        G4double px = sint * cosp;
        G4double py = sint * sinp;
        G4double pz = cost;

        parent_polarization.setX(px);
        parent_polarization.setY(py);
        parent_polarization.setZ(pz);

    } else {

        G4FieldManager* fieldMgr = aStep.GetTrack()->GetVolume()->GetLogicalVolume()->GetFieldManager();
        if (fieldMgr == nullptr) {
            G4TransportationManager* transportMgr =
                G4TransportationManager::GetTransportationManager();
            G4PropagatorInField* fFieldPropagator =
                transportMgr->GetPropagatorInField();
            if (fFieldPropagator) fieldMgr =
                                      fFieldPropagator->GetCurrentFieldManager();
        }

        const G4Field* field = nullptr;
        if (fieldMgr != nullptr) field = fieldMgr->GetDetectorField();

        if (field != nullptr) {
            G4double point[4];
            point[0] = (aStep.GetPostStepPoint()->GetPosition())[0];
            point[1] = (aStep.GetPostStepPoint()->GetPosition())[1];
            point[2] = (aStep.GetPostStepPoint()->GetPosition())[2];
            point[3] = aTrack.GetGlobalTime();

            G4double fieldValue[6] = {0., 0., 0., 0., 0., 0.};
            field->GetFieldValue(point, fieldValue);
            G4ThreeVector B(fieldValue[0], fieldValue[1], fieldValue[2]);

            // Call the spin precession only for non-zero mag. field
            if (B.mag2() > 0.) parent_polarization =
                                   internal::SpinPrecession(*this, aStep, B, fRemainderLifeTime); /* ++ */
        }
    }

    // decay table
    G4DecayTable* decaytable = aParticleDef->GetDecayTable();
    if (decaytable != nullptr) {
        for (G4int ip = 0; ip < decaytable->entries(); ip++) {
            decaytable->GetDecayChannel(ip)->SetPolarization(parent_polarization);
        }
    }

    auto pParticleChangeForDecay{static_cast<G4ParticleChangeForDecay*>(DecayIt(aTrack, aStep))}; /* ++ */
    pParticleChangeForDecay->ProposePolarization(parent_polarization);

    return pParticleChangeForDecay;
}

} // namespace Mustard::inline Extension::Geant4X::inline Decay
