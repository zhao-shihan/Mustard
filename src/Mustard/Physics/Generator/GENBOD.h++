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
/// GENBOD is faster than RAMBO, but event weights can vary largly
/// when final states are massless. This generator is suitable for
/// general uses.
///
/// @tparam N Number of final state particles (N >= 2)
template<int N>
class GENBOD : public VersatileEventGenerator<N, 3 * N - 4> {
public:
    /// @brief Random state container type
    using typename VersatileEventGenerator<N, 3 * N - 4>::RandomState;
    /// @brief Generated event type
    using typename VersatileEventGenerator<N, 3 * N - 4>::Event;

public:
    // Inherit constructor
    using VersatileEventGenerator<N, 3 * N - 4>::VersatileEventGenerator;

    /// @brief Generate event in center-of-mass frame using precomputed random numbers
    /// @param cmsE Center-of-mass energy
    /// @param u Flat random numbers in 0--1 (3*N-4 values required)
    /// @return Generated event
    virtual auto operator()(double cmsE, const RandomState& u) -> Event override;
};

} // namespace Mustard::inline Physics::inline Generator

#include "Mustard/Physics/Generator/GENBOD.inl"
