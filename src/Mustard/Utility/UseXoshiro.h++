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

#include "Mustard/CLI/CLI.h++"

#include "muc/optional"

#include <memory>
#include <type_traits>

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
/// @see CLHEPX::Random::Xoshiro256StarStar, CLHEPX::Random::Xoshiro512StarStar
/// @see ROOTX::Math::Xoshiro256PlusPlus, ROOTX::Math::Xoshiro512PlusPlus
template<unsigned ABitWidth>
class UseXoshiro {
public:
    /// @brief Initialize with automatic seeding
    /// @param cli MonteCarloModule CLI interface for seed configuration (optional)
    /// @note Also performs decorrelation between CLHEP and ROOT engines
    ///       and parallel computing reseeding
    explicit UseXoshiro(muc::optional_ref<const CLI::CLI<>> cli = {});
    /// @brief Clean up and reset global engine pointers
    ~UseXoshiro();

private:
    /// @brief Xoshiro 256 random engine storage
    struct Random256;
    /// @brief Xoshiro 512 random engine storage
    struct Random512;
    /// @brief Random engine storage
    using Random = std::conditional_t<
        ABitWidth == 256,
        Random256,
        std::conditional_t<
            ABitWidth == 512,
            Random512,
            void>>;

private:
    std::unique_ptr<Random> fRandom; ///< Engine instances
};

extern template class UseXoshiro<256>;
extern template class UseXoshiro<512>;

} // namespace Mustard::inline Utility
