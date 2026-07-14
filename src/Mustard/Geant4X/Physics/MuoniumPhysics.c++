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

#include "Mustard/Geant4X/Particle/Antimuonium.h++"
#include "Mustard/Geant4X/Particle/Muonium.h++"
#include "Mustard/Geant4X/Physics/MuoniumPhysics.h++"
#include "Mustard/Geant4X/Process/MuoniumFormation.h++"
#include "Mustard/Geant4X/Process/MuoniumThermalScattering.h++"

#include "G4EmBuilder.hh"
#include "G4MuonMinus.hh"
#include "G4MuonPlus.hh"
#include "G4ProcessManager.hh"

namespace Mustard::Geant4X::inline Physics {

MuoniumPhysics::MuoniumPhysics(int verbose) :
    G4VPhysicsConstructor{"MuoniumPhysics"},
    fFineStructure{} {
    verboseLevel = verbose;
}

auto MuoniumPhysics::AddFineStructure(const FineStructureForAtomThermalScattering* fineStructure) -> void {
    if (fineStructure) {
        fFineStructure.push_back(fineStructure);
    }
}

auto MuoniumPhysics::ConstructParticle() -> void {
    G4EmBuilder::ConstructMinimalEmSet();
    G4MuonMinus::Definition();
    G4MuonPlus::Definition();
    Muonium::Definition();
    Antimuonium::Definition();
}

auto MuoniumPhysics::ConstructProcess() -> void {
    const auto formation{new MuoniumFormation};
    const auto thermalScattering{new MuoniumThermalScattering};
    for (auto&& fs : std::as_const(fFineStructure)) {
        thermalScattering->AddFineStructure(fs);
    }

    const auto muonPlus{G4MuonPlus::Definition()->GetProcessManager()};
    muonPlus->AddRestProcess(formation);

    const auto muonium{Muonium::Definition()->GetProcessManager()};
    muonium->AddContinuousProcess(thermalScattering);

    const auto antiMuonium{Antimuonium::Definition()->GetProcessManager()};
    antiMuonium->AddContinuousProcess(thermalScattering);
}

} // namespace Mustard::Geant4X::inline Physics
