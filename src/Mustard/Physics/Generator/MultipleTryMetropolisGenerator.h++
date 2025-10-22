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

#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Math/Random/Distribution/Gaussian.h++"
#include "Mustard/Physics/Generator/MCMCGenerator.h++"
#include "Mustard/Physics/QFT/MatrixElement.h++"

#include "CLHEP/Random/RandomEngine.h"

#include "muc/math"
#include "muc/numeric"

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>

namespace Mustard::inline Physics::inline Generator {

/// @class MultipleTryMetropolisGenerator
/// @brief Multiple-try Metropolis (MTM) MCMC event generator,
/// possibly with user-defined acceptance.
///
/// Generates events distributed according to |M|² × acceptance, and
/// weight = 1 / acceptance.
///
/// Advanced MCMC sampler that uses multiple trial points per iteration to
/// improve sampling efficiency in high-dimensional spaces.
///
/// The Markov chain requires reinitialize after each change to
/// initial-state momenta. So this generator is unsuitable
/// for case where frequent variation of initial-state momenta is required.
///
/// @tparam M Number of initial-state particles
/// @tparam N Number of final-state particles
/// @tparam A Matrix element of the process to be generated
template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
class MultipleTryMetropolisGenerator : public MCMCGenerator<M, N, A> {
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
    /// @param stepSize Step size (proposal sigma) for proposal increment distribution (optional, use default value if not set)
    MultipleTryMetropolisGenerator(const InitialStateMomenta& pI, const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                   std::optional<double> thinningRatio = {}, std::optional<unsigned> acfSampleSize = {},
                                   std::optional<double> stepSize = {});
    /// @brief Construct event generator
    /// @param pI initial-state 4-momenta
    /// @param polarization Initial-state polarization vector
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    /// @param thinningRatio Thinning factor (between 0--1, optional, use default value if not set)
    /// @param acfSampleSize Sample size for estimation autocorrelation function (ACF) (optional, use default value if not set)
    /// @param stepSize Step size (proposal sigma) for proposal increment distribution (optional, use default value if not set)
    /// @note This overload is only enabled for polarized decay
    MultipleTryMetropolisGenerator(const InitialStateMomenta& pI, CLHEP::Hep3Vector polarization,
                                   const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                   std::optional<double> thinningRatio = {}, std::optional<unsigned> acfSampleSize = {},
                                   std::optional<double> stepSize = {})
        requires std::derived_from<A, QFT::PolarizedMatrixElement<1, N>>;
    /// @brief Construct event generator
    /// @param pI initial-state 4-momenta
    /// @param polarization Initial-state polarization vectors
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    /// @param thinningRatio Thinning factor (between 0--1, optional, use default value if not set)
    /// @param acfSampleSize Sample size for estimation autocorrelation function (ACF) (optional, use default value if not set)
    /// @param stepSize Step size (proposal sigma) for proposal increment distribution (optional, use default value if not set)
    /// @note This overload is only enabled for polarized scattering
    MultipleTryMetropolisGenerator(const InitialStateMomenta& pI, const std::array<CLHEP::Hep3Vector, M>& polarization,
                                   const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                   std::optional<double> thinningRatio = {}, std::optional<unsigned> acfSampleSize = {},
                                   std::optional<double> stepSize = {})
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1);

    // Keep the class abstract
    virtual ~MultipleTryMetropolisGenerator() override = 0;

    /// @brief Set MCMC step size
    /// @param stepSize Step size (proposal sigma) for proposal increment distribution
    auto StepSize(double stepSize) -> void;

private:
    /// @brief Markov chain burn in stage
    /// @param rng Reference to CLHEP random engine
    virtual auto BurnIn(CLHEP::HepRandomEngine& rng) -> void override;
    /// @brief Advance Markov chain by one event
    /// @param rng Reference to CLHEP random engine
    /// @return true if proposal accepted, false if not
    virtual auto NextEvent(CLHEP::HepRandomEngine& rng) -> bool override;

private:
    Math::Random::Gaussian<double> fGaussian; ///< Gaussian distribution
    double fStepSize;                         ///< Step scale along one direction in random state space

    static constexpr auto fgNTrial{5};                                            ///< Number of trial points
    static inline const auto fgScalingFactor{3.12 / std::sqrt(MarkovChain::dim)}; ///< Step size scaling factor. Ref: of M. B´edard et al. SPA 122 (2012) 758–786, https://doi.org/10.1016/j.spa.2011.11.004
};

} // namespace Mustard::inline Physics::inline Generator

#include "Mustard/Physics/Generator/MultipleTryMetropolisGenerator.inl"
