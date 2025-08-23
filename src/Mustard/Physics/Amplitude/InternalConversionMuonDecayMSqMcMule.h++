// -*- C++ -*-
//
// Copyright 2020-2025  The Mustard development team
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

#include "Mustard/Physics/Amplitude/PolarizedSquaredAmplitude.h++"

namespace Mustard::inline Physics::inline Amplitude {

/// @class InternalConversionMuonDecayMSqMcMule
/// @brief Matrix element squared for μ⁻ → e⁻ννe⁺e⁻ and μ⁺ → e⁺ννe⁻e⁺ decays
///
/// Implements polarized matrix element squared for muon decay with internal conversion
/// (radiative decay where virtual photon converts to e⁺e⁻ pair). Referenceing
/// McMule's analytical formula.
class InternalConversionMuonDecayMSqMcMule : public PolarizedSquaredAmplitude<1, 5> {
public:
    using PolarizedSquaredAmplitude::PolarizedSquaredAmplitude; ///< Inherit constructors

    /// @brief Calculate squared matrix element for internal conversion muon decay
    /// @param pI Muon 4-momentum
    /// @param pF Final state momenta:
    ///          μ⁻ decay: [e⁻, ν, ν, e⁺, e⁻]
    ///          μ⁺ decay: [e⁺, ν, ν, e⁻, e⁺]
    /// @return |M|² value in CLHEP unit system
    ///
    /// @note Implementation based on McMule's analytical expressions
    virtual auto operator()(const InitialStateMomenta& pI, const FinalStateMomenta& pF) const -> double override;
};

} // namespace Mustard::inline Physics::inline Amplitude
