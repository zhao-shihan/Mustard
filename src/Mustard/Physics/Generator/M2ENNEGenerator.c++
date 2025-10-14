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
#include "Mustard/Physics/Generator/M2ENNEGenerator.h++"
#include "Mustard/Utility/PhysicalConstant.h++"

#include "muc/math"

#include "fmt/core.h"

#include <cmath>
#include <stdexcept>
#include <utility>

namespace Mustard::inline Physics::inline Generator {

using namespace PhysicalConstant;

M2ENNEGenerator::M2ENNEGenerator(std::string_view parent, CLHEP::Hep3Vector momentum, double irCut,
                                 std::optional<double> thinningRatio, std::optional<unsigned> acfSampleSize,
                                 std::optional<double> stepSize, std::optional<QFT::MSqM2ENNE::Ver> mSqVer) :
    MultipleTryMetropolisGenerator{{}, {}, {}, std::move(thinningRatio), acfSampleSize.value_or(40000), stepSize.value_or(0.1)} {
    if (mSqVer) {
        MSqVersion(*mSqVer);
    }
    Parent(parent);
    ParentMomentum(momentum);
    Mass({electron_mass_c2, 0, 0, electron_mass_c2});
    IRCut(irCut);
}

auto M2ENNEGenerator::Parent(std::string_view parent) -> void {
    if (parent == "muonium") {
        PDGID({-11, 12, -14, 11});
    } else if (parent == "antimuonium") {
        PDGID({11, -12, 14, -11});
    } else {
        Throw<std::invalid_argument>(fmt::format("Parent should be muonium or antimuonium, got '{}'", parent));
    }
}

auto M2ENNEGenerator::ParentMomentum(CLHEP::Hep3Vector momentum) -> void {
    const auto energy{std::sqrt(momentum.mag2() + muc::pow(muonium_mass_c2, 2))};
    ISMomenta({energy, momentum});
}

auto M2ENNEGenerator::IRCut(double irCut) -> void {
    MultipleTryMetropolisGenerator::IRCut(3, irCut);
}

} // namespace Mustard::inline Physics::inline Generator
