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

/// @class M2ENNEE0AvMSqMcMule
/// @brief Matrix element squared for μ⁻ → e⁻ννe⁺e⁻ and μ⁺ → e⁺ννe⁻e⁺ decays.
/// Neutrino energies are averaged over.
///
/// Implements polarized matrix element squared for muon decay with internal conversion
/// (radiative decay where virtual photon converts to e⁺e⁻ pair). Referenceing
/// McMule's analytical formula.
class M2ENNEE0AvMSqMcMule : public PolarizedMatrixElement<1, 5> {
public:
    using PolarizedMatrixElement::PolarizedMatrixElement; ///< Inherit constructors

    /// @brief Calculate squared matrix element for internal conversion muon decay
    /// @param pI Muon 4-momentum
    /// @param pF Final state momenta:
    ///          μ⁻ decay: [e⁻, ν, ν, e⁺, e⁻]
    ///          μ⁺ decay: [e⁺, ν, ν, e⁻, e⁺]
    /// @return |M|² value in CLHEP unit system
    ///
    /// @note Implementation based on McMule's analytical expressions
    MUSTARD_OPTIMIZE_FAST virtual auto operator()(const InitialStateMomenta& pI, const FinalStateMomenta& pF) const -> double override;

private:
    MUSTARD_OPTIMIZE_FAST static auto OneBorn(double s12, double s13, double s14, double s23, double s24, double s34,
                                              double m12, double m22, double) -> double;
    MUSTARD_OPTIMIZE_FAST static auto OneBornPol(double s12, double s13, double s14, double s23, double s24, double s34,
                                                 double m12, double m22, double,
                                                 double s2n, double s3n, double s4n) -> double;
    MUSTARD_OPTIMIZE_FAST static auto TwoBorn(double s12, double s13, double s14, double s23, double s24, double s34,
                                              double m12, double m22, double m32) -> double;
    MUSTARD_OPTIMIZE_FAST static auto TwoBornPol(double s12, double s13, double s14, double s23, double s24, double s34,
                                                 double m12, double m22, double m32,
                                                 double s2n, double s3n, double s4n) -> double;
};

} // namespace Mustard::inline Physics::QFT
