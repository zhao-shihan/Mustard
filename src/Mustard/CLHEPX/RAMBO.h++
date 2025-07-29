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

#include "Mustard/Utility/PrettyLog.h++"

#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/Random.h"
#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "CLHEP/Vector/LorentzVector.h"
#include "CLHEP/Vector/ThreeVector.h"

#include "muc/math"
#include "muc/numeric"

#include "fmt/core.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>
#include <stdexcept>
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
    requires(N >= 2)
class RAMBO {
public:
    /// @brief Construct RAMBO generator
    /// @param eCM Center-of-mass energy (must exceed sum of masses)
    /// @param mass Array of particle masses (index order preserved)
    /// @exception std::domain_error if center-of-mass energy is insufficient
    constexpr RAMBO(double eCM, std::array<double, N> mass);

public:
    using State = std::array<CLHEP::HepLorentzVector, N>; ///< Final state particle four-momenta
    struct Event {
        double weight; ///< Event weight (1 for massless final states, non-1 otherwise)
        State state;   ///< Particle four-momenta
    };

public:
    /// @brief Generate event using precomputed random numbers
    /// @param u Flat random numbers in 0--1 (4*N values required)
    auto operator()(const std::array<double, 4 * N>& u) const -> Event;
    /// @brief Generate event using CLHEP random engine
    /// @param rng Reference to CLHEP random engine
    auto operator()(CLHEP::HepRandomEngine& rng) const -> Event;
    /// @brief Generate event using global CLHEP engine
    auto operator()() const -> Event { return (*this)(*CLHEP::HepRandom::getTheEngine()); }

    /// @brief Generate event with lab-frame boost using precomputed randoms
    /// @param u Flat random numbers in 0--1 (4*N values)
    /// @param beta Boost vector (v/c) for lab frame transformation
    auto operator()(const std::array<double, 4 * N>& u, const CLHEP::Hep3Vector& beta) const -> Event;
    /// @brief Generate event with lab-frame boost using CLHEP engine
    /// @param rng Reference to CLHEP random engine
    /// @param beta Boost vector (v/c) for lab frame
    auto operator()(CLHEP::HepRandomEngine& rng, const CLHEP::Hep3Vector& beta) const -> Event;
    /// @brief Generate boosted event using global CLHEP engine
    auto operator()(const CLHEP::Hep3Vector& beta) const -> Event { return (*this)(*CLHEP::HepRandom::getTheEngine(), beta); }

private:
    double fECM;                 ///< Center-of-mass energy
    std::array<double, N> fMass; ///< Particle rest masses
    bool fAllMassAreTiny;        ///< Massless approximation flag
};

} // namespace Mustard::CLHEPX

#include "Mustard/CLHEPX/RAMBO.inl"
