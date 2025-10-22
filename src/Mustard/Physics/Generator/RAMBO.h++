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

#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Physics/Generator/VersatileEventGenerator.h++"
#include "Mustard/Utility/FunctionAttribute.h++"

#include "CLHEP/Units/SystemOfUnits.h"
#include "CLHEP/Vector/LorentzVector.h"

#include "muc/math"
#include "muc/numeric"

#include "fmt/core.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <utility>

namespace Mustard::inline Physics::inline Generator {

/// @brief N-body phase space generator using RAMBO algorithm
///
/// Implements the RAMBO algorithm for generating N-particle phase space events.
/// Based on:
///   R. Kleiss, W.J. Stirling, S.D. Ellis, "A New Monte Carlo Treatment Of
///   Multiparticle Phase Space At High-Energies", CPC40 (1986) 359.
///
/// This generator is very suitable for generating unweighted massless
/// final states.
///
/// Complexity: O(N), but typically (e.g. N < 10) slower than GENBOD when
/// final states are massive. RAMBO will be faster with massless final states.
///
/// However, RAMBO has following good properties:
///  - For massless final states, RAMBO can generate weight=1 events.
///  - For near-massless final states RAMBO can generate weight~1 events.
///
/// @tparam M Number of initial-state particles (M ≥ 1)
/// @tparam N Number of final-state particles (N ≥ 2)
template<int M, int N>
    requires(N >= 2)
class RAMBO : public VersatileEventGenerator<M, N, 4 * N> {
public:
    /// @brief Initial-state 4-momentum
    using typename VersatileEventGenerator<M, N, 4 * N>::InitialStateMomenta;
    /// @brief Final-state 4-momentum container type
    using typename VersatileEventGenerator<M, N, 4 * N>::FinalStateMomenta;
    /// @brief Generated event type
    using typename VersatileEventGenerator<M, N, 4 * N>::Event;
    /// @brief Random state container type
    using typename VersatileEventGenerator<M, N, 4 * N>::RandomState;

public:
    /// @brief Construct RAMBO phase space generator
    /// @param pdgID Array of final-state particle PDG IDs (index order preserved)
    /// @param mass Array of final-state particle masses (index order preserved)
    RAMBO(const std::array<int, N>& pdgID, const std::array<double, N>& mass);

    /// @brief Generate event in c.m. frame using precomputed random numbers
    /// @param u Flat random numbers in 0--1
    /// @param pI Initial-state 4-momenta
    /// @return Generated event
    virtual auto operator()(const RandomState& u, InitialStateMomenta pI) -> Event override;
    // Inherit operator() overloads
    using VersatileEventGenerator<M, N, 4 * N>::operator();

private:
    std::array<double, N> fWeightFactor;
};

} // namespace Mustard::inline Physics::inline Generator

#include "Mustard/Physics/Generator/RAMBO.inl"
