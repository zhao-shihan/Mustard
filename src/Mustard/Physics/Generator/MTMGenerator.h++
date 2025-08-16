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

#include "Mustard/Execution/Executor.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Parallel/ReseedRandomEngine.h++"
#include "Mustard/Physics/Amplitude/SquaredAmplitude.h++"
#include "Mustard/Physics/Generator/GENBOD.h++"
#include "Mustard/Utility/VectorArithmeticOperator.h++"

#include "CLHEP/Random/RandGaussQ.h"
#include "CLHEP/Random/Random.h"
#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Vector/ThreeVector.h"

#include "muc/array"
#include "muc/math"
#include "muc/numeric"

#include "fmt/core.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <functional>
#include <limits>
#include <utility>

namespace Mustard::inline Physics::inline Generator {

/// @class MTMGenerator
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
/// @tparam A Squared amplitude of the process to be generated
template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
class MTMGenerator : public EventGenerator<M, N> {
public:
    /// @brief Initial-state 4-momentum
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
    MTMGenerator(double cmsE, const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                 double delta, int discard);

    /// @brief Set MCMC step size
    /// @param delta Step scale along one direction in random state space (0 < delta < 0.5)
    auto MCMCDelta(double delta) -> void;
    /// @brief Set discard count between samples
    /// @param discard Samples discarded between two events generated in the Markov chain
    auto MCMCDiscard(int n) -> void;
    /// @brief Set user-defined bias function in PDF (PDF = |M|² × bias)
    /// @param B User-defined bias
    auto Bias(BiasFunction B) -> void;

    /// @brief Initialize Markov chain
    /// @param rng Reference to CLHEP random engine
    auto BurnIn(CLHEP::HepRandomEngine& rng = *CLHEP::HepRandom::getTheEngine()) -> void;

    /// @brief Generate event in center-of-mass frame
    /// @param pI Initial-state 4-momenta (only for its boost)
    /// @param rng Reference to CLHEP random engine
    /// @return Generated event
    virtual auto operator()(InitialStateMomenta pI, CLHEP::HepRandomEngine& rng) -> Event override;
    // Inherit operator() overloads
    using EventGenerator<M, N>::operator();

public:
    /// @brief Weight normalization result
    struct WeightNormalizationFactor {
        double factor; ///< Estimated normalization constant
        double error;  ///< Estimation error
        double nEff;   ///< Statistically-effective sample count
    };

public:
    /// @brief Estimate bias weight normalization factor
    /// Multiply event weights with the factor to normalize weights to the number of (generated) events
    /// @note Use CheckWeightNormalizationFactor to check the result
    auto EstimateWeightNormalizationFactor(unsigned long long n) -> WeightNormalizationFactor;
    /// @brief Print and validate normalization factor quality
    /// @return true if normalization factor quality is OK
    static auto CheckWeightNormalizationFactor(WeightNormalizationFactor wnf) -> bool;

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

    /// @brief Notify MCMC that (re)burn-in is required
    auto BurnInRequired() -> void { fBurntIn = false; }

private:
    /// @brief Check if initial state momentum passed to generator matches
    ///        currently set CMS energy
    /// @param pI Initial-state 4-momenta passed to generator
    auto CheckCMSEnergyUnchanged(const InitialStateMomenta& pI) const -> void;
    /// @brief Advance Markov chain by one event
    /// @param delta Step scale along one direction in random state space (0 < delta < 0.5)
    /// @param rng Reference to CLHEP random engine
    auto NextEvent(double delta, CLHEP::HepRandomEngine& rng) -> void;
    /// @brief Get bias with range check
    /// @param momenta Final states' 4-momenta
    /// @exception `std::runtime_error` if invalid bias value produced
    /// @return B(p1, ..., pN)
    auto ValidBias(const FinalStateMomenta& momenta) const -> double;
    /// @brief Get reweighted PDF value with range check
    /// @param event Final states from phase space
    /// @param bias Bias value at the same phase space point (from BiasWithCheck)
    /// @exception `std::runtime_error` if invalid PDF value produced
    /// @return |M|²(p1, ..., pN) × bias(p1, ..., pN)
    auto ValidBiasedPDF(const Event& event, double bias) const -> double;

private:
    struct MarkovChain {
        double acceptance;               ///< Acceptance of a sample
        GENBOD<M, N>::RandomState state; ///< State of the chain
    };

protected:
    [[no_unique_address]] A fSquaredAmplitude; ///< Squared amplitude

private:
    double fCMSEnergy;        ///< Currently set CM energy
    GENBOD<M, N> fGENBOD;     ///< Phase space generator
    BiasFunction fBias;       ///< User bias function
                              //
    double fMCMCDelta;        ///< MCMC max step size along one dimension
    int fMCMCDiscard;         ///< Events discarded between 2 samples
                              //
    bool fBurntIn;            ///< Burn-in completed flag
    MarkovChain fMarkovChain; ///< Current Markov chain state
    Event fEvent;             ///< Current event in chain

    static constexpr int fgMCMCDim{std::tuple_size_v<typename GENBOD<M, N>::RandomState>};
};

} // namespace Mustard::inline Physics::inline Generator

#include "Mustard/Physics/Generator/MTMGenerator.inl"
