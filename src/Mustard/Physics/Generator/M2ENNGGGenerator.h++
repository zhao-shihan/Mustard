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
#include "Mustard/Physics/QFT/MSqM2ENNGG.h++"

#include <optional>
#include <string_view>

namespace Mustard::inline Physics::inline Generator {

/// @class M2ENNGGGenerator
/// @brief MCMC generator for mu->enngg decays
/// Kinematics: μ⁻ → e⁻ ν ν γ γ
///             μ⁺ → e⁺ ν ν γ γ
class M2ENNGGGenerator : public MultipleTryMetropolisGenerator<1, 5, QFT::MSqM2ENNGG> {
public:
    /// @brief Construct generator for specific parent
    /// @param parent "mu-" or "mu+" (determines PDG IDs in generated event)
    /// @param momentum Muon momentum
    /// @param polarization Muon polarization vector
    /// @param softCutoff Low-energy cutoff for final-state photons (in the c.m. frame)
    /// @param collinearCutoff Collinear cutoff on angles between final-state photons and e⁺/e⁻ (in the c.m. frame)
    /// @param thinningRatio Thinning factor (between 0--1, optional, use default value if not set)
    /// @param acfSampleSize Sample size for estimation autocorrelation function (ACF) (optional, use default value if not set)
    /// @param stepSize Step size (proposal sigma) for proposal increment distribution (optional, use default value if not set)
    M2ENNGGGenerator(std::string_view parent, Vector3D momentum, Vector3D polarization,
                     double softCutoff, double collinearCutoff,
                     std::optional<double> thinningRatio = {}, std::optional<unsigned> acfSampleSize = {},
                     std::optional<double> stepSize = {});

    /// @brief Set parent particle
    /// @param parent "mu-" or "mu+"
    /// @exception std::invalid_argument for invalid parent names
    auto Parent(std::string_view parent) -> void;
    /// @brief Set parent momentum
    /// @param momentum Muon momentum
    auto Momentum(Vector3D momentum) -> void;
    /// @brief Set soft cutoff for final-state photons
    /// @param softCutoff Low-energy cutoff for final-state photons (in the c.m. frame)
    auto SoftCutoff(double softCutoff) -> void;
    /// @brief Set collinear cutoff for final-state photons
    /// @param collinearCutoff Cutoff on angles between final-state photons and e⁺/e⁻ (in the c.m. frame)
    auto CollinearCutoff(double collinearCutoff) -> void;
};

} // namespace Mustard::inline Physics::inline Generator
