// -*- C++ -*-
//
// Copyright (C) 2020-2026  Shihan Zhao and contributors
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
#include "Mustard/Env/MonteCarloEnv.h++"
#include "Mustard/Execution/Executor.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Math/MarkovChainStatistic.h++"
#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Statistic.h++"
#include "Mustard/Math/Vector.h++"
#include "Mustard/Parallel/ReseedRandomEngine.h++"
#include "Mustard/Physics/Generator/GENBOD.h++"
#include "Mustard/Physics/Generator/MatrixElementBasedGenerator.h++"
#include "Mustard/Physics/QFT/MatrixElement.h++"
#include "Mustard/Physics/QFT/PolarizedMatrixElement.h++"
#include "Mustard/Utility/VectorCast.h++"

#include "Eigen/Core"

#include "mplr/mplr.hpp"

#include "muc/algorithm"
#include "muc/array"
#include "muc/chrono"
#include "muc/math"
#include "muc/numeric"
#include "muc/random"
#include "muc/utility"

#include "gsl/gsl"

#include "fmt/format.h"

#include <algorithm>
#include <array>
#include <concepts>
#include <functional>
#include <limits>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <typeinfo>
#include <utility>

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
    /// @brief Construct event generator
    /// @param pI initial-state 4-momenta
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    /// @param acfSampleSize Sample size for estimating autocorrelation function (ACF) (optional, use default value if not set)
    /// @param acfReplicate Number of ACF replicate segments (at least 3; global, auto-raised to the number of processes; optional, use default value if not set)
    MCMCGenerator(const InitialStateMomenta& pI, const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                  std::optional<int> acfSampleSize = {}, std::optional<int> acfReplicate = {});
    /// @brief Construct event generator
    /// @param pI initial-state 4-momenta
    /// @param polarization Initial-state polarization vector(s)
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    /// @param acfSampleSize Sample size for estimating autocorrelation function (ACF) (optional, use default value if not set)
    /// @param acfReplicate Number of ACF replicate segments (at least 3; global, auto-raised to the number of processes; optional, use default value if not set)
    /// @note This overload is only enabled for polarized decay.
    MCMCGenerator(const InitialStateMomenta& pI, const typename A::InitialStatePolarization& polarization,
                  const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                  std::optional<int> acfSampleSize = {}, std::optional<int> acfReplicate = {})
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>>;

    /// @brief Get initial-state polarization vector(s)
    /// @note This overload is only enabled for polarized process.
    auto Polarization() const -> const typename A::InitialStatePolarization&
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>>;
    /// @brief Get initial-state polarization vector
    /// @param i Particle index (0 ≤ i < M)
    /// @note This overload is only enabled for polarized scattering.
    auto Polarization(int i) const -> Vector3D
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1);

    /// @brief Set initial-state polarization vector(s)
    /// @param pol Polarization vector(s) (all |pol| ≤ 1)
    /// @note This overload is only enabled for polarized process.
    auto Polarization(const typename A::InitialStatePolarization& pol) -> void
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>>;
    /// @brief Set polarization for single initial-state particle
    /// @param i Particle index (0 ≤ i < M)
    /// @param pol Polarization vector (|pol| ≤ 1)
    /// @note This overload is only enabled for polarized scattering.
    auto Polarization(int i, Vector3D pol) -> void
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1);

    /// @brief Set user-defined acceptance function in PDF (PDF = |M|² × acceptance)
    /// @param Acceptance User-defined acceptance
    /// @warning The Markov chain requires reinitialize after set.
    auto Acceptance(AcceptanceFunction acceptance) -> void;

    /// @brief Set sample size for estimation autocorrelation function (ACF)
    /// @param n Sample size
    auto ACFSampleSize(int n) -> void;
    /// @brief Set number of ACF replicate segments
    /// The number is global; if it is less than the number of processes, it is
    /// raised to the number of processes during initialization.
    /// @param n Number of replicate segments (at least 3)
    auto ACFReplicate(int n) -> void;

    /// @brief Return true if Markov chain initialized
    /// @return true if initialized
    auto MCMCInitialized() -> auto { return fMCMCInitialized; }
    /// @brief Initialize Markov chain
    /// @param executor Executor used to run the ACF replicates in parallel
    /// @param rng Reference to a random number engine
    /// @return Statistics of the Markov chain sample
    /// @note This function must be called before the first event is generated.
    /// @note Collective operation: all MPI processes must call it.
    /// @note Reseeds @p rng collectively, advances each process's Markov chain by
    ///       `(locally executed tasks) * fACFSampleSize`, and temporarily overwrites
    ///       the executor's execution, operation, and task names.
    /// @note Consecutive replicate segments on one process are not strictly
    ///       independent, so the reported standard error is a replication estimate.
    /// @note The executor must run at least as many tasks as there are processes,
    ///       and every process should execute at least one task.
    /// @note Task assignment is dispatcher-dependent; with a dynamic dispatcher,
    ///       the post-initialization chain state and thinning may vary between runs.
    /// @note At least 3 ACF replicates are required.
    /// @throws std::logic_error if the ACF sample size has not been set
    auto MCMCInitialize(Executor<long long>& executor, muc::rng_ref rng = Env::RandomEngine()) -> MarkovChainStatistic<MarkovChain::dim>;

    /// @brief Generate event in c.m. frame
    /// @param rng Reference to a random number engine
    /// @return Generated event
    /// @throws std::logic_error if the Markov chain has not been initialized
    /// @warning Initial-state momenta passed to this function are ignored.
    /// Use `Momenta` to set initial-state momenta.
    virtual auto operator()(muc::rng_ref rng, InitialStateMomenta) -> Event override;
    // Avoid hiding other operator() overloads from base class
    using Base::operator();

protected:
    /// @brief Set initial-state 4-momenta
    /// @param pI initial-state 4-momenta
    /// @warning The Markov chain requires reinitialize if value changes.
    auto Momenta(const InitialStateMomenta& pI) -> void;
    /// @brief Set final-state masses
    /// @param mass Array of particle masses
    /// @warning The Markov chain requires reinitialize if value changes.
    auto Mass(const std::array<double, N>& mass) -> void;

    /// @brief Set low-energy cutoff for single final-state particle to avoid infrared divergence (if applicable)
    /// @param i Particle index (0 ≤ i < N)
    /// @param cutoff Soft cutoff value (on kinetic energy in the c.m. frame)
    /// @warning The Markov chain requires reinitialize after set.
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
    /// @param rng Reference to a random number engine
    /// @param state0 Initial state
    /// @param state Proposed state (modified in-place)
    /// @note Call this in `NextEvent()`.
    auto ProposePID(muc::rng_ref rng, const std::array<int, N>& pID0, std::array<int, N>& pID) -> void;

private:
    /// @brief Markov chain burn in stage
    /// @param rng Reference to a random number engine
    virtual auto BurnIn(muc::rng_ref rng) -> void = 0;
    /// @brief Advance Markov chain by one event
    /// @param rng Reference to a random number engine
    /// @return true if proposal accepted, false if not
    virtual auto NextEvent(muc::rng_ref rng) -> bool = 0;

protected:
    int fACFReplicate;     ///< Number of ACF replicate segments
    int fACFSampleSize;    ///< Sample size for estimating autocorrelation function (ACF)
                           //
    bool fMCMCInitialized; ///< Initialization completed flag
    int fSamplingInterval; ///< Samples discarded between two generated
    MarkovChain fMC;       ///< Current Markov chain state

    static constexpr auto fgDefaultInvalidACFSampleSize{static_cast<decltype(fACFSampleSize)>(-1)};
};

} // namespace Mustard::inline Physics::inline Generator

#include "Mustard/Physics/Generator/MCMCGenerator.inl"
