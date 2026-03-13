// -*- C++ -*-
//
// Copyright (C) 2020-2025  Mustard developers
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

#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Math/Vector.h++"
#include "Mustard/Parallel/ReseedRandomEngine.h++"
#include "Mustard/Physics/Generator/GENBOD.h++"
#include "Mustard/Physics/Generator/MatrixElementBasedGenerator.h++"
#include "Mustard/Physics/QFT/MatrixElement.h++"
#include "Mustard/Physics/QFT/PolarizedMatrixElement.h++"
#include "Mustard/Utility/VectorCast.h++"

#include "CLHEP/Random/Random.h"
#include "CLHEP/Random/RandomEngine.h"

#include "Eigen/Dense"

#include "mplr/mplr.hpp"

#include "muc/algorithm"
#include "muc/array"
#include "muc/chrono"
#include "muc/math"
#include "muc/numeric"
#include "muc/utility"

#include "gsl/gsl"

#include "fmt/ranges.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <functional>
#include <limits>
#include <optional>
#include <stdexcept>
#include <typeinfo>
#include <utility>
#include <vector>

namespace Mustard::inline Physics::inline Generator {

/// @class MCMCGenerator
/// @brief Base class for MCMC generators, possibly with user-defined acceptance.
///
/// Generates events distributed according to |M|² × acceptance, and
/// weight = 1 / acceptance.
///
/// The Markov chain requires reinitialize after each change to
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

protected:
    /// @brief Random state container type
    using RandomState = typename GENBOD<M, N>::RandomState;
    /// @brief Markov chain state container
    struct MarkovChain {
        static constexpr int dim{std::tuple_size_v<RandomState>};
        struct State {
            RandomState u;          ///< Random state
            std::array<int, N> pID; ///< Particle ID permutation (for swapping identical particles, if any)
        } state;                    ///< State of the chain
        double mSqAcceptanceDetJ;   ///< |M|² × acceptance × |J|
        Event event;                ///< The corresponding event
    };

public:
    /// @brief Autocorrelation function (curve) type
    using AutocorrelationFunction = std::vector<std::pair<unsigned, Eigen::Array<double, MarkovChain::dim, 1>>>;

public:
    /// @brief Construct event generator
    /// @param pI initial-state 4-momenta
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    /// @param thinningRatio Thinning factor (non-negative, optional, use default value if not set)
    /// @param acfSampleSize Sample size for estimation autocorrelation function (ACF) (optional, use default value if not set)
    MCMCGenerator(const InitialStateMomenta& pI, const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                  std::optional<double> thinningRatio = {}, std::optional<unsigned> acfSampleSize = {});
    /// @brief Construct event generator
    /// @param pI initial-state 4-momenta
    /// @param polarization Initial-state polarization vector(s)
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    /// @param thinningRatio Thinning factor (non-negative, optional, use default value if not set)
    /// @param acfSampleSize Sample size for estimation autocorrelation function (ACF) (optional, use default value if not set)
    /// @note This overload is only enabled for polarized decay
    MCMCGenerator(const InitialStateMomenta& pI, const typename A::InitialStatePolarization& polarization,
                  const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                  std::optional<double> thinningRatio = {}, std::optional<unsigned> acfSampleSize = {})
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>>;

    /// @brief Get initial-state polarization vector(s)
    /// @note This overload is only enabled for polarized process
    auto Polarization() const -> const typename A::InitialStatePolarization&
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>>;
    /// @brief Get initial-state polarization vector
    /// @param i Particle index (0 ≤ i < M)
    /// @note This overload is only enabled for polarized scattering
    auto Polarization(int i) const -> Vector3D
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1);

    /// @brief Set initial-state polarization vector(s)
    /// @param pol Polarization vector(s) (all |pol| ≤ 1)
    /// @note This overload is only enabled for polarized process
    auto Polarization(const typename A::InitialStatePolarization& pol) -> void
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>>;
    /// @brief Set polarization for single initial-state particle
    /// @param i Particle index (0 ≤ i < M)
    /// @param pol Polarization vector (|pol| ≤ 1)
    /// @note This overload is only enabled for polarized scattering
    auto Polarization(int i, Vector3D pol) -> void
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1);

    /// @brief Set user-defined acceptance function in PDF (PDF = |M|² × acceptance)
    /// @param Acceptance User-defined acceptance
    /// @warning The Markov chain requires reinitialize after set
    auto Acceptance(AcceptanceFunction acceptance) -> void;

    /// @brief Set thinning ratio
    /// Larger the thinning ratio, more samples will be discarded,
    /// and events generated will be more likely to be i.i.d.
    /// Thinning factor ~ thinning ratio * integrated autocorrelation
    /// @param value Thinning factor (>=0)
    auto ThinningRatio(double value) -> void;
    /// @brief Set sample size for estimation autocorrelation function (ACF)
    /// @param n Sample size
    auto ACFSampleSize(unsigned n) -> void;

    /// @brief Return true if Markov chain initialized
    /// @return true if initialized
    auto MCMCInitialized() -> auto { return fMCMCInitialized; }
    /// @brief Initialize Markov chain
    /// @param rng Reference to CLHEP random engine
    auto MCMCInitialize(CLHEP::HepRandomEngine& rng = *CLHEP::HepRandom::getTheEngine()) -> AutocorrelationFunction;

    /// @brief Generate event in c.m. frame
    /// @param rng Reference to CLHEP random engine
    /// @return Generated event
    /// @warning Initial-state momenta passed to this function are ignored.
    /// Use `Momenta` to set initial-state momenta
    virtual auto operator()(CLHEP::HepRandomEngine& rng, InitialStateMomenta) -> Event override;
    // Avoid hiding other operator() overloads from base class
    using Base::operator();

protected:
    /// @brief Set initial-state 4-momenta
    /// @param pI initial-state 4-momenta
    /// @warning The Markov chain requires reinitialize if value changes
    auto Momenta(const InitialStateMomenta& pI) -> void;
    /// @brief Set final-state masses
    /// @param mass Array of particle masses
    /// @warning The Markov chain requires reinitialize if value changes
    auto Mass(const std::array<double, N>& mass) -> void;

    /// @brief Set low-energy cutoff for single final-state particle to avoid infrared divergence (if applicable)
    /// @param i Particle index (0 ≤ i < N)
    /// @param cutoff Soft cutoff value (on kinetic energy in the c.m. frame)
    /// @warning The Markov chain requires reinitialize after set
    auto SoftCutoff(int i, double cutoff) -> void;
    /// @brief Set collinear cutoff for two particles to avoid infrared divergence (if applicable)
    /// @param pID Pair of particle indices (0 ≤ index < N)
    /// @param cutoff Collinear cutoff value (on angle between the two particles in the c.m. frame)
    auto CollinearCutoff(std::pair<int, int> pID, double cutoff) -> void;

    // Avoid hiding other Acceptance overloads from base class
    using Base::Acceptance;

    /// @brief Notify MCMC that reinitialize is required
    auto MCMCInitializationRequired() -> void;

    /// @brief Transform hypercube to phase space
    /// @param u A random state
    /// @return An event from phase space and detJ
    auto DirectPhaseSpace(const RandomState& u) -> std::pair<Event, double>;
    /// @brief Transform state space to phase space
    /// @param state A Markov chain state
    /// @return An event from phase space and detJ
    auto PhaseSpace(const MarkovChain::State& state) -> std::pair<Event, double>;

    /// @brief Proposal distribution for particle mapping. Propose swapping identical particle by chance
    /// @param rng Reference to CLHEP random engine
    /// @param state0 Initial state
    /// @param state Proposed state (modified in-place)
    /// @note Call this in `NextEvent()`.
    auto ProposePID(CLHEP::HepRandomEngine& rng, const std::array<int, N>& pID0, std::array<int, N>& pID) -> void;

private:
    /// @brief Markov chain burn in stage
    /// @param rng Reference to CLHEP random engine
    virtual auto BurnIn(CLHEP::HepRandomEngine& rng) -> void = 0;
    /// @brief Advance Markov chain by one event
    /// @param rng Reference to CLHEP random engine
    /// @return true if proposal accepted, false if not
    virtual auto NextEvent(CLHEP::HepRandomEngine& rng) -> bool = 0;

protected:
    double fThinningRatio;   ///< User-defined thinning ratio
    unsigned fACFSampleSize; ///< Sample size for estimating ACF
                             //
    bool fMCMCInitialized;   ///< Initialization completed flag
    unsigned fThinningSize;  ///< Samples discarded between two generated
    MarkovChain fMC;         ///< Current Markov chain state

    static constexpr auto fgDefaultInvalidACFSampleSize{static_cast<decltype(fACFSampleSize)>(-1)};
};

} // namespace Mustard::inline Physics::inline Generator

#include "Mustard/Physics/Generator/MCMCGenerator.inl"
