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

#pragma once

#include "Mustard/Physics/Generator/MultipleTryMetropolisGenerator.h++"
#include "Mustard/Physics/QFT/MSqM2ENN.h++"

#include "CLHEP/Vector/ThreeVector.h"

#include <optional>
#include <string_view>

namespace Mustard::inline Physics::inline Generator {

class M2ENNGenerator : public MultipleTryMetropolisGenerator<1, 3, QFT::MSqM2ENN> {
public:
    M2ENNGenerator(std::string_view parent, CLHEP::Hep3Vector momentum,
                   std::optional<double> thinningRatio = {}, std::optional<unsigned> acfSampleSize = {},
                   std::optional<double> stepSize = {});

    auto Parent(std::string_view parent) -> void;
    auto ParentMomentum(CLHEP::Hep3Vector momentum) -> void;
};

} // namespace Mustard::inline Physics::inline Generator
