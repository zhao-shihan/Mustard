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

#include "muc/math"
#include "muc/numeric"

#include "fmt/core.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <utility>

namespace Mustard::CLHEPX {

/// @brief RAMBO (Random Momenta Beautifully Organized) phase space generator
///
/// Implements the RAMBO algorithm for generating N-particle phase space events
/// according to Lorentz-invariant phase space. Based on:
///   Kleiss, Stirling, Ellis, "A New Monte Carlo Treatment of Multiparticle Phase Space at High Energies"
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

} // namespace Mustard::CLHEPX

#include "Mustard/CLHEPX/RAMBO.inl"
