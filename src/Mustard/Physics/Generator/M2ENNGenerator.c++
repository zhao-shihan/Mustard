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

#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Physics/Generator/M2ENNGenerator.h++"
#include "Mustard/Utility/PhysicalConstant.h++"

#include "muc/math"

#include "fmt/core.h"

#include <cmath>
#include <stdexcept>
#include <utility>

namespace Mustard::inline Physics::inline Generator {

using namespace PhysicalConstant;

M2ENNGenerator::M2ENNGenerator(std::string_view parent, CLHEP::Hep3Vector momentum,
                               std::optional<double> thinningRatio, std::optional<unsigned> acfSampleSize,
                               std::optional<double> stepSize) :
    MultipleTryMetropolisGenerator{{}, {}, {}, thinningRatio, acfSampleSize.value_or(100000), stepSize.value_or(0.1)} {
    Parent(parent);
    ParentMomentum(momentum);
    Mass({electron_mass_c2, 0, 0});
}

auto M2ENNGenerator::Parent(std::string_view parent) -> void {
    if (parent == "mu-") {
        PDGID({11, -12, 14});
    } else if (parent == "mu+") {
        PDGID({-11, 12, -14});
    } else {
        Throw<std::invalid_argument>(fmt::format("Parent should be mu- or mu+, got '{}'", parent));
    }
}

auto M2ENNGenerator::ParentMomentum(CLHEP::Hep3Vector momentum) -> void {
    const auto energy{std::sqrt(momentum.mag2() + muc::pow(muon_mass_c2, 2))};
    ISMomenta({energy, momentum});
}

} // namespace Mustard::inline Physics::inline Generator
