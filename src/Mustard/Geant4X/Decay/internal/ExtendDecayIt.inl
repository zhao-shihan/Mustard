// -*- C++ -*-
//
// Copyright 2020-2025  The Mustard development team
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

namespace Mustard::Geant4X::inline Decay::internal {

// To properly bias the decay, we have no choice but copy the whole implementation.
// Lines marked with /* ++ */ are added or modified by us.
template<typename Decay>
auto ExtendDecayIt<Decay>::DecayIt(const G4Track& aTrack, const G4Step&) -> G4VParticleChange* {
    static_assert(std::derived_from<Decay, G4Decay>);

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

    // The DecayIt() method returns by pointer a particle-change object.
    // Units are expressed in GEANT4 internal units.

    // Initialize ParticleChange
    //   all members of G4VParticleChange are set to equal to
    //   corresponding member in G4Track
    this->fParticleChangeForDecay.Initialize(aTrack);

    // get particle
    const G4DynamicParticle* aParticle = aTrack.GetDynamicParticle();
    const G4ParticleDefinition* aParticleDef = aParticle->GetDefinition();

    // check if  the particle is stable
    if (aParticleDef->GetPDGStable()) {
        return &this->fParticleChangeForDecay;
    }

    // check if thePreAssignedDecayProducts exists
    const G4DecayProducts* o_products = (aParticle->GetPreAssignedDecayProducts());
    G4bool isPreAssigned = (o_products != nullptr);
    G4DecayProducts* products = nullptr;

    // decay table
    G4DecayTable* decaytable = aParticleDef->GetDecayTable();

    // check if external decayer exists
    G4bool isExtDecayer = (decaytable == nullptr) && (this->pExtDecayer != nullptr);

    // Error due to NO Decay Table
    if ((decaytable == nullptr) && !isExtDecayer && !isPreAssigned) {
        if (this->GetVerboseLevel() > 0) {
            G4cout << "G4Decay::DoIt  : decay table not defined  for ";
            G4cout << aParticle->GetDefinition()->GetParticleName() << G4endl;
        }
        G4ExceptionDescription ed;
        ed << "For " << aParticle->GetDefinition()->GetParticleName()
           << " decay probability exist but decay table is not defined "
           << "- the particle will be killed;\n"
           << "    isExtDecayer: " << isExtDecayer
           << "; isPreAssigned: " << isPreAssigned;
        G4Exception("G4Decay::DecayIt ",
                    "DECAY101", JustWarning, ed);

        this->fParticleChangeForDecay.SetNumberOfSecondaries(0);
        // Kill the parent particle
        this->fParticleChangeForDecay.ProposeTrackStatus(fStopAndKill);
        this->fParticleChangeForDecay.ProposeLocalEnergyDeposit(0.0);

        this->ClearNumberOfInteractionLengthLeft();
        return &this->fParticleChangeForDecay;
    }

    if (isPreAssigned) {
        // copy decay products
        products = new G4DecayProducts(*o_products);
    } else if (isExtDecayer) {
        // decay according to external decayer
        products = this->pExtDecayer->ImportDecayProducts(aTrack);
    } else {
        // Decay according to decay table.
        // Keep trying to choose a candidate decay channel if the dynamic mass
        // of the decaying particle is below the sum of the PDG masses of the
        // candidate daughter particles.
        // This is needed because the decay table used in Geant4 is based on
        // the assumption of nominal PDG masses, but a wide resonance can have
        // a dynamic masses well below its nominal PDG masses, and therefore
        // some of its decay channels can be below the kinematical threshold.
        // Note that, for simplicity, we ignore here the possibility that
        // one or more of the candidate daughter particles can be, in turn,
        // wide resonance. However, if this is the case, and the channel is
        // accepted, then the masses of the resonance daughter particles will
        // be sampled by taking into account their widths.
        G4VDecayChannel* decaychannel = nullptr;
        G4double massParent = aParticle->GetMass();
        decaychannel = decaytable->SelectADecayChannel(massParent);
        if (decaychannel == nullptr) {
            // decay channel not found
            G4ExceptionDescription ed;
            ed << "Can not determine decay channel for "
               << aParticleDef->GetParticleName() << G4endl
               << "  mass of dynamic particle: "
               << massParent / CLHEP::GeV << " (GeV)" << G4endl
               << "  dacay table has " << decaytable->entries()
               << " entries" << G4endl;
            G4double checkedmass = massParent;
            if (massParent < 0.) {
                checkedmass = aParticleDef->GetPDGMass();
                ed << "Using PDG mass (" << checkedmass / CLHEP::GeV
                   << "(GeV)) in IsOKWithParentMass" << G4endl;
            }
            for (G4int ic = 0; ic < decaytable->entries(); ++ic) {
                G4VDecayChannel* dc = decaytable->GetDecayChannel(ic);
                ed << ic << ": BR " << dc->GetBR() << ", IsOK? "
                   << dc->IsOKWithParentMass(checkedmass)
                   << ", --> ";
                G4int ndaughters = dc->GetNumberOfDaughters();
                for (G4int id = 0; id < ndaughters; ++id) {
                    if (id > 0)
                        ed << " + "; // seperator, except for first
                    ed << dc->GetDaughterName(id);
                }
                ed << G4endl;
            }
            G4Exception("G4Decay::DoIt", "DECAY003", FatalException, ed);
        } else {
            // execute DecayIt()
#ifdef G4VERBOSE
            G4int temp = decaychannel->GetVerboseLevel();
            if (this->GetVerboseLevel() > 1) {
                G4cout << "G4Decay::DoIt  : selected decay channel  addr:"
                       << decaychannel << G4endl;
                decaychannel->SetVerboseLevel(this->GetVerboseLevel());
            }
#endif
            products = decaychannel->DecayIt(aParticle->GetMass());
            // apply weight if necessary /* ++ */
            if (const auto edc{dynamic_cast<const DecayChannelExtension*>(decaychannel)}) {      /* ++ */
                this->fParticleChangeForDecay.ProposeWeight(aTrack.GetWeight() * edc->Weight()); /* ++ */
            } /* ++ */
#ifdef G4VERBOSE
            if (this->GetVerboseLevel() > 1) {
                decaychannel->SetVerboseLevel(temp);
            }
#endif
#ifdef G4VERBOSE
            if (this->GetVerboseLevel() > 2) {
                if (!products->IsChecked())
                    products->DumpInfo();
            }
#endif
        }
    }

    // get parent particle information ...................................
    G4double ParentEnergy = aParticle->GetTotalEnergy();
    G4double ParentMass = aParticle->GetMass();
    if (ParentEnergy < ParentMass) {
        G4ExceptionDescription ed;
        ed << "Total Energy is less than its mass - increased the energy"
           << "\n Particle: " << aParticle->GetDefinition()->GetParticleName()
           << "\n Energy:" << ParentEnergy / CLHEP::MeV << "[MeV]"
           << "\n Mass:" << ParentMass / CLHEP::MeV << "[MeV]";
        G4Exception("G4Decay::DecayIt ",
                    "DECAY102", JustWarning, ed);
        ParentEnergy = ParentMass;
    }

    G4ThreeVector ParentDirection(aParticle->GetMomentumDirection());

    // boost all decay products to laboratory frame
    G4double energyDeposit = 0.0;
    G4double finalGlobalTime = aTrack.GetGlobalTime();
    G4double finalLocalTime = aTrack.GetLocalTime();
    if (aTrack.GetTrackStatus() == fStopButAlive) {
        // AtRest case
        finalGlobalTime += this->fRemainderLifeTime;
        finalLocalTime += this->fRemainderLifeTime;
        energyDeposit += aParticle->GetKineticEnergy();
        if (isPreAssigned)
            products->Boost(ParentEnergy, ParentDirection);
    } else {
        // PostStep case
        if (!isExtDecayer)
            products->Boost(ParentEnergy, ParentDirection);
    }
    // set polarization for daughter particles
    this->DaughterPolarization(aTrack, products);

    // add products in this->fParticleChangeForDecay
    G4int numberOfSecondaries = products->entries();
    this->fParticleChangeForDecay.SetNumberOfSecondaries(numberOfSecondaries);
#ifdef G4VERBOSE
    if (this->GetVerboseLevel() > 1) {
        G4cout << "G4Decay::DoIt  : Decay vertex :";
        G4cout << " Time: " << finalGlobalTime / CLHEP::ns << "[ns]";
        G4cout << " X:" << (aTrack.GetPosition()).x() / CLHEP::cm << "[cm]";
        G4cout << " Y:" << (aTrack.GetPosition()).y() / CLHEP::cm << "[cm]";
        G4cout << " Z:" << (aTrack.GetPosition()).z() / CLHEP::cm << "[cm]";
        G4cout << G4endl;
        G4cout << "G4Decay::DoIt  : decay products in Lab. Frame" << G4endl;
        products->DumpInfo();
    }
#endif
    G4int index;
    G4ThreeVector currentPosition;
    const G4TouchableHandle thand = aTrack.GetTouchableHandle();
    for (index = 0; index < numberOfSecondaries; index++) {
        // get current position of the track
        currentPosition = aTrack.GetPosition();
        // create a new track object
        G4Track* secondary = new G4Track(products->PopProducts(),
                                         finalGlobalTime,
                                         currentPosition);
        // switch on good for tracking flag
        secondary->SetGoodForTrackingFlag();
        secondary->SetTouchableHandle(thand);
        // add the secondary track in the List
        this->fParticleChangeForDecay.AddSecondary(secondary);
    }
    delete products;

    // Kill the parent particle
    this->fParticleChangeForDecay.ProposeTrackStatus(fStopAndKill);
    this->fParticleChangeForDecay.ProposeLocalEnergyDeposit(energyDeposit);
    this->fParticleChangeForDecay.ProposeLocalTime(finalLocalTime);

    // Clear NumberOfInteractionLengthLeft
    this->ClearNumberOfInteractionLengthLeft();

    return &this->fParticleChangeForDecay;
}

} // namespace Mustard::Geant4X::inline Decay::internal
