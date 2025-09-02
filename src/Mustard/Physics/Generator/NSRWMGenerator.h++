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

#include "CLHEP/Random/RandomEngine.h"

#include "muc/math"

#include <algorithm>
#include <cmath>
#include <ranges>
#include <concepts>

namespace Mustard::inline Physics::inline Generator {

/// @class NSRWMGenerator
/// @brief Base class for normal symmetric random walk Metropolis (N-SRWM)
/// generators, possibly with user-defined acceptance.
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
class NSRWMGenerator : public MCMCGenerator<M, N, A> {
public:
    // Inherit constructor
    using MCMCGenerator<M, N, A>::MCMCGenerator;

protected:
    /// @brief Markov chain state container
    using typename MCMCGenerator<M, N, A>::MarkovChain;

protected:
    /// @brief Normal symmetric proposal distribution
    /// @param rng Reference to CLHEP random engine
    /// @param delta Step scale along one direction in random state space (0 < delta < 0.5)
    /// @param state0 Initial state
    /// @param state Proposed state (modified in-place)
    auto NSRWMProposeState(CLHEP::HepRandomEngine& rng, double delta,
                           const MarkovChain::State& state0, MarkovChain::State& state) -> void;

private:
    Math::Random::Gaussian<double> fGaussian; ///< Gaussian distribution
};

} // namespace Mustard::inline Physics::inline Generator

#include "Mustard/Physics/Generator/NSRWMGenerator.inl"
