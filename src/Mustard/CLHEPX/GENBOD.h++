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

#include "Mustard/CLHEPX/EventGenerator.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "CLHEP/Units/SystemOfUnits.h"
#include "CLHEP/Vector/LorentzVector.h"

#include "muc/algorithm"
#include "muc/math"
#include "muc/numeric"

#include "gsl/gsl"

#include "fmt/core.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <utility>

namespace Mustard::CLHEPX {

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
class GENBOD : public EventGenerator<N, 3 * N - 4> {
public:
    /// @brief Particle four-momentum container type
    using typename EventGenerator<N, 3 * N - 4>::State;
    /// @brief Random state container type
    using typename EventGenerator<N, 3 * N - 4>::RandomState;
    /// @brief Generated event type
    using typename EventGenerator<N, 3 * N - 4>::Event;

public:
    /// @brief Construct RAMBO generator
    /// @param eCM Center-of-mass energy (must exceed sum of masses)
    /// @param mass Array of particle masses (index order preserved)
    constexpr GENBOD(double eCM, const std::array<double, N>& mass);

    /// @brief Generate event using precomputed random numbers
    /// @param u Flat random numbers in 0--1 (3 * N - 4 values required)
    /// @return Generated event
    virtual auto operator()(const RandomState& u) const -> Event override;

private:
    double fEkCM; ///< Kinetic energy in CM frame (eCM - ∑mass)
};

} // namespace Mustard::CLHEPX

#include "Mustard/CLHEPX/GENBOD.inl"
