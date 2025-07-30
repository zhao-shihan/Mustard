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

#include "CLHEP/Random/Random.h"
#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Vector/LorentzVector.h"
#include "CLHEP/Vector/ThreeVector.h"

#include "muc/numeric"

#include <array>
#include <limits>
#include <stdexcept>

namespace Mustard::CLHEPX {

namespace internal {

/// @internal
/// @brief Sentinel value for random state dimension
///
/// Special value indicating the generator doesn't require fixed-size random states.
/// Used to enable partial template specialization.
inline constexpr int AnyRandomStateDim{std::numeric_limits<int>::max()};

} // namespace internal

/// @class EventGenerator
/// @brief Base class for N-particle event generators
///
/// Abstract base class for generating events with N final-state particles.
/// Provides common interface for event generation with/without lab-frame boosts.
///
/// @tparam N Number of particles in final state (N ≥ 2)
/// @tparam M Dimension of random state (default=AnyRandomStateDim)
///
/// @requires
/// - N ≥ 2 (minimum two particles)
/// - M ≥ 3N - 4 (sufficient random dimensions for phase space)
template<int N, int M = internal::AnyRandomStateDim>
    requires(N >= 2 and M >= 3 * N - 4)
class EventGenerator;

/// @class EventGenerator<N, internal::AnyRandomStateDim>
/// @brief Specialization for engine-based random number generation
///
/// Uses CLHEP random engines directly.
///
/// @tparam N Number of particles (inherited from primary template)
template<int N>
    requires(N >= 2)
class EventGenerator<N, internal::AnyRandomStateDim> {
public:
    /// @brief Particle four-momentum container type
    using State = std::array<CLHEP::HepLorentzVector, N>;
    /// @brief Generated event type
    struct Event {
        double weight; ///< Event weight
        State state;   ///< Particle four-momenta
    };

public:
    /// @brief Construct event generator
    /// @param eCM Center-of-mass energy (must exceed sum of masses)
    /// @param mass Array of particle masses (index order preserved)
    /// @exception std::domain_error if center-of-mass energy is insufficient
    constexpr EventGenerator(double eCM, const std::array<double, N>& mass);

    // Virtual destructor
    constexpr virtual ~EventGenerator() = default;

    /// @brief Generate event using CLHEP random engine
    /// @param rng Reference to CLHEP random engine
    /// @return Generated event
    virtual auto operator()(CLHEP::HepRandomEngine& rng) const -> Event = 0;
    /// @brief Generate event using global CLHEP engine
    /// @return Generated event
    auto operator()() const -> Event;

    /// @brief Generate event with lab-frame boost using CLHEP engine
    /// @param rng Reference to CLHEP random engine
    /// @param beta Boost vector (v/c) for lab frame
    /// @return Generated event
    auto operator()(CLHEP::HepRandomEngine& rng, CLHEP::Hep3Vector beta) const -> Event;
    /// @brief Generate boosted event using global CLHEP engine
    /// @return Generated event
    auto operator()(CLHEP::Hep3Vector beta) const -> Event;

protected:
    double fECM;                 ///< Center-of-mass energy
    std::array<double, N> fMass; ///< Particle rest masses
};

/// @class EventGenerator<N, M>
/// @ingroup event_generation
/// @brief Specialization for precomputed random state generation
///
/// Can uses fixed-size arrays of pre-generated random numbers.
///
/// @tparam N Number of particles (N ≥ 2)
/// @tparam M Dimension of random state (M ≥ 3N - 4)
///
/// @requires
/// - N ≥ 2 (minimum two particles)
/// - M ≥ 3N - 4 (sufficient random dimensions for phase space)
template<int N, int M>
    requires(N >= 2 and M >= 3 * N - 4)
class EventGenerator : public EventGenerator<N, internal::AnyRandomStateDim> {
public:
    /// @brief Particle four-momentum container type
    using typename EventGenerator<N, internal::AnyRandomStateDim>::State;
    /// @brief Random state container type
    using RandomState = std::array<double, M>;
    /// @brief Generated event type
    using typename EventGenerator<N, internal::AnyRandomStateDim>::Event;

public:
    // Inherit constructor
    using EventGenerator<N, internal::AnyRandomStateDim>::EventGenerator;

    /// @brief Generate event using precomputed random numbers
    /// @param u Flat random numbers in 0--1 (M values required)
    /// @return Generated event
    virtual auto operator()(const RandomState& u) const -> Event = 0;
    /// @brief Generate event using CLHEP random engine
    /// @param rng Reference to CLHEP random engine
    /// @return Generated event
    auto operator()(CLHEP::HepRandomEngine& rng) const -> Event override final;

    /// @brief Generate event with lab-frame boost using precomputed randoms
    /// @param u Flat random numbers in 0--1 (M values)
    /// @param beta Boost vector (v/c) for lab frame transformation
    /// @return Generated event
    auto operator()(const RandomState& u, CLHEP::Hep3Vector beta) const -> Event;
};

} // namespace Mustard::CLHEPX

#include "Mustard/CLHEPX/EventGenerator.inl"
