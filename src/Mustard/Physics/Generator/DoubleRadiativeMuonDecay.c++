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

#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Physics/Generator/DoubleRadiativeMuonDecay.h++"
#include "Mustard/Utility/PhysicalConstant.h++"

#include "fmt/core.h"

#include <stdexcept>

namespace Mustard::inline Physics::inline Generator {

using namespace PhysicalConstant;

DoubleRadiativeMuonDecay::DoubleRadiativeMuonDecay(std::string_view parent, CLHEP::Hep3Vector polarization, BiasFunction B,
                                                   double delta, int discard) :
    MTMGenerator{muon_mass_c2, polarization, {}, {}, delta, discard} {
    Parent(parent);
    Mass({electron_mass_c2, 0, 0, 0, 0});
    Bias(std::move(B));
}

auto DoubleRadiativeMuonDecay::Parent(std::string_view parent) -> void {
    if (parent == "mu-") {
        PDGID({11, -12, 14, 22, 22});
    } else if (parent == "mu+") {
        PDGID({-11, 12, -14, 22, 22});
    } else {
        Throw<std::invalid_argument>(fmt::format("Parent should be mu- or mu+, got '{}'", parent));
    }
}

} // namespace Mustard::inline Physics::inline Generator
