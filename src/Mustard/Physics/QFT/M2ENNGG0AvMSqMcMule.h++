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

#include "Mustard/Physics/QFT/PolarizedMatrixElement.h++"
#include "Mustard/Utility/FunctionAttribute.h++"

namespace Mustard::inline Physics::QFT {

/// @class M2ENNGG0AvMSqMcMule
/// @brief Matrix element squared for μ⁻ → e⁻ννγγ and μ⁺ → e⁺ννγγ decays.
/// Neutrino energies are averaged over.
///
/// Implements polarized matrix element calculation for double radiative muon decay
/// referenceing McMule's analytical formula.
class M2ENNGG0AvMSqMcMule : public PolarizedMatrixElement<1, 5> {
public:
    using PolarizedMatrixElement::PolarizedMatrixElement;

    /// @brief Calculate squared matrix element for double radiative muon decay
    /// @param pI Muon 4-momentum
    /// @param pF Final state 4-momenta: [e, ν, ν, γ₁, γ₂]
    /// @return |M|² value in CLHEP unit system
    ///
    /// @note Implementation based on McMule's analytical expressions
    MUSTARD_OPTIMIZE_FAST virtual auto operator()(const InitialStateMomenta& pI, const FinalStateMomenta& pF) const -> double override;

private:
    MUSTARD_OPTIMIZE_FAST static auto Unpolarized(double mm2, double me2, double s12, double s15, double s16, double s25, double s26, double s56,
                                                  double den1, double den2, double den3, double den4, double den5, double den6) -> double;
    MUSTARD_OPTIMIZE_FAST static auto PolarizedS2n(double mm2, double me2, double s12, double s15, double s16, double s25, double s26, double s56,
                                                   double den1, double den2, double den3, double den4, double den5, double den6) -> double;
    MUSTARD_OPTIMIZE_FAST static auto PolarizedS5n(double mm2, double me2, double s12, double s15, double s16, double s25, double s26, double s56,
                                                   double den1, double den2, double den3, double den4, double den5, double den6) -> double;
    MUSTARD_OPTIMIZE_FAST static auto PolarizedS6n(double mm2, double me2, double s12, double s15, double s16, double s25, double s26, double s56,
                                                   double den1, double den2, double den3, double den4, double den5, double den6) -> double;
};

} // namespace Mustard::inline Physics::QFT
