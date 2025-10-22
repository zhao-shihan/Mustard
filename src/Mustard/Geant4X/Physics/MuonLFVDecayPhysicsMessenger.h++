// -*- C++ -*-
//
// Copyright (C) 2020-2025  Mustard developers
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

#include "Mustard/Geant4X/Interface/SingletonMessenger.h++"

#include <memory>

class G4UIcmdWithADouble;

namespace Mustard::Geant4X::inline Physics {

class MuonLFVDecayPhysics;
class MuoniumLFVDecayPhysics;

class MuonLFVDecayPhysicsMessenger final : public SingletonMessenger<MuonLFVDecayPhysicsMessenger,
                                                                     MuonLFVDecayPhysics,
                                                                     MuoniumLFVDecayPhysics> {
    friend Mustard::Env::Memory::SingletonInstantiator;

private:
    MuonLFVDecayPhysicsMessenger();
    ~MuonLFVDecayPhysicsMessenger();

public:
    auto SetNewValue(G4UIcommand* command, G4String value) -> void override;

private:
    std::unique_ptr<G4UIcmdWithADouble> fDoubleRadiativeDecayBR;
};

} // namespace Mustard::Geant4X::inline Physics
