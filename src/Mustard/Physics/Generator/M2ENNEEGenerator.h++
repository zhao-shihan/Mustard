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

#include "Mustard/Math/Vector.h++"
#include "Mustard/Physics/Generator/MultipleTryMetropolisGenerator.h++"
#include "Mustard/Physics/QFT/MSqM2ENNEE.h++"

#include <optional>
#include <string_view>

namespace Mustard::inline Physics::inline Generator {

/// @class M2ENNEEGenerator
/// @brief MCMC generator for mu->ennee decays
/// Kinematics: μ⁻ → e⁻ ν ν e⁺ e⁻
///             μ⁺ → e⁺ ν ν e⁻ e⁺
class M2ENNEEGenerator : public MultipleTryMetropolisGenerator<1, 5, QFT::MSqM2ENNEE> {
public:
    /// @brief Construct generator for specific parent
    /// @param parent "mu-" or "mu+" (determines PDG IDs in generated event)
    /// @param momentum Muon momentum
    /// @param polarization Muon polarization vector
    /// @param thinningRatio Thinning factor (non-negative, optional, use default value if not set)
    /// @param acfSampleSize Sample size for estimation autocorrelation function (ACF) (optional, use default value if not set)
    /// @param stepSize Step size (proposal sigma) for proposal increment distribution (optional, use default value if not set)
    /// @param mSqVer The matrix element version
    M2ENNEEGenerator(std::string_view parent, Vector3D momentum, Vector3D polarization,
                     std::optional<double> thinningRatio = {}, std::optional<unsigned> acfSampleSize = {},
                     std::optional<double> stepSize = {}, std::optional<QFT::MSqM2ENNEE::Ver> mSqVer = {});

    /// @brief Set matrix element version
    /// @param mSqVer The matrix element version
    auto MSqVersion(QFT::MSqM2ENNEE::Ver mSqVer) -> void { fMatrixElement.Version(mSqVer); }

    /// @brief Set parent particle
    /// @param parent "mu-" or "mu+"
    /// @exception std::invalid_argument for invalid parent names
    auto Parent(std::string_view parent) -> void;
    /// @brief Set parent momentum
    /// @param momentum Muon momentum
    auto Momentum(Vector3D momentum) -> void;
};

} // namespace Mustard::inline Physics::inline Generator
