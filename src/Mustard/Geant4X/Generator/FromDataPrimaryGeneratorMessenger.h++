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

#include "Mustard/Geant4X/Interface/SingletonMessenger.h++"

#include <memory>
#include <string_view>
#include <utility>

class G4UIcmdWithAnInteger;
class G4UIcommand;
class G4UIdirectory;

namespace Mustard::Geant4X::inline Generator {

class FromDataPrimaryGenerator;

class FromDataPrimaryGeneratorMessenger final : public Geant4X::SingletonMessenger<FromDataPrimaryGeneratorMessenger,
                                                                                   FromDataPrimaryGenerator> {
    friend Env::Memory::SingletonInstantiator;

private:
    FromDataPrimaryGeneratorMessenger();
    ~FromDataPrimaryGeneratorMessenger();

public:
    auto SetNewValue(G4UIcommand* command, G4String value) -> void override;

private:
    static auto ParseFileNameAndObjectName(std::string_view value) -> std::pair<std::string_view, std::string_view>;

private:
    std::unique_ptr<G4UIdirectory> fDirectory;
    std::unique_ptr<G4UIcommand> fEventData;
    std::unique_ptr<G4UIcmdWithAnInteger> fNVertex;
};

} // namespace Mustard::Geant4X::inline Generator
