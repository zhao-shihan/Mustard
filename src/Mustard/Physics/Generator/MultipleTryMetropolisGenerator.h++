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

#include "Mustard/Physics/Generator/NSRWMGenerator.h++"
#include "Mustard/Physics/QFT/MatrixElement.h++"

#include "CLHEP/Random/RandomEngine.h"

#include "muc/numeric"

#include <array>
#include <concepts>

namespace Mustard::inline Physics::inline Generator {

/// @class MultipleTryMetropolisGenerator
/// @brief Multiple-try Metropolis (MTM) MCMC sampler for event generation,
/// possibly with user-defined acceptance. MTM sampler can help resolve the
/// curse of dimensionality.
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
class MultipleTryMetropolisGenerator : public NSRWMGenerator<M, N, A> {
public:
    /// @brief Generated event type
    using typename NSRWMGenerator<M, N, A>::Event;

public:
    // Inherit constructor
    using NSRWMGenerator<M, N, A>::NSRWMGenerator;
    // Keep the class abstract
    virtual ~MultipleTryMetropolisGenerator() override = 0;

private:
    /// @brief Advance Markov chain by one event using multiple-try Metropolis algorithm
    /// @param rng Reference to CLHEP random engine
    /// @param stepSize Step scale of random walk
    virtual auto NextEvent(CLHEP::HepRandomEngine& rng, double stepSize) -> Event override;
};

} // namespace Mustard::inline Physics::inline Generator

#include "Mustard/Physics/Generator/MultipleTryMetropolisGenerator.inl"
