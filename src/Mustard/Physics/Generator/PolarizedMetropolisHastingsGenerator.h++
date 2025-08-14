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

#include "Mustard/Physics/Amplitude/PolarizedSquaredAmplitude.h++"
#include "Mustard/Physics/Generator/MetropolisHastingsGenerator.h++"

#include "CLHEP/Vector/ThreeVector.h"

#include "muc/numeric"

#include <algorithm>
#include <array>
#include <concepts>

namespace Mustard::inline Physics::inline Generator {

/// @class PolarizedMetropolisHastingsGenerator
/// @brief Metropolis-Hastings MCMC sampler for event generation
/// from polarized initial state, possibly with user-defined bias.
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
template<int M, int N, std::derived_from<PolarizedSquaredAmplitude<M, N>> A>
class PolarizedMetropolisHastingsGenerator : public MetropolisHastingsGenerator<M, N, A> {
public:
    /// @brief Construct event generator
    /// @param cmsE Center-of-mass energy
    /// @param polarization Initial-state polarization vectors
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    /// @param delta Maximun step along one direction in random state space (0--0.5, should be small)
    /// @param discard Samples discarded between two events generated in the Markov chain
    PolarizedMetropolisHastingsGenerator(double cmsE, const std::array<CLHEP::Hep3Vector, M>& polarization,
                                         const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                         double delta, int discard);

    /// @brief Get polarization vector for single initial particle
    /// @param i Particle index (0 ≤ i < M)
    auto InitialStatePolarization(int i) const -> auto { return this->fSquaredAmplitude.InitialStatePolarization(i); }
    /// @brief Get all polarization vectors
    auto InitialStatePolarization() const -> const auto& { return this->fSquaredAmplitude.InitialStatePolarization(); }

    /// @brief Set polarization for single initial particle
    /// @param i Particle index (0 ≤ i < M)
    /// @param polarization Polarization vector (|p| ≤ 1)
    /// @note Triggers Markov chain reset (requires new burn-in)
    auto InitialStatePolarization(int i, CLHEP::Hep3Vector p) -> void;
    /// @brief Set all polarization vectors
    /// @param polarization Array of polarization vectors for each initial particle (all |p| ≤ 1)
    /// @note Triggers Markov chain reset (requires new burn-in)
    auto InitialStatePolarization(const std::array<CLHEP::Hep3Vector, M>& p) -> void;
};

/// @class PolarizedMetropolisHastingsGenerator<1, N, A>
/// @brief Specialization for 1-body initial state
///
/// @tparam N Number of final-state particles
/// @tparam A Squared amplitude of the process to be generated
template<int N, std::derived_from<PolarizedSquaredAmplitude<1, N>> A>
class PolarizedMetropolisHastingsGenerator<1, N, A> : public MetropolisHastingsGenerator<1, N, A> {
public:
    /// @brief Construct event generator
    /// @param cmsE Center-of-mass energy
    /// @param polarization Initial-state polarization vector
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    /// @param delta Maximun step along one direction in random state space (0--0.5, should be small)
    /// @param discard Samples discarded between two events generated in the Markov chain
    PolarizedMetropolisHastingsGenerator(double cmsE, CLHEP::Hep3Vector polarization,
                                         const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                         double delta, int discard);

    /// @brief Get polarization vector
    auto InitialStatePolarization() const -> auto { return this->fSquaredAmplitude.InitialStatePolarization(); }
    /// @brief Set polarization vector
    /// @param p Polarization vector (|p| ≤ 1)
    /// @note Triggers Markov chain reset (requires new burn-in)
    auto InitialStatePolarization(CLHEP::Hep3Vector p) -> void;
};

} // namespace Mustard::inline Physics::inline Generator

#include "Mustard/Physics/Generator/PolarizedMetropolisHastingsGenerator.inl"
