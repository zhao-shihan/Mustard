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

#include "Mustard/CLI/Module/MonteCarloModule.h++"

#include <memory>

namespace Mustard::inline Utility {

/// @brief Configures Xoshiro random engines for CLHEP and ROOT frameworks
///
/// RAII wrapper that sets Xoshiro** for CLHEP and Xoshiro++ for ROOT as their
/// respective global random number engines. Manages engine lifetime and ensures
/// proper initialization and cleanup.
///
/// @tparam ABitWidth Xoshiro variant bit width (256 or 512)
///
/// @note Features:
///   - Sets CLHEP's global engine to Xoshiro**
///   - Sets ROOT's global engine to Xoshiro++
///   - Automatic engine lifetime management
///   - Parallel computing reseeding support
///   - CLI-based seeding option
///
/// @warning This class should be instantiated once at application startup
/// @see CLHEPX::Random::Xoshiro256SS, CLHEPX::Random::Xoshiro512SS
/// @see ROOTX::Math::Xoshiro256PP, ROOTX::Math::Xoshiro512PP
template<unsigned ABitWidth>
class UseXoshiro {
public:
    /// @brief Initialize with automatic seeding
    /// @note Also performs decorrelation between CLHEP and ROOT engines
    ///       and parallel computing reseeding
    UseXoshiro();
    /// @brief Initialize with CLI-based seeding
    /// @param cli MonteCarloModule CLI interface for seed configuration
    /// @note Also performs decorrelation between CLHEP and ROOT engines
    ///       and parallel computing reseeding
    explicit UseXoshiro(const CLI::MonteCarloModule& cli);
    /// @brief Clean up and reset global engine pointers
    ~UseXoshiro();

private:
    /// @brief Random engine storage
    struct Random;

private:
    std::unique_ptr<Random> fRandom; ///< Engine instances
};

} // namespace Mustard::inline Utility
