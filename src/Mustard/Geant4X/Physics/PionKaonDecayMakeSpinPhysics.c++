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

#include "Mustard/Geant4X/Decay/PionKaonDecayMakeSpin.h++"
#include "Mustard/Geant4X/Physics/PionKaonDecayMakeSpinPhysics.h++"

#include "G4DecayTable.hh"
#include "G4EmBuilder.hh"
#include "G4KaonMinus.hh"
#include "G4KaonPlus.hh"
#include "G4KaonZeroLong.hh"
#include "G4PionMinus.hh"
#include "G4PionPlus.hh"
#include "G4ProcessTable.hh"

#include <limits>
#include <stdexcept>

namespace Mustard::Geant4X::inline Physics {

PionKaonDecayMakeSpinPhysics::PionKaonDecayMakeSpinPhysics(G4int verbose) :
    G4VPhysicsConstructor{"PionKaonDecayMakeSpinPhysics"} {
    verboseLevel = verbose;
}

auto PionKaonDecayMakeSpinPhysics::ConstructParticle() -> void {
    G4EmBuilder::ConstructMinimalEmSet();
    G4PionPlus::Definition();
    G4PionMinus::Definition();
    G4KaonPlus::Definition();
    G4KaonMinus::Definition();
    G4KaonZeroLong::Definition();
}

auto PionKaonDecayMakeSpinPhysics::ConstructProcess() -> void {
    const auto RegisterDecayMakeSpin{
        [decayMakeSpin = new PionKaonDecayMakeSpin,
         processTable = G4ProcessTable::GetProcessTable()](auto&& particle) {
            const auto manager{particle->GetProcessManager()};
            if (manager == nullptr) {
                return;
            }
            const auto decay{processTable->FindProcess("Decay", particle)};
            if (decay) {
                manager->RemoveProcess(decay);
            }
            manager->AddProcess(decayMakeSpin);
            // set ordering for PostStepDoIt and AtRestDoIt
            manager->SetProcessOrdering(decayMakeSpin, idxPostStep);
            manager->SetProcessOrdering(decayMakeSpin, idxAtRest);
        }};
    RegisterDecayMakeSpin(G4PionPlus::Definition());
    RegisterDecayMakeSpin(G4PionMinus::Definition());
    RegisterDecayMakeSpin(G4KaonPlus::Definition());
    RegisterDecayMakeSpin(G4KaonMinus::Definition());
    RegisterDecayMakeSpin(G4KaonZeroLong::Definition());
}

} // namespace Mustard::Geant4X::inline Physics
