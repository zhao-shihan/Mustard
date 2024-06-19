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
class G4UIcmdWithAnInteger;
class G4UIdirectory;

namespace Mustard::inline Extension::Geant4X::inline DecayChannel {

class MuonInternalPairProductionDecayChannel;

class MuonInternalPairProductionDecayChannelMessenger final : public Geant4X::SingletonMessenger<MuonInternalPairProductionDecayChannelMessenger,
                                                                                                 MuonInternalPairProductionDecayChannel> {
    friend class Env::Memory::SingletonInstantiator;

private:
    MuonInternalPairProductionDecayChannelMessenger();
    ~MuonInternalPairProductionDecayChannelMessenger();

public:
    auto SetNewValue(G4UIcommand* command, G4String value) -> void override;

private:
    std::unique_ptr<G4UIdirectory> fDirectory;
    std::unique_ptr<G4UIcmdWithADouble> fMetropolisDelta;
    std::unique_ptr<G4UIcmdWithAnInteger> fMetropolisDiscard;
};

} // namespace Mustard::inline Extension::Geant4X::inline DecayChannel
