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

#include "Mustard/Physics/Generator/MultipleTryMetropolisGenerator.h++"
#include "Mustard/Physics/QFT/MSqM2ENNE.h++"

#include "CLHEP/Vector/ThreeVector.h"

#include <optional>
#include <string_view>

namespace Mustard::inline Physics::inline Generator {

/// @class M2ENNEGenerator
/// @brief MCMC generator for muonium decays. Bound state effects are neglected
/// Kinematics: M → e⁺ ν ν e⁻
///             M̅ → e⁻ ν ν e⁺
class M2ENNEGenerator : public MultipleTryMetropolisGenerator<1, 4, QFT::MSqM2ENNE> {
public:
    /// @brief Construct generator for specific parent
    /// @param parent "muonium" or "antimuonium" (determines PDG IDs in generated event)
    /// @param momentum Muonium momentum
    /// @param thinningRatio Thinning factor (between 0--1, optional, use default value if not set)
    /// @param acfSampleSize Sample size for estimation autocorrelation function (ACF) (optional, use default value if not set)
    /// @param stepSize Step size (proposal sigma) for proposal increment distribution (optional, use default value if not set)
    /// @param mSqVer The matrix element version
    M2ENNEGenerator(std::string_view parent, CLHEP::Hep3Vector momentum,
                    std::optional<double> thinningRatio = {}, std::optional<unsigned> acfSampleSize = {},
                    std::optional<double> stepSize = {}, std::optional<QFT::MSqM2ENNE::Ver> mSqVer = {});

    /// @brief Set matrix element version
    /// @param mSqVer The matrix element version
    auto MSqVersion(QFT::MSqM2ENNE::Ver mSqVer) -> void { fMatrixElement.Version(mSqVer); }

    /// @brief Set parent particle
    /// @param parent "muonium" or "antimuonium"
    /// @exception std::invalid_argument for invalid parent names
    auto Parent(std::string_view parent) -> void;
    /// @brief Set parent momentum
    /// @param momentum Muonium momentum
    auto ParentMomentum(CLHEP::Hep3Vector momentum) -> void;
};

} // namespace Mustard::inline Physics::inline Generator
