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

#include "Mustard/Physics/QFT/MSqME2ENNE.h++"
#include "Mustard/Physics/QFT/MatrixElement.h++"

namespace Mustard::inline Physics::QFT {

/// @class MSqM2ENNE
/// @brief Matrix element squared for M(μ⁺e⁻) → e⁺ννe⁻ process.
/// Implements unpolarized matrix element. Bound state effects are neglected
class MSqM2ENNE : public MatrixElement<1, 4> {
public:
    /// @brief Matrix element version
    using Ver = enum MSqME2ENNE::Ver;

public:
    /// @brief The constructor
    /// @param ver Matrix element version
    MSqM2ENNE(Ver ver = Ver::QEDTree2D);

    /// @brief Set matrix element version
    /// @param ver The matrix element version
    auto Version(Ver ver) -> void { fMSqME2ENNE.Version(ver); }

    /// @brief Calculate squared matrix element for internal conversion muon decay
    /// @param pI Muonium initial state 4-momenta
    /// @param pF Final state momenta: [e⁺, ν, ν, e⁻]
    /// @return |M|² value in CLHEP unit system
    ///
    /// @note Implementation based on McMule's analytical expressions
    virtual auto operator()(const InitialStateMomenta& pI, const FinalStateMomenta& pF) const -> double override;

private:
    MSqME2ENNE fMSqME2ENNE;
};

} // namespace Mustard::inline Physics::QFT
