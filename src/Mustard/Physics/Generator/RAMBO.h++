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

#include "Mustard/Physics/Generator/VersatileEventGenerator.h++"
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

namespace Mustard::inline Physics::inline Generator {

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
class RAMBO : public VersatileEventGenerator<N, 4 * N> {
public:
    /// @brief Particle four-momentum container type
    using typename VersatileEventGenerator<N, 4 * N>::Momenta;
    /// @brief Random state container type
    using typename VersatileEventGenerator<N, 4 * N>::RandomState;
    /// @brief Generated event type
    using typename VersatileEventGenerator<N, 4 * N>::Event;

public:
    // Inherit constructor
    using VersatileEventGenerator<N, 4 * N>::VersatileEventGenerator;

    /// @brief Generate event in center-of-mass frame using precomputed random numbers
    /// @param cmsE Center-of-mass energy
    /// @param u Flat random numbers in 0--1 (4*N values required)
    /// @return Generated event
    virtual auto operator()(double cmsE, const RandomState& u) const -> Event override;
};

} // namespace Mustard::inline Physics::inline Generator

#include "Mustard/Physics/Generator/RAMBO.inl"
