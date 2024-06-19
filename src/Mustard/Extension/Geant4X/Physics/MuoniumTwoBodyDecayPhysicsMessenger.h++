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

#include "Mustard/Extension/Geant4X/Interface/SingletonMessenger.h++"

#include <memory>

class G4UIcmdWithADouble;
class G4UIcmdWithoutParameter;
class G4UIdirectory;

namespace Mustard::inline Extension::Geant4X::inline Physics {

class MuoniumTwoBodyDecayPhysics;

class MuoniumTwoBodyDecayPhysicsMessenger final : public Geant4X::SingletonMessenger<MuoniumTwoBodyDecayPhysicsMessenger,
                                                                                     MuoniumTwoBodyDecayPhysics> {
    friend Env::Memory::SingletonInstantiator;

private:
    MuoniumTwoBodyDecayPhysicsMessenger();
    ~MuoniumTwoBodyDecayPhysicsMessenger();

public:
    auto SetNewValue(G4UIcommand* command, G4String value) -> void override;

private:
    std::unique_ptr<G4UIdirectory> fDirectory;
    std::unique_ptr<G4UIcmdWithADouble> fAnnihilationDecayBR;
    std::unique_ptr<G4UIcmdWithADouble> fM2eeDecayBR;
    std::unique_ptr<G4UIcmdWithoutParameter> fUpdateDecayBR;
};

} // namespace Mustard::inline Extension::Geant4X::inline Physics
