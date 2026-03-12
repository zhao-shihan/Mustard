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
#include "Mustard/Physics/Generator/M2ENNGGenerator.h++"
#include "Mustard/Utility/PhysicalConstant.h++"

#include "muc/math"

#include "fmt/core.h"

#include <cmath>
#include <stdexcept>
#include <utility>

namespace Mustard::inline Physics::inline Generator {

using namespace PhysicalConstant;

M2ENNGGenerator::M2ENNGGenerator(std::string_view parent, Vector3D momentum, Vector3D polarization,
                                   double softCutoff, double collinearCutoff,
                                   std::optional<double> thinningRatio, std::optional<unsigned> acfSampleSize,
                                   std::optional<double> stepSize) :
    MultipleTryMetropolisGenerator{{}, polarization, {}, {}, std::move(thinningRatio), acfSampleSize.value_or(100000), stepSize.value_or(0.1)} {
    Parent(parent);
    Momentum(momentum);
    Mass({electron_mass_c2, 0, 0, 0});
    SoftCutoff(softCutoff);
    CollinearCutoff(collinearCutoff);
}

auto M2ENNGGenerator::Parent(std::string_view parent) -> void {
    if (parent == "mu-") {
        PDGID({11, -12, 14, 22});
    } else if (parent == "mu+") {
        PDGID({-11, 12, -14, 22});
    } else {
        Throw<std::invalid_argument>(fmt::format("Parent should be mu- or mu+, got '{}'", parent));
    }
}

auto M2ENNGGenerator::Momentum(Vector3D momentum) -> void {
    const auto energy{std::sqrt(momentum.mag2() + muc::pow(muon_mass_c2, 2))};
    Momenta({energy, momentum});
}

auto M2ENNGGenerator::SoftCutoff(double softCutoff) -> void {
    MultipleTryMetropolisGenerator::SoftCutoff(3, softCutoff);
}

auto M2ENNGGenerator::CollinearCutoff(double collinearCutoff) -> void {
    MultipleTryMetropolisGenerator::CollinearCutoff({0, 3}, collinearCutoff);
}

} // namespace Mustard::inline Physics::inline Generator
