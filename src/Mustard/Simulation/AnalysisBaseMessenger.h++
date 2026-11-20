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

#include "Mustard/Geant4X/Interface/SingletonMessenger.h++"

#include "G4UIcmdWithAString.hh"
#include "G4UIdirectory.hh"

#include "muc/concepts"

#include <concepts>
#include <memory>
#include <string_view>

namespace Mustard::Simulation {

template<typename AReceiver>
class AnalysisBaseMessenger final : public Geant4X::SingletonMessenger<AnalysisBaseMessenger<AReceiver>,
                                                                       AReceiver> {
    friend Mustard::Env::SingletonFactory;

private:
    AnalysisBaseMessenger();
    ~AnalysisBaseMessenger() = default;

public:
    auto SetNewValue(G4UIcommand* command, G4String value) -> void override;

private:
    std::unique_ptr<G4UIdirectory> fDirectory;
    std::unique_ptr<G4UIcmdWithAString> fFilePath;
    std::unique_ptr<G4UIcmdWithAString> fFileMode;
};

} // namespace Mustard::Simulation

#include "Mustard/Simulation/AnalysisBaseMessenger.inl"
