// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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
#include "Mustard/Physics/Generator/GENBOD.h++"
#include "Mustard/Physics/Generator/MatrixElementBasedGenerator.h++"
#include "Mustard/Physics/QFT/MatrixElement.h++"
#include "Mustard/Physics/QFT/PolarizedMatrixElement.h++"

#include "CLHEP/Random/Random.h"
#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Vector/ThreeVector.h"

#include "mplr/mplr.hpp"

#include "muc/algorithm"
#include "muc/array"
#include "muc/chrono"
#include "muc/math"
#include "muc/numeric"
#include "muc/utility"

#include "gsl/gsl"

#include "fmt/core.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <functional>
#include <limits>
#include <optional>
#include <typeinfo>
#include <utility>

namespace Mustard::inline Physics::inline Generator {

/// @class MCMCGenerator
/// @brief Base class for MCMC generators, possibly with user-defined acceptance.
///
/// Generates events distributed according to |M|² × acceptance, and
/// weight = 1 / acceptance.
///
/// The Markov chain requires burn-in after each change to
/// initial-state momenta. So this generator is unsuitable
/// for case where frequent variation of initial-state momenta is required.
///
/// @tparam M Number of initial-state particles
/// @tparam N Number of final-state particles
/// @tparam A Matrix element of the process to be generated
template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
class MCMCGenerator : public MatrixElementBasedGenerator<M, N, A> {
private:
    /// @brief The base class
    using Base = MatrixElementBasedGenerator<M, N, A>;

public:
    /// @brief Initial-state 4-momentum (or container type when M>1)
    using typename Base::InitialStateMomenta;
    /// @brief Final-state 4-momentum container type
    using typename Base::FinalStateMomenta;
    /// @brief Generated event type
    using typename Base::Event;
    /// @brief User-defined acceptance function type
    using typename Base::AcceptanceFunction;

public:
    /// @brief Construct event generator
    /// @param pI initial-state 4-momenta
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    /// @param delta Step scale along one direction in random state space (0 < delta < 0.5)
    /// (optional here, but should be set no later than before generation)
    /// @param discard Samples discarded between two events generated from the Markov chain
    /// (optional here, but should be set no later than before generation)
    MCMCGenerator(const InitialStateMomenta& pI, const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                  std::optional<double> delta = {}, std::optional<unsigned> discard = {});
    /// @brief Construct event generator
    /// @param pI initial-state 4-momenta
    /// @param polarization Initial-state polarization vector
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    /// @param delta Step scale along one direction in random state space (0 < delta < 0.5)
    /// (optional here, but should be set no later than before generation)
    /// @param discard Samples discarded between two events generated from the Markov chain
    /// (optional here, but should be set no later than before generation)
    /// @note This overload is only enabled for polarized decay
    MCMCGenerator(const InitialStateMomenta& pI, CLHEP::Hep3Vector polarization,
                  const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                  std::optional<double> delta = {}, std::optional<unsigned> discard = {})
        requires std::derived_from<A, QFT::PolarizedMatrixElement<1, N>>;
    /// @brief Construct event generator
    /// @param pI initial-state 4-momenta
    /// @param polarization Initial-state polarization vectors
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    /// @param delta Step scale along one direction in random state space (0 < delta < 0.5)
    /// (optional here, but should be set no later than before generation)
    /// @param discard Samples discarded between two events generated from the Markov chain
    /// (optional here, but should be set no later than before generation)
    /// @note This overload is only enabled for polarized scattering
    MCMCGenerator(const InitialStateMomenta& pI, const std::array<CLHEP::Hep3Vector, M>& polarization,
                  const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                  std::optional<double> delta = {}, std::optional<unsigned> discard = {})
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1);

    /// @brief Get polarization vector
    /// @note This overload is only enabled for polarized decay
    auto InitialStatePolarization() const -> CLHEP::Hep3Vector
        requires std::derived_from<A, QFT::PolarizedMatrixElement<1, N>>;
    /// @brief Get polarization vector
    /// @param i Particle index (0 ≤ i < M)
    /// @note This overload is only enabled for polarized scattering
    auto InitialStatePolarization(int i) const -> CLHEP::Hep3Vector
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1);
    /// @brief Get all polarization vectors
    /// @note This overload is only enabled for polarized scattering
    auto InitialStatePolarization() const -> const std::array<CLHEP::Hep3Vector, M>&
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1);

    /// @brief Set polarization vector
    /// @param pol Polarization vector (|pol| ≤ 1)
    /// @note This overload is only enabled for polarized decay
    /// @warning The Markov chain requires burn-in if value changes
    auto InitialStatePolarization(CLHEP::Hep3Vector pol) -> void
        requires std::derived_from<A, QFT::PolarizedMatrixElement<1, N>>;
    /// @brief Set polarization for single initial particle
    /// @param i Particle index (0 ≤ i < M)
    /// @param pol Polarization vector (|pol| ≤ 1)
    /// @note This overload is only enabled for polarized scattering
    /// @warning The Markov chain requires burn-in if value changes
    auto InitialStatePolarization(int i, CLHEP::Hep3Vector pol) -> void
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1);
    /// @brief Set all polarization vectors
    /// @param pol Array of polarization vectors for each initial particle (all |pol| ≤ 1)
    /// @note This overload is only enabled for polarized scattering
    /// @warning The Markov chain requires burn-in if value changes
    auto InitialStatePolarization(const std::array<CLHEP::Hep3Vector, M>& pol) -> void
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1);

    /// @brief Set user-defined acceptance function in PDF (PDF = |M|² × acceptance)
    /// @param Acceptance User-defined acceptance
    /// @warning The Markov chain requires burn-in after set
    auto Acceptance(AcceptanceFunction Acceptance) -> void;

    /// @brief Set MCMC step size
    /// @param delta Step scale along one direction in random state space (0 < delta < 0.5)
    auto MCMCDelta(double delta) -> void;
    /// @brief Set discard count between samples
    /// @param discard Samples discarded between two events generated from the Markov chain
    auto MCMCDiscard(unsigned n) -> void;

    /// @brief Initialize Markov chain
    /// @param rng Reference to CLHEP random engine
    auto BurnIn(CLHEP::HepRandomEngine& rng = *CLHEP::HepRandom::getTheEngine()) -> void;

    /// @brief Generate event in c.m. frame
    /// @param rng Reference to CLHEP random engine
    /// @return Generated event
    /// @warning Initial-state momenta passed to this function are ignored.
    /// Use `ISMomenta` to set initial-state momenta
    /// @throws `std::logic_error` if neither fMCMCDelta nor MCMCDiscard has been set
    virtual auto operator()(CLHEP::HepRandomEngine& rng, InitialStateMomenta) -> Event override;
    // Inherit operator() overloads
    using Base::operator();

protected:
    /// @brief Set initial-state 4-momenta
    /// @param pI initial-state 4-momenta
    /// @warning The Markov chain requires burn-in if value changes
    auto ISMomenta(const InitialStateMomenta& pI) -> void;

    /// @brief Set final-state masses
    /// @param mass Array of particle masses
    /// @warning The Markov chain requires burn-in if value changes
    auto Mass(const std::array<double, N>& mass) -> void;

    /// @brief Set IR cuts for single final state particle
    /// @param i Particle index (0 ≤ i < N)
    /// @param cut IR cut value
    /// @warning The Markov chain requires burn-in after set
    auto IRCut(int i, double cut) -> void;
    /// @brief Add an identical particle index set.
    /// @note Add the set is not necessary but recommended for reducing
    /// discrpancies between spectra of identical particles from MCMC.
    auto AddIdenticalSet(std::vector<int> set) -> void;

    /// @brief Notify MCMC that (re)burn-in is required
    auto BurnInRequired() -> void { fBurntIn = false; }

protected:
    /// @brief Markov chain state container
    struct MarkovChain {
        static constexpr int dim{std::tuple_size_v<typename GENBOD<M, N>::RandomState>};
        struct State {
            GENBOD<M, N>::RandomState u; ///< Random state
            std::array<int, N> pID;      ///< Particle ID mapping (for swapping identical particles)
        } state;                         ///< State of the chain
        double mSqAcceptanceDetJ;        ///< |M|² × acceptance × |J|
    };

    /// @brief Transform hypercube to phase space
    /// @param u A random state
    /// @return An event from phase space
    auto DirectPhaseSpace(const typename GENBOD<M, N>::RandomState& u) -> auto { return this->fGENBOD(u, Base::ISMomenta()); }
    /// @brief Transform state space to phase space
    /// @param state A Markov chain state
    /// @return An event from phase space
    auto PhaseSpace(const MarkovChain::State& state) -> Event;

    /// @brief Proposal distribution for particle mapping. Propose swapping identical particle by chance
    /// @param rng Reference to CLHEP random engine
    /// @param state0 Initial state
    /// @param state Proposed state (modified in-place)
    /// @note Call this in `NextEvent()`.
    auto ProposePID(CLHEP::HepRandomEngine& rng, const std::array<int, N>& pID0, std::array<int, N>& pID) -> void;

private:
    /// @brief Advance Markov chain by one event
    /// @param rng Reference to CLHEP random engine
    /// @param delta Step scale along one direction in random state space (0 < delta < 0.5)
    virtual auto NextEvent(CLHEP::HepRandomEngine& rng, double delta) -> Event = 0;

protected:
    std::vector<std::vector<int>> fIdenticalSet; ///< Identical particle sets
                                                 //
    double fMCMCDelta;                           ///< Step scale along one direction in random state space
    unsigned fMCMCDiscard;                       ///< Events discarded between 2 samples
                                                 //
    bool fBurntIn;                               ///< Burn-in completed flag
    MarkovChain fMarkovChain;                    ///< Current Markov chain state

    static constexpr double fgDefaultInvalidMCMCDelta{std::numeric_limits<double>::quiet_NaN()};
    static constexpr unsigned fgDefaultInvalidMCMCDiscard{std::numeric_limits<unsigned>::max()};
};

} // namespace Mustard::inline Physics::inline Generator

#include "Mustard/Physics/Generator/MCMCGenerator.inl"
