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

#pragma once

#include "Mustard/Utility/NonMoveableBase.h++"

#include "G4VPhysicsConstructor.hh"

#include "gsl/gsl"

class G4DecayTable;
class G4ParticleDefinition;
class G4String;

namespace Mustard::inline Extension::Geant4X::inline Physics {

class DecayPhysicsBase : public NonMoveableBase,
                         public G4VPhysicsConstructor {
public:
    using G4VPhysicsConstructor::G4VPhysicsConstructor;

    virtual auto UpdateDecayBR() -> void = 0;
    virtual auto ConstructParticle() -> void = 0;
    virtual auto ConstructProcess() -> void = 0;

protected:
    auto UpdateDecayBRFor(const G4ParticleDefinition* particle) -> void;
    virtual auto InsertDecayChannel(const G4String& parentName, gsl::not_null<G4DecayTable*> decay) -> void = 0;
    virtual auto AssignRareDecayBR(gsl::not_null<G4DecayTable*> decay) -> void = 0;
};

} // namespace Mustard::inline Extension::Geant4X::inline Physics
