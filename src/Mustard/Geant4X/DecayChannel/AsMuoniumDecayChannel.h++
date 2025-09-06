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

#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/IO/Print.h++"
#include "Mustard/Utility/LiteralUnit.h++"
#include "Mustard/Utility/PhysicalConstant.h++"

#include "G4DecayProducts.hh"
#include "G4ParticleDefinition.hh"
#include "G4RandomDirection.hh"
#include "G4String.hh"
#include "G4VDecayChannel.hh"
#include "Randomize.hh"

#include "muc/ceta_string"
#include "muc/math"
#include "muc/numeric"

#include "gsl/gsl"

#include "fmt/format.h"

#include <cmath>
#include <concepts>
#include <cstdio>
#include <stdexcept>
#include <vector>

namespace Mustard::Geant4X::inline DecayChannel {

template<std::derived_from<G4VDecayChannel> AMuonDecayChannel, muc::ceta_string AName>
class AsMuoniumDecayChannel : public AMuonDecayChannel {
public:
    AsMuoniumDecayChannel(const G4String& parentName, G4double br, G4int verbose = 1);

    auto DecayIt(G4double) -> G4DecayProducts* override;

private:
    int fAtomicShellProductIndex;
};

} // namespace Mustard::Geant4X::inline DecayChannel

#include "Mustard/Geant4X/DecayChannel/AsMuoniumDecayChannel.inl"
