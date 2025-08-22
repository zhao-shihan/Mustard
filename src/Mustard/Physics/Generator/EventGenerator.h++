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

#include "muc/numeric"

#include <algorithm>
#include <array>
#include <limits>
#include <stdexcept>
#include <utility>

namespace Mustard::inline Physics::inline Generator {

/// @class EventGenerator
/// @brief Base class for M-to-N event generators
///
/// Abstract base class for generating events with M-body initial state
/// and N-body final state.
/// Provides common interface for event generation.
///
/// D = -1: The base class.
/// D >= 3N-4: Can uses fixed-size arrays of pre-generated random numbers.
///
/// @tparam M Number of initial-state particles (M ≥ 1)
/// @tparam N Number of final-state particles (N ≥ 1)
/// @tparam D Dimension of random state (default or D ≥ Dim(phase space))
template<int M, int N, int D = -1>
    requires(M >= 1 and N >= 1 and (D == -1 or D >= 3 * N - 4))
class EventGenerator;

/// @class EventGenerator<M, N>
/// @brief Specialization for random-engine-based generator
///
/// Uses CLHEP random engines directly.
///
/// @tparam M Number of initial-state particles (M ≥ 1)
/// @tparam N Number of final-state particles (N ≥ 1)
template<int M, int N>
class EventGenerator<M, N> {
public:
    /// @brief Initial-state 4-momentum (or container type when M>1)
    using InitialStateMomenta = std::conditional_t<M == 1, CLHEP::HepLorentzVector,
                                                   std::array<CLHEP::HepLorentzVector, M>>;
    /// @brief Final-state 4-momentum container type
    using FinalStateMomenta = std::array<CLHEP::HepLorentzVector, N>;
    /// @brief Generated event type
    struct Event {
        double weight;            ///< Event weight
        std::array<int, N> pdgID; ///< Particle PDG IDs
        FinalStateMomenta p;      ///< Particle 4-momenta
    };

public:
    // Virtual destructor
    constexpr virtual ~EventGenerator() = default;

    /// @brief Generate event according to initial state
    /// @param rng Reference to CLHEP random engine
    /// @param pI Initial-state 4-momenta (maybe unused, depend on specific generator)
    /// @return Generated event
    virtual auto operator()(CLHEP::HepRandomEngine& rng, InitialStateMomenta pI) -> Event = 0;
    /// @brief Generate event according to initial state using global CLHEP engine
    /// @param pI Initial-state 4-momenta (maybe unused, depend on specific generator)
    /// @return Generated event
    auto operator()(const InitialStateMomenta& pI) -> Event;
    /// @brief Generate event in center-of-mass frame.
    /// This overload is intended for generators with fixed CMS energy (e.g. decay)
    /// @param rng Reference to CLHEP random engine
    /// @return Generated event
    auto operator()(CLHEP::HepRandomEngine& rng = *CLHEP::HepRandom::getTheEngine()) -> Event;

protected:
    /// @brief Calculate center-of-mass energy from initial state
    /// @param pI Initial-state 4-momenta
    /// @return Total CM energy (invariant mass)
    static auto CalculateCMSEnergy(const InitialStateMomenta& pI) -> double;
    /// @brief Boost initial state to CM frame
    ///
    /// Transforms initial-state momenta to center-of-mass frame:
    ///   - For 1-body initial state: resets momentum to (m,0,0,0)
    ///   - For multiple-body initial state: boosts to zero-momentum frame
    ///
    /// @param pI Initial-state 4-momenta (modified in-place)
    /// @return Boost vector (β) from CM frame to original frame
    ///
    /// @note Return value should be saved for `BoostToOriginalFrame` call
    /// @warning Always called before event generation in CM frame
    [[nodiscard]] static auto BoostToCMS(InitialStateMomenta& p) -> CLHEP::Hep3Vector;
    /// @brief Boost final state to original frame
    ///
    /// Applies inverse boost to return final state from CM frame
    /// to original frame.
    ///
    /// @param beta Boost vector returned from `BoostToCMS` call
    /// @param p Final-state 4-momenta (modified in-place)
    ///
    /// @note Must use the β returned by `BoostToCMS` for correct transformation
    /// @warning Always called after event generation in CM frame
    static auto BoostToOriginalFrame(CLHEP::Hep3Vector beta, FinalStateMomenta& p) -> void;
};

template<int M, int N, int D>
    requires(M >= 1 and N >= 1 and (D == -1 or D >= 3 * N - 4))
class EventGenerator : public EventGenerator<M, N> {
public:
    /// @brief Initial-state 4-momentum (or container type when M>1)
    using typename EventGenerator<M, N>::InitialStateMomenta;
    /// @brief Generated event type
    using typename EventGenerator<M, N>::Event;
    /// @brief Random state container type
    using RandomState = std::array<double, D>;

public:
    /// @brief Generate event according to initial state using precomputed random numbers
    /// @param u Flat random numbers in 0--1 (D values required)
    /// @param pI Initial-state 4-momenta (maybe unused, depend on specific generator)
    /// @return Generated event
    virtual auto operator()(const RandomState& u, InitialStateMomenta pI) -> Event = 0;
    /// @brief Generate event in center-of-mass frame using precomputed random numbers.
    /// This overload is intended for generators with fixed CMS energy (e.g. decay)
    /// @param u Flat random numbers in 0--1 (D values required)
    /// @return Generated event
    auto operator()(const RandomState& u) -> Event;

    /// @brief Generate event according to initial state
    /// @param rng Reference to CLHEP random engine
    /// @param pI Initial-state 4-momenta (maybe unused, depend on specific generator)
    /// @return Generated event
    virtual auto operator()(CLHEP::HepRandomEngine& rng, InitialStateMomenta pI) -> Event override;
    // Inherit operator() overloads
    using EventGenerator<M, N>::operator();
};

} // namespace Mustard::inline Physics::inline Generator

#include "Mustard/Physics/Generator/EventGenerator.inl"
