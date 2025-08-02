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

#include "CLHEP/Random/Random.h"
#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Vector/LorentzVector.h"
#include "CLHEP/Vector/ThreeVector.h"

#include <array>
#include <limits>
#include <stdexcept>
#include <utility>

namespace Mustard::inline Physics::inline Generator {

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
    using Momenta = std::array<CLHEP::HepLorentzVector, N>;
    /// @brief Generated event type
    struct Event {
        double weight;            ///< Event weight
        std::array<int, N> pdgID; ///< Particle PDG IDs
        Momenta p;                ///< Particle four-momenta
    };

public:
    // Virtual destructor
    constexpr virtual ~EventGenerator() = default;

    /// @brief Generate event in center-of-mass frame
    /// @param cmsE Center-of-mass energy (maybe unused, depend on specific generator)
    /// @param rng Reference to CLHEP random engine
    /// @return Generated event
    virtual auto operator()(double cmsE, CLHEP::HepRandomEngine& rng) const -> Event = 0;
    /// @brief Generate event in center-of-mass frame using global CLHEP engine
    /// @param cmsE Center-of-mass energy (maybe unused, depend on specific generator)
    /// @return Generated event
    auto operator()(double cmsE) const -> Event;
    /// @brief Generate event in center-of-mass frame,
    /// this overload is intended for generators with fixed CMS energy (CMS energy unused)
    /// @return Generated event
    auto operator()(CLHEP::HepRandomEngine& rng = *CLHEP::HepRandom::getTheEngine()) const -> Event;

    /// @brief Generate event with lab-frame boost
    /// @param cmsE Center-of-mass energy (maybe unused, depend on specific generator)
    /// @param beta Boost vector for lab frame
    /// @param rng Reference to CLHEP random engine (default: global CLHEP engine)
    /// @return Generated event
    auto operator()(double cmsE, CLHEP::Hep3Vector beta, CLHEP::HepRandomEngine& rng = *CLHEP::HepRandom::getTheEngine()) const -> Event;
    /// @brief Generate boosted event using global CLHEP engine,
    /// this overload is intended for generators with fixed CMS energy (CMS energy unused)
    /// @param beta Boost vector for lab frame
    /// @param rng Reference to CLHEP random engine (default: global CLHEP engine)
    /// @return Generated event
    auto operator()(CLHEP::Hep3Vector beta, CLHEP::HepRandomEngine& rng = *CLHEP::HepRandom::getTheEngine()) const -> Event;
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
    /// @brief Random state container type
    using RandomState = std::array<double, M>;
    /// @brief Generated event type
    using typename EventGenerator<N, internal::AnyRandomStateDim>::Event;

public:
    /// @brief Generate event in center-of-mass frame using precomputed random numbers
    /// @param cmsE Center-of-mass energy (maybe unused, depend on specific generator)
    /// @param u Flat random numbers in 0--1 (M values required)
    /// @return Generated event
    virtual auto operator()(double cmsE, const RandomState& u) const -> Event = 0;
    /// @brief Generate event in center-of-mass frame using precomputed random numbers
    /// this overload is intended for generators with fixed CMS energy (CMS energy unused)
    /// @param u Flat random numbers in 0--1 (M values required)
    /// @return Generated event
    auto operator()(const RandomState& u) const -> Event;

    /// @brief Generate event with lab-frame boost using precomputed randoms
    /// @param cmsE Center-of-mass energy (maybe unused, depend on specific generator)
    /// @param u Flat random numbers in 0--1 (M values)
    /// @param beta Boost vector for lab frame
    /// @return Generated event
    auto operator()(double cmsE, CLHEP::Hep3Vector beta, const RandomState& u) const -> Event;
    /// @brief Generate event with lab-frame boost using precomputed randoms
    /// this overload is intended for generators with fixed CMS energy (CMS energy unused)
    /// @param u Flat random numbers in 0--1 (M values)
    /// @param beta Boost vector for lab frame
    /// @return Generated event
    auto operator()(CLHEP::Hep3Vector beta, const RandomState& u) const -> Event;

    /// @brief Generate event in center-of-mass frame
    /// @param cmsE Center-of-mass energy (maybe unused, depend on specific generator)
    /// @param rng Reference to CLHEP random engine
    /// @return Generated event
    virtual auto operator()(double cmsE, CLHEP::HepRandomEngine& rng) const -> Event override;
};

} // namespace Mustard::inline Physics::inline Generator

#include "Mustard/Physics/Generator/EventGenerator.inl"
