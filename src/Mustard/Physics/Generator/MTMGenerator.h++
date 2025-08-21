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
#include "Mustard/Physics/Generator/EventGenerator.h++"
#include "Mustard/Physics/Generator/GENBOD.h++"
#include "Mustard/Physics/internal/MultipleTryMetropolisCore.h++"
#include "Mustard/Utility/VectorArithmeticOperator.h++"

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
class MTMGenerator : public EventGenerator<M, N>,
                     public internal::MultipleTryMetropolisCore<M, N, A> {
public:
    /// @brief Initial-state 4-momentum
    using typename EventGenerator<M, N>::InitialStateMomenta;
    /// @brief Final-state 4-momentum container type
    using typename EventGenerator<M, N>::FinalStateMomenta;
    /// @brief Generated event type
    using typename EventGenerator<M, N>::Event;
    /// @brief User-defined bias function type
    using typename internal::MultipleTryMetropolisCore<M, N, A>::BiasFunction;
    /// @brief IR cut value container type for IR-unsafe final states
    using typename internal::MultipleTryMetropolisCore<M, N, A>::IRCutArray;

public:
    // Inherit constructor
    using internal::MultipleTryMetropolisCore<M, N, A>::MultipleTryMetropolisCore;

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
        double value; ///< Estimated normalization constant
        double error; ///< Estimation error
        double nEff;  ///< Statistically-effective sample count
    };

public:
    /// @brief Estimate bias weight normalization factor
    /// Multiply event weights with the factor to normalize weights to
    /// the number of generated events
    /// @note Use CheckWeightNormalizationFactor to check the result
    auto EstimateWeightNormalizationFactor(unsigned long long n) -> WeightNormalizationFactor;
    /// @brief Print and validate normalization factor quality
    /// @return true if normalization factor quality is OK
    static auto CheckWeightNormalizationFactor(WeightNormalizationFactor wnf) -> bool;

private:
    /// @brief Check if initial state momentum passed to generator matches
    ///        currently set CMS energy
    /// @param pI Initial-state 4-momenta passed to generator
    auto CheckCMSEnergyUnchanged(const InitialStateMomenta& pI) const -> void;
};

} // namespace Mustard::inline Physics::inline Generator

#include "Mustard/Physics/Generator/MTMGenerator.inl"
