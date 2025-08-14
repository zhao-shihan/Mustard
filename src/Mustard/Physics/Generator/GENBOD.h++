// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
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

#include "CLHEP/Units/SystemOfUnits.h"
#include "CLHEP/Vector/LorentzVector.h"

#include "muc/math"
#include "muc/numeric"

#include "gsl/gsl"

#include "fmt/core.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <utility>

namespace Mustard::inline Physics::inline Generator {

/// @brief N-body phase space generator using GENBOD algorithm
///
/// Implements the GENBOD function (W515 from CERNLIB) for generating
/// N-particle phase space events.
/// Based on:
///   F. James, Monte Carlo Phase Space, CERN 68-15 (1968)
///
/// Algorithm steps:
///  1. Generate sorted uniform variates for invariant masses
///  2. Calculate relative momenta in sequential rest frames
///  3. Construct initial two-particle system
///  4. Iteratively add particles with random rotations and apply
///     correct boosts
///
/// Complexity: O(N^2), but typically (e.g. N < 10) faster than RAMBO when
/// final states are massive.
///
/// However, unlike RAMBO, event weights from GENBOD can vary largly
/// although final states are massless. RAMBO is recommended for
/// generating unweighted massless final states.
///
/// @tparam N Number of final state particles (N >= 2)
template<int M, int N>
    requires(N >= 2)
class GENBOD : public VersatileEventGenerator<M, N, 3 * N - 4> {
public:
    /// @brief Initial-state 4-momentum
    using typename VersatileEventGenerator<M, N, 3 * N - 4>::InitialStateMomenta;
    /// @brief Final-state 4-momentum container type
    using typename VersatileEventGenerator<M, N, 3 * N - 4>::FinalStateMomenta;
    /// @brief Generated event type
    using typename VersatileEventGenerator<M, N, 3 * N - 4>::Event;
    /// @brief Random state container type
    using typename VersatileEventGenerator<M, N, 3 * N - 4>::RandomState;

public:
    // Inherit constructor
    using VersatileEventGenerator<M, N, 3 * N - 4>::VersatileEventGenerator;

    /// @brief Generate event in center-of-mass frame using precomputed random numbers
    /// @param pI Initial-state 4-momenta
    /// @param u Flat random numbers in 0--1
    /// @return Generated event
    virtual auto operator()(InitialStateMomenta pI, const RandomState& u) -> Event override;
    // Inherit operator() overloads
    using VersatileEventGenerator<M, N, 3 * N - 4>::operator();
};

} // namespace Mustard::inline Physics::inline Generator

#include "Mustard/Physics/Generator/GENBOD.inl"
