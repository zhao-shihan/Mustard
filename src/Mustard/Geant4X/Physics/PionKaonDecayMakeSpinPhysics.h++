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

#pragma once

#include "G4VPhysicsConstructor.hh"

namespace Mustard::Geant4X::inline Physics {

class PionKaonDecayMakeSpinPhysics : public G4VPhysicsConstructor {
public:
    PionKaonDecayMakeSpinPhysics(G4int verbose);

    virtual auto ConstructParticle() -> void override;
    virtual auto ConstructProcess() -> void override;
};

} // namespace Mustard::Geant4X::inline Physics
