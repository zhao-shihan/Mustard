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

#include "Mustard/Math/Random/Distribution/Gaussian.h++"
#include "Mustard/Physics/Generator/MCMCGenerator.h++"
#include "Mustard/Physics/QFT/MatrixElement.h++"
#include "Mustard/Utility/VectorAssign.h++"
#include "Mustard/Utility/VectorCast.h++"

#include "CLHEP/Random/RandomEngine.h"

#include "Eigen/Cholesky"

#include "muc/math"
#include "muc/numeric"

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <optional>
#include <utility>

namespace Mustard::inline Physics::inline Generator {

/// @class AdaptiveMTMGenerator
/// @brief Adaptive Multiple-try Metropolis (aMTM) MCMC event generator,
/// possibly with user-defined acceptance.
///
/// Generates events distributed according to |M|² × acceptance, and
/// weight = 1 / acceptance.
///
/// Advanced MCMC sampler that uses multiple trial points per iteration to
/// improve sampling efficiency in high-dimensional spaces. Implements adaptive
/// covariance estimation to optimize proposal distributions.
///
/// The Markov chain requires reinitialize after each change to
/// initial-state momenta. So this generator is unsuitable
/// for case where frequent variation of initial-state momenta is required.
///
/// @tparam M Number of initial-state particles
/// @tparam N Number of final-state particles
/// @tparam A Matrix element of the process to be generated
template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
class AdaptiveMTMGenerator : public MCMCGenerator<M, N, A> {
private:
    /// @brief The base class
    using Base = MCMCGenerator<M, N, A>;

protected:
    /// @brief Initial-state 4-momentum (or container type when M>1)
    using typename Base::InitialStateMomenta;
    /// @brief Markov chain state container
    using typename Base::MarkovChain;

public:
    /// @brief Construct event generator
    /// @param pI initial-state 4-momenta
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    /// @param thinningRatio Thinning factor (between 0--1, optional, use default value if not set)
    /// @param acfSampleSize Sample size for estimation autocorrelation function (ACF) (optional, use default value if not set)
    AdaptiveMTMGenerator(const InitialStateMomenta& pI, const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                         std::optional<double> thinningRatio = {}, std::optional<unsigned> acfSampleSize = {});
    /// @brief Construct event generator
    /// @param pI initial-state 4-momenta
    /// @param polarization Initial-state polarization vector
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    /// @param thinningRatio Thinning factor (between 0--1, optional, use default value if not set)
    /// @param acfSampleSize Sample size for estimation autocorrelation function (ACF) (optional, use default value if not set)
    /// @note This overload is only enabled for polarized decay
    AdaptiveMTMGenerator(const InitialStateMomenta& pI, CLHEP::Hep3Vector polarization,
                         const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                         std::optional<double> thinningRatio = {}, std::optional<unsigned> acfSampleSize = {})
        requires std::derived_from<A, QFT::PolarizedMatrixElement<1, N>>;
    /// @brief Construct event generator
    /// @param pI initial-state 4-momenta
    /// @param polarization Initial-state polarization vectors
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    /// @param thinningRatio Thinning factor (between 0--1, optional, use default value if not set)
    /// @param acfSampleSize Sample size for estimation autocorrelation function (ACF) (optional, use default value if not set)
    /// @note This overload is only enabled for polarized scattering
    AdaptiveMTMGenerator(const InitialStateMomenta& pI, const std::array<CLHEP::Hep3Vector, M>& polarization,
                         const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                         std::optional<double> thinningRatio = {}, std::optional<unsigned> acfSampleSize = {})
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1);

    // Keep the class abstract
    virtual ~AdaptiveMTMGenerator() override = 0;

private:
    /// @brief Markov chain burn in stage
    /// @param rng Reference to CLHEP random engine
    virtual auto BurnIn(CLHEP::HepRandomEngine& rng) -> void override;
    /// @brief Advance Markov chain by one event
    /// @param rng Reference to CLHEP random engine
    /// @return true if proposal accepted, false if not
    virtual auto NextEvent(CLHEP::HepRandomEngine& rng) -> bool override { return NextEventImpl(rng); }

    /// @brief Advance Markov chain by one event using
    /// adaptive multiple-try Metropolis (aMTM) algorithm
    /// @param rng Reference to CLHEP random engine
    /// @param burnInStepSize Step scale of random walk during burn in stage
    /// @return true if proposal accepted, false if not
    auto NextEventImpl(CLHEP::HepRandomEngine& rng, double burnInStepSize = 0) -> bool;

private:
    Math::Random::Gaussian<double> fGaussian;                                      ///< Gaussian distribution
    unsigned long long fIteration;                                                 ///< Current iteration count
    double fLearningRate;                                                          ///< Learning rate for adaptation
    Eigen::Vector<double, MarkovChain::dim> fRunningMean;                          ///< Running mean of states
    Eigen::Matrix<double, MarkovChain::dim, MarkovChain::dim> fProposalCovariance; ///< Proposal covariance
    Eigen::Matrix<double, MarkovChain::dim, MarkovChain::dim> fProposalSigma;      ///< Proposal standard deviation

    static constexpr auto fgDefaultACFSampleSize{20000};                          ///< Default ACF sample size
    static constexpr auto fgNTrial{5};                                            ///< Number of trial points
    static constexpr auto fgInitProposalStepSize{0.2};                            ///< Initial proposal step size
    static constexpr auto fgLearningRatePower{-0.6};                              ///< Learning rate decay power. Ref: Simon Fontaine, Mylène Bédard (2022), https://doi.org/10.3150/21-BEJ1408
    static inline const auto fgScalingFactor{3.12 / std::sqrt(MarkovChain::dim)}; ///< Step size scaling factor. Ref: of M. B´edard et al. SPA 122 (2012) 758–786 https://doi.org/10.1016/j.spa.2011.11.004
};

} // namespace Mustard::inline Physics::inline Generator

#include "Mustard/Physics/Generator/AdaptiveMTMGenerator.inl"
