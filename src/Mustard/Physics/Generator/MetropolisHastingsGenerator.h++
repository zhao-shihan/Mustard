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
#include "Mustard/Parallel/ReseedRandomEngine.h++"
#include "Mustard/Physics/Generator/GENBOD.h++"
#include "Mustard/Utility/PrettyLog.h++"
#include "Mustard/Utility/VectorArithmeticOperator.h++"

#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/Random.h"
#include "CLHEP/Random/RandomEngine.h"

#include "muc/array"
#include "muc/math"

#include "fmt/core.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <limits>
#include <utility>

namespace Mustard::inline Physics::inline Generator {

/// @class MetropolisHastingsGenerator
/// @brief Metropolis-Hastings MCMC sampler for event generation,
/// possibly with user-defined bias.
///
/// Generates events distributed according to |M|² × bias.
///
/// @tparam N Number of final-state particles
template<int N>
class MetropolisHastingsGenerator : public EventGenerator<N> {
public:
    /// @brief Particle four-momentum container type
    using typename EventGenerator<N>::Momenta;
    /// @brief Generated event type
    using typename EventGenerator<N>::Event;

public:
    /// @brief Construct event generator
    /// @param cmsE Center-of-mass energy
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    /// @param delta Maximun step along one direction in random state space (0--0.5, should be small)
    /// @param discard Samples discarded between two events generated in the Markov chain
    MetropolisHastingsGenerator(double cmsE, const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                double delta, int discard);

    /// @brief Set center-of-mass energy
    /// @param cmsE Center-of-mass energy
    auto CMSEnergy(double cmsE) -> void;

    /// @brief Set MCMC step size
    /// @param delta Maximun step along one direction in random state space (0--0.5, should be small)
    auto MHDelta(double delta) -> void { fMHDelta = delta; }
    /// @brief Set discard count between samples
    /// @param discard Samples discarded between two events generated in the Markov chain
    auto MHDiscard(int n) -> void { fMHDiscard = n; }
    /// @brief Set user-defined bias function in PDF (PDF = |M|² × bias)
    /// @param B User-defined bias
    auto Bias(std::function<auto(const Momenta&)->double> B) -> void;

    /// @brief Initialize Markov chain
    /// @param rng Reference to CLHEP random engine
    auto BurnIn(CLHEP::HepRandomEngine& rng = *CLHEP::HepRandom::getTheEngine()) -> void;
    /// @brief Generate event in center-of-mass frame
    /// @param cmsE Center-of-mass energy (maybe unused, depend on specific generator)
    /// @param rng Reference to CLHEP random engine
    /// @return Generated event
    virtual auto operator()(double, CLHEP::HepRandomEngine& rng) -> Event override;

    /// @brief Process-specific squared amplitude |M|²
    virtual auto SquaredAmplitude(const Momenta& momenta) const -> double = 0;

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

protected:
    /// @brief Get currently set center-of-mass frame energy
    auto CMSEnergy() const -> auto { return fCMSEnergy; }

    /// @brief Set particle PDG IDs
    /// @param pdgID Array of particle PDG IDs
    auto PDGID(const std::array<int, N>& pdgID) -> void { fGENBOD.PDGID(pdgID); }
    /// @brief Set particle masses
    /// @param mass Array of particle masses
    auto Mass(const std::array<double, N>& mass) -> void;

    /// @brief Notify MCMC that (re)burn-in is required
    auto BurnInRequired() -> void { fBurntIn = false; }

private:
    /// @brief Advance Markov chain by one event
    /// @param delta Maximun step along one direction in random state space
    /// @param rng Reference to CLHEP random engine
    auto NextEvent(double delta, CLHEP::HepRandomEngine& rng) -> void;
    /// @brief Get bias with non-negativity check
    /// @param momenta Final states' 4-momenta
    /// @exception `std::runtime_error` if negative bias value produced
    /// @return B(p1, ..., pN)
    auto BiasWithCheck(const Momenta& momenta) const -> double;

public:
    /// @brief Print and validate normalization factor quality
    /// @return true if normalization factor quality is OK
    static auto CheckWeightNormalizationFactor(WeightNormalizationFactor wnf) -> bool;

private:
    double fCMSEnergy;                                 ///< Currently set CM energy
    GENBOD<N> fGENBOD;                                 ///< Phase space generator
                                                       //
    double fMHDelta;                                   ///< MCMC max step size along one dimension
    int fMHDiscard;                                    ///< Events discarded between 2 samples
    std::function<auto(const Momenta&)->double> fBias; ///< User bias function
                                                       //
    bool fBurntIn;                                     ///< Burn-in completed flag
    GENBOD<N>::RandomState fRandomState;               ///< Current random state
    Event fEvent;                                      ///< Current event in chain
    double fBiasedPDF;                                 ///< Current biased PDF value
};

} // namespace Mustard::inline Physics::inline Generator

#include "Mustard/Physics/Generator/MetropolisHastingsGenerator.inl"
