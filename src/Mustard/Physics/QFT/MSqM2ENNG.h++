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

#include "Mustard/Physics/QFT/PolarizedMatrixElement.h++"
#include "Mustard/Utility/FunctionAttribute.h++"

namespace Mustard::inline Physics::QFT {

/// @class MSqM2ENNG
/// @brief Matrix element squared for μ⁻ → e⁻ννγ and μ⁺ → e⁺ννγ decays.
/// Neutrino momenta are averaged over.
///
/// Implements polarized matrix element calculation for radiative muon decay
/// referenceing McMule's analytical formula.
class MSqM2ENNG : public PolarizedMatrixElement<1, 4> {
public:
    // Inherit constructor
    using PolarizedMatrixElement::PolarizedMatrixElement;

    /// @brief Calculate squared matrix element for radiative muon decay
    /// @param pI Muon 4-momentum
    /// @param pF Final state 4-momenta: [e, ν, ν, γ]
    /// @return |M|² value in CLHEP unit system
    ///
    /// @note Implementation based on McMule's analytical expressions
    virtual auto operator()(const InitialStateMomenta& pI, const FinalStateMomenta& pF) const -> double override;

private:
    MUSTARD_OPTIMIZE_FAST static auto PM2ENNGav(double mm, double me2, double p1p2, double p1p4,
                                                double p2p4, double np2, double np4) -> double;
};

} // namespace Mustard::inline Physics::QFT
