// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

#include "Mustard/Geant4X/Physics/DecayPhysicsBase.h++"
#include "Mustard/Geant4X/Physics/MuonNLODecayPhysicsMessenger.h++"

#include "gsl/gsl"

class G4DecayTable;
class G4String;

namespace Mustard::Geant4X::inline Physics {

class MuoniumNLODecayPhysics : public DecayPhysicsBase {
public:
    MuoniumNLODecayPhysics(G4int verbose);

    auto RadiativeDecayBR(double b) -> void { fRadiativeDecayBR = b; }

    virtual auto UpdateDecayBR() -> void override;
    virtual auto ResetDecayBR() -> void override;
    virtual auto ConstructParticle() -> void override;
    virtual auto ConstructProcess() -> void override;

protected:
    virtual auto InsertDecayChannel(const G4String& parentName, gsl::not_null<G4DecayTable*> decay) -> void override;
    virtual auto AssignMinorDecayBR(gsl::not_null<G4DecayTable*> decay) -> void override;
    virtual auto ResetMinorDecayBR(gsl::not_null<G4DecayTable*> decay) -> void override;

protected:
    double fRadiativeDecayBR;

private:
    MuonNLODecayPhysicsMessenger::Register<MuoniumNLODecayPhysics> fMessengerRegister;
};

} // namespace Mustard::Geant4X::inline Physics
