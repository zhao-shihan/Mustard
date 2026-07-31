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
#include "Mustard/Utility/LiteralUnit.h++"
#include "Mustard/Utility/PhysicalConstant.h++"

#include "G4MuonPlus.hh"
#include "G4ParticleChange.hh"
#include "G4VRestProcess.hh"
#include "Randomize.hh"

#include "gtl/phmap.hpp"

#include <utility>

class G4Run;

namespace Mustard::Geant4X::inline Process {

class MuoniumFormation : public G4VRestProcess {
public:
    MuoniumFormation();

    auto DefaultParaMuoniumFraction(double val) -> void;

    auto IsApplicable(const G4ParticleDefinition&) -> bool override;
    auto StartTracking(G4Track*) -> void override;
    auto AtRestDoIt(const G4Track& track, const G4Step&) -> G4VParticleChange* override;

private:
    virtual auto FormationProbabilityIn(const G4Material& material) -> double;
    virtual auto ParaMuoniumFractionIn(const G4Material& material) -> double;
    auto GetMeanLifeTime(const G4Track& track, G4ForceCondition*) -> double override;

private:
    double fDefaultParaMuoniumFraction;

    std::pair<const G4Run*, int> fCurrentRun;
    gtl::flat_hash_map<const G4MaterialPropertiesTable*, double> fFormationProbabilityCache;
    gtl::flat_hash_map<const G4MaterialPropertiesTable*, double> fParaMuoniumFractionCache;

    G4ParticleChange fParticleChange;

    MuoniumPhysicsMessenger::Register<MuoniumFormation> fMessengerRegister;
};

} // namespace Mustard::Geant4X::inline Process
