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

#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Physics/Generator/M2ENNGGGenerator.h++"
#include "Mustard/Utility/PhysicalConstant.h++"

#include "muc/math"

#include "fmt/core.h"

#include <cmath>
#include <stdexcept>
#include <utility>

namespace Mustard::inline Physics::inline Generator {

using namespace PhysicalConstant;

M2ENNGGGenerator::M2ENNGGGenerator(std::string_view parent, CLHEP::Hep3Vector momentum, CLHEP::Hep3Vector polarization, double irCut,
                                   std::optional<double> delta, std::optional<unsigned> discard) :
    ClassicalMetropolisGenerator{{}, polarization, {}, {}, delta, discard} {
    Parent(parent);
    ParentMomentum(momentum);
    Mass({electron_mass_c2, 0, 0, 0, 0});
    IRCut(irCut);
    AddIdenticalSet({3, 4});
}

auto M2ENNGGGenerator::Parent(std::string_view parent) -> void {
    if (parent == "mu-") {
        PDGID({11, -12, 14, 22, 22});
    } else if (parent == "mu+") {
        PDGID({-11, 12, -14, 22, 22});
    } else {
        Throw<std::invalid_argument>(fmt::format("Parent should be mu- or mu+, got '{}'", parent));
    }
}

auto M2ENNGGGenerator::ParentMomentum(CLHEP::Hep3Vector momentum) -> void {
    const auto energy{std::sqrt(momentum.mag2() + muc::pow(muon_mass_c2, 2))};
    ISMomenta({energy, momentum});
}

auto M2ENNGGGenerator::IRCut(double irCut) -> void {
    ClassicalMetropolisGenerator::IRCut(3, irCut);
    ClassicalMetropolisGenerator::IRCut(4, irCut);
}

} // namespace Mustard::inline Physics::inline Generator
