// -*- C++ -*-
//
// Copyright 2020-2025  The Mustard development team
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

#include "Mustard/Execution/Executor.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Parallel/ReseedRandomEngine.h++"
#include "Mustard/Physics/Generator/EventGenerator.h++"
#include "Mustard/Physics/Generator/GENBOD.h++"
#include "Mustard/Physics/QFT/MatrixElement.h++"
#include "Mustard/Physics/QFT/PolarizedMatrixElement.h++"
#include "Mustard/Utility/VectorArithmeticOperator.h++"

#include "CLHEP/Random/RandGaussQ.h"
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

#include "fmt/ranges.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <functional>
#include <limits>
#include <typeinfo>
#include <utility>

namespace Mustard::inline Physics::inline Generator {

/// @class MultipleTryMetropolisGenerator
/// @brief Multiple-try Metropolis (MTM) MCMC sampler for event generation,
/// possibly with user-defined bias. MTM sampler can help resolve the
/// curse of dimensionality.
///
/// Generates events distributed according to |M|² × bias, and
/// weight = 1 / bias.
///
/// The Markov chain requires burn-in after each change to
/// center-of-mass energy. So this generator is unsuitable
/// for case where frequent variation of CMS energy is required.
///
/// @tparam M Number of initial-state particles
/// @tparam N Number of final-state particles
/// @tparam A Matrix element of the process to be generated
template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
class MultipleTryMetropolisGenerator : public EventGenerator<M, N> {
public:
    /// @brief Initial-state 4-momentum (or container type when M>1)
    using typename EventGenerator<M, N>::InitialStateMomenta;
    /// @brief Final-state 4-momentum container type
    using typename EventGenerator<M, N>::FinalStateMomenta;
    /// @brief Generated event type
    using typename EventGenerator<M, N>::Event;
    /// @brief User-defined bias function type
    using BiasFunction = std::function<auto(const FinalStateMomenta&)->double>;

public:
    /// @brief Construct event generator
    /// @param cmsE Center-of-mass energy
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    /// @param delta Step scale along one direction in random state space (0 < delta < 0.5)
    /// @param discard Samples discarded between two events generated in the Markov chain
    MultipleTryMetropolisGenerator(double cmsE, const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                   double delta, unsigned discard);
    /// @brief Construct event generator
    /// @param cmsE Center-of-mass energy
    /// @param polarization Initial-state polarization vector
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    /// @param delta Step scale along one direction in random state space (0 < delta < 0.5)
    /// @param discard Samples discarded between two events generated in the Markov chain
    /// @note This overload is only enabled for polarized decay
    MultipleTryMetropolisGenerator(double cmsE, CLHEP::Hep3Vector polarization,
                                   const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                   double delta, unsigned discard)
        requires std::derived_from<A, QFT::PolarizedMatrixElement<1, N>>;
    /// @brief Construct event generator
    /// @param cmsE Center-of-mass energy
    /// @param polarization Initial-state polarization vectors
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    /// @param delta Step scale along one direction in random state space (0 < delta < 0.5)
    /// @param discard Samples discarded between two events generated in the Markov chain
    /// @note This overload is only enabled for polarized scattering
    MultipleTryMetropolisGenerator(double cmsE, const std::array<CLHEP::Hep3Vector, M>& polarization,
                                   const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                   double delta, unsigned discard)
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1);

    // Keep the class abstract
    virtual ~MultipleTryMetropolisGenerator() override = 0;

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
    /// @note Triggers Markov chain reset (requires new burn-in)
    /// This overload is only enabled for polarized decay
    auto InitialStatePolarization(CLHEP::Hep3Vector pol) -> void
        requires std::derived_from<A, QFT::PolarizedMatrixElement<1, N>>;
    /// @brief Set polarization for single initial particle
    /// @param i Particle index (0 ≤ i < M)
    /// @param pol Polarization vector (|pol| ≤ 1)
    /// @note Triggers Markov chain reset (requires new burn-in).
    /// This overload is only enabled for polarized scattering
    auto InitialStatePolarization(int i, CLHEP::Hep3Vector pol) -> void
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1);
    /// @brief Set all polarization vectors
    /// @param pol Array of polarization vectors for each initial particle (all |pol| ≤ 1)
    /// @note Triggers Markov chain reset (requires new burn-in)
    /// This overload is only enabled for polarized scattering
    auto InitialStatePolarization(const std::array<CLHEP::Hep3Vector, M>& pol) -> void
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1);

    /// @brief Set user-defined bias function in PDF (PDF = |M|² × bias)
    /// @param B User-defined bias
    auto Bias(BiasFunction B) -> void;

    /// @brief Set MCMC step size
    /// @param delta Step scale along one direction in random state space (0 < delta < 0.5)
    auto MCMCDelta(double delta) -> void;
    /// @brief Set discard count between samples
    /// @param discard Samples discarded between two events generated in the Markov chain
    auto MCMCDiscard(unsigned n) -> void;

    /// @brief Initialize Markov chain
    /// @param rng Reference to CLHEP random engine
    auto BurnIn(CLHEP::HepRandomEngine& rng = *CLHEP::HepRandom::getTheEngine()) -> void;

    /// @brief Generate event in center-of-mass frame
    /// @param rng Reference to CLHEP random engine
    /// @param pI Initial-state 4-momenta (only for its boost)
    /// @return Generated event
    virtual auto operator()(CLHEP::HepRandomEngine& rng, InitialStateMomenta pI) -> Event override;
    // Inherit operator() overloads
    using EventGenerator<M, N>::operator();

public:
    /// @brief Integration internal state
    struct IntegrationState {
        muc::array2ld sum;    ///< Sum of integrand and squared integrand
        unsigned long long n; ///< Sample size
    };

    /// @brief Integration result
    struct IntegrationResult {
        double value;       ///< Estimated normalization constant
        double uncertainty; ///< MC integration uncertainty
    };

public:
    /// @brief Estimate normalization factor.
    /// Multiply event weights with the normalization factor
    /// to normalize weights to the number of generated events.
    /// Essential for calculating total cross-section or width
    /// when bias function is set
    /// @param executor An executor instance
    /// @param precisionGoal Target relative uncertainty (e.g. 0.01 for 1% rel. unc.)
    /// @param integrationState Integration state for continuing normalization
    /// @param rng Reference to CLHEP random engine
    /// @return Estimated normalization factor and integration state
    auto EstimateNormalizationFactor(Executor<unsigned long long>& executor, double precisionGoal,
                                     std::array<IntegrationState, 2> integrationState = {},
                                     CLHEP::HepRandomEngine& rng = *CLHEP::HepRandom::getTheEngine()) -> std::pair<IntegrationResult, std::array<IntegrationState, 2>>;

protected:
    /// @brief Get currently set center-of-mass frame energy
    auto CMSEnergy() const -> auto { return fCMSEnergy; }
    /// @brief Set center-of-mass energy
    /// @param cmsE Center-of-mass energy
    /// @warning The Markov chain requires burn-in after center-of-mass energy changes
    auto CMSEnergy(double cmsE) -> void;

    /// @brief Set particle PDG IDs
    /// @param pdgID Array of particle PDG IDs
    auto PDGID(const std::array<int, N>& pdgID) -> void { fGENBOD.PDGID(pdgID); }
    /// @brief Set particle masses
    /// @param mass Array of particle masses
    auto Mass(const std::array<double, N>& mass) -> void;

    /// @brief Set IR cuts for single final state particle
    /// @param i Particle index (0 ≤ i < N)
    /// @param cut IR cut value
    /// @warning The Markov chain requires burn-in after IR cut changes
    auto IRCut(int i, double cut) -> void;
    /// @brief Add an identical particle index set.
    /// @note Add the set is not necessary but recommended for reducing
    /// discrpancies between spectra of identical particles from MCMC.
    auto AddIdenticalSet(std::vector<int> set) -> void;

    /// @brief Notify MCMC that (re)burn-in is required
    auto BurnInRequired() -> void { fBurntIn = false; }

private:
    struct MarkovChain {
        struct State {
            GENBOD<M, N>::RandomState u; ///< Random state
            std::array<int, N> pID;      ///< Particle ID mapping (for swapping identical particles)
        } state;                         ///< State of the chain
        double biasedMSqDetJ;            ///< |M|² × bias × |J|
    };

private:
    /// @brief Check whether initial state momentum passed to generator matches
    /// currently set CMS energy
    /// @param pI Initial-state 4-momenta passed to generator
    auto CheckCMSEnergyMatch(const InitialStateMomenta& pI) const -> void;
    /// @brief Advance Markov chain by one event
    /// @param rng Reference to CLHEP random engine
    /// @param delta Step scale along one direction in random state space (0 < delta < 0.5)
    auto NextEvent(CLHEP::HepRandomEngine& rng, double delta) -> Event;

    /// @brief Transform hypercube to phase space
    /// @param u A random state
    /// @return An event from phase space
    auto DirectPhaseSpace(const typename GENBOD<M, N>::RandomState& u) -> auto { return fGENBOD(u, {fCMSEnergy, {}}); }
    /// @brief Transform state space to phase space
    /// @param state A Markov chain state
    /// @return An event from phase space
    auto PhaseSpace(const MarkovChain::State& state) -> Event;
    /// @brief Check final-state momenta pass the IR cut
    /// @param pF Final states' 4-momenta
    /// @return true if momenta is IR-safe
    auto IRSafe(const FinalStateMomenta& pF) const -> bool;
    /// @brief Get bias with range check
    /// @param pF Final states' 4-momenta
    /// @exception `std::runtime_error` if invalid bias value produced
    /// @return B(p1, ..., pN)
    auto ValidBias(const FinalStateMomenta& pF) const -> double;
    /// @brief Get reweighted PDF value with range check
    /// @param pF Final states' 4-momenta
    /// @param event Final states from phase space
    /// @param bias Bias value at the same phase space point (from BiasWithCheck)
    /// @exception `std::runtime_error` if invalid PDF value produced
    /// @return |M|²(p1, ..., pN) × bias(p1, ..., pN) × |J|(p1, ..., pN)
    auto ValidBiasedMSqDetJ(const FinalStateMomenta& pF, double bias, double detJ) const -> double;

private:
    double fCMSEnergy;                           ///< Currently set CM energy
    [[no_unique_address]] A fMatrixElement;      ///< Matrix element
    std::vector<std::pair<int, double>> fIRCut;  ///< IR cuts
    std::vector<std::vector<int>> fIdenticalSet; ///< Identical particle sets
    BiasFunction fBias;                          ///< User bias function
    GENBOD<M, N> fGENBOD;                        ///< Phase space generator
                                                 //
    double fMCMCDelta;                           ///< MCMC max step size along one dimension
    unsigned fMCMCDiscard;                       ///< Events discarded between 2 samples
                                                 //
    bool fBurntIn;                               ///< Burn-in completed flag
    MarkovChain fMarkovChain;                    ///< Current Markov chain state

    static constexpr int fgMCMCDim{std::tuple_size_v<typename GENBOD<M, N>::RandomState>};
};

} // namespace Mustard::inline Physics::inline Generator

#include "Mustard/Physics/Generator/MultipleTryMetropolisGenerator.inl"
