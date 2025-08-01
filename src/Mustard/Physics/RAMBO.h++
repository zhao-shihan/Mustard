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

#include "Mustard/Physics/EventGenerator.h++"
#include "Mustard/Utility/PrettyLog.h++"

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

namespace Mustard::inline Physics {

/// @brief N-body phase space generator using RAMBO algorithm
///
/// Implements the RAMBO algorithm for generating N-particle phase space events.
/// Based on:
///   R. Kleiss, W.J. Stirling, S.D. Ellis, "A New Monte Carlo Treatment Of
///   Multiparticle Phase Space At High-Energies", CPC40 (1986) 359.
///
/// RAMBO is slower than GENBOD, but with following good properties:
///  - For massless final states, RAMBO can generate weight=1 events.
///  - For near-massless final states RAMBO can generate weight~1 events.
/// This generator is very suitable for generating unweighted massless
/// final states.
///
/// @tparam N Number of final state particles (N >= 2)
template<int N>
class RAMBO : public EventGenerator<N, 4 * N> {
public:
    /// @brief Particle four-momentum container type
    using typename EventGenerator<N, 4 * N>::State;
    /// @brief Random state container type
    using typename EventGenerator<N, 4 * N>::RandomState;
    /// @brief Generated event type
    using typename EventGenerator<N, 4 * N>::Event;

public:
    /// @brief Construct RAMBO generator
    /// @param eCM Center-of-mass energy (must exceed sum of masses)
    /// @param mass Array of particle masses (index order preserved)
    constexpr RAMBO(double eCM, const std::array<double, N>& mass);

    /// @brief Generate event using precomputed random numbers
    /// @param u Flat random numbers in 0--1 (4*N values required)
    /// @return Generated event
    virtual auto operator()(const RandomState& u) const -> Event override;

private:
    bool fAllMassAreTiny; ///< Massless approximation flag
};

} // namespace Mustard::inline Physics

#include "Mustard/Physics/RAMBO.inl"
