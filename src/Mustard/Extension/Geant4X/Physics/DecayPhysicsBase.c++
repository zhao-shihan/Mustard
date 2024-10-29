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

#include "Mustard/Extension/Geant4X/Physics/DecayPhysicsBase.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "G4DecayTable.hh"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace Mustard::inline Extension::Geant4X::inline Physics {

auto DecayPhysicsBase::UpdateDecayBRFor(const G4ParticleDefinition* particle) -> void {
    const auto decay{particle->GetDecayTable()};
    // set rare decay mode first
    AssignMinorDecayBR(decay);
    // then set main decay mode
    double mainDecayBR{1};
    for (auto i{1}; i < decay->entries(); ++i) {
        mainDecayBR -= decay->GetDecayChannel(i)->GetBR();
    }
    if (mainDecayBR < -std::numeric_limits<double>::epsilon()) {
        decay->DumpInfo();
        PrettyWarning("Try resetting all BRs");
        ResetDecayBR();
        throw std::runtime_error{PrettyException("Impossible to normalize decay branching ratio (sum of rare channel BR > 1), all BRs have been reset")};
    }
    decay->GetDecayChannel(0)->SetBR(std::max(0., mainDecayBR));
}

auto DecayPhysicsBase::ResetDecayBRFor(const G4ParticleDefinition* particle) -> void {
    const auto decay{particle->GetDecayTable()};
    // reset rare decay mode first
    ResetMinorDecayBR(decay);
    // then reset main decay mode
    double mainDecayBR{1};
    for (auto i{1}; i < decay->entries(); ++i) {
        mainDecayBR -= decay->GetDecayChannel(i)->GetBR();
    }
    if (mainDecayBR < -std::numeric_limits<double>::epsilon()) {
        decay->DumpInfo();
        throw std::logic_error{PrettyException("Impossible to normalize decay branching ratio (sum of rare channel BR > 1)")};
    }
    decay->GetDecayChannel(0)->SetBR(std::max(0., mainDecayBR));
}

} // namespace Mustard::inline Extension::Geant4X::inline Physics
