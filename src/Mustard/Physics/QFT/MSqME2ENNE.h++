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

#include "Mustard/Physics/QFT/MatrixElement.h++"
#include "Mustard/Utility/FunctionAttribute.h++"

namespace Mustard::inline Physics::QFT {

/// @class MSqME2ENNE
/// @brief Matrix element squared for μ⁻e⁺ → e⁻ννe⁺ and μ⁺e⁻ → e⁺ννe⁻ process.
/// Implements unpolarized matrix element.
class MSqME2ENNE : public MatrixElement<2, 4> {
public:
    /// @brief Matrix element version
    enum struct Ver {
        QEDTree2D, // QED 2 tree diagrams
        QEDTree4D  // QED 4 tree diagrams
    };

public:
    /// @brief The constructor
    /// @param ver the matrix element version
    MSqME2ENNE(Ver ver = Ver::QEDTree2D);

    /// @brief Set matrix element version
    /// @param ver The matrix element version
    auto Version(Ver ver) -> void { fVersion = ver; }

    /// @brief Calculate squared matrix element for internal conversion muon decay
    /// @param pI Initial state 4-momenta:
    ///          [μ⁻, e⁺] → e⁻ννe⁺
    ///          [μ⁻, e⁺] → e⁺ννe⁻
    /// @param pF Final state momenta:
    ///          μ⁻e⁺ → [e⁻, ν, ν, e⁺]
    ///          μ⁻e⁺ → [e⁺, ν, ν, e⁻]
    /// @return |M|² value in CLHEP unit system
    ///
    /// @note Implementation based on McMule's analytical expressions
    virtual auto operator()(const InitialStateMomenta& pI, const FinalStateMomenta& pF) const -> double override;

private:
    MUSTARD_OPTIMIZE_FAST static auto MSqQEDTree2D(double mMuSq, double mESq,
                                                   double s12, double s13, double s14, double s15, double s16,
                                                   double s23, double s24, double s25, double s26,
                                                   double s34, double s35, double s36,
                                                   double s45, double s46,
                                                   double s56) -> double;
    MUSTARD_OPTIMIZE_FAST static auto MSqQEDTree4D(double mMuSq, double mESq,
                                                   double s12, double s13, double s14, double s15, double s16,
                                                   double s23, double s24, double s25, double s26,
                                                   double s34, double s35, double s36,
                                                   double s45, double s46,
                                                   double s56) -> double;

private:
    Ver fVersion;
};

} // namespace Mustard::inline Physics::QFT
