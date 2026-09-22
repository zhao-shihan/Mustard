// -*- C++ -*-
//
// Copyright (C) 2020-2026  Shihan Zhao and contributors
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

#include "Mustard/CLHEPX/Random/AsHepRandomEngine.h++"
#include "Mustard/CLI/CLI.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256StarStar.h++"
#include "Mustard/Math/Random/Generator/Xoshiro512StarStar.h++"
#include "Mustard/ROOTX/Math/AsTRandom.h++"
#include "Mustard/Utility/ClassSemanticHelper.h++"

#include "muc/optional"

#include <type_traits>

namespace Mustard::inline Utility {

/// @brief Makes one single Mustard Xoshiro engine the random engine of both
/// CLHEP and ROOT.
///
/// RAII wrapper that owns a Xoshiro** engine and installs it as CLHEP's global
/// engine and ROOT's gRandom. Since both frameworks then work on the very same
/// engine, there is exactly one random number stream in the process, whose
/// state is reachable through three handles:
///   - the owned engine itself, a muc::random_number_engine,
///   - a CLHEPX::Random::AsHepRandomEngine referring to it, as CLHEP's global engine,
///   - a ROOTX::Math::AsTRandom referring to it, as ROOT's gRandom.
///
/// @tparam ABitWidth Xoshiro state width; only 256 and 512 are available.
///
/// @note The engine is seeded from the Monte Carlo CLI module's --seed option
/// when that module is present and the option is given, and is left at its
/// deterministic default state otherwise. Then it is reseeded so that MPI ranks
/// continue with distinct streams.
/// @note Seeding goes through the engine itself, so the CLHEP and ROOT handles
/// keep reporting seed 0 (unknown) until a seed is set through them.
/// @note The engine accessors below are not `const`: they hand out a mutable
/// reference to the shared engine.
/// @note Not copyable nor movable: the CLHEP and ROOT handles refer to the
/// engine owned by this object.
/// @warning This class should be instantiated once at application startup.
/// @see CLHEPX::Random::AsHepRandomEngine, ROOTX::Math::AsTRandom, Parallel::ReseedRandomEngine
template<unsigned ABitWidth>
    requires(ABitWidth == 256 or ABitWidth == 512)
class UseXoshiro : public Utility::NoncopyableBase {
public:
    /// @brief Type of the owned Xoshiro engine.
    using RandomEngineType = std::conditional_t<ABitWidth == 256,
                                                Math::Random::Xoshiro256StarStar,
                                                Math::Random::Xoshiro512StarStar>;
    /// @brief Type of the CLHEP handle of the owned engine.
    using CLHEPRandomEngineType = CLHEPX::Random::AsHepRandomEngine<RandomEngineType&>;
    /// @brief Type of the ROOT handle of the owned engine.
    using ROOTRandomEngineType = ROOTX::Math::AsTRandom<RandomEngineType&>;

    /// @brief Initialize with automatic seeding.
    /// @param cli MonteCarloModule CLI interface for seed configuration (optional)
    explicit UseXoshiro(muc::optional_ref<const CLI::CLI<>> cli = {});
    /// @brief Uninstall the engine from CLHEP and ROOT.
    ~UseXoshiro();

    /// @brief Returns the owned random engine.
    /// @return Reference to the unified Xoshiro engine.
    auto RandomEngine() -> auto& { return fEngine; }
    /// @brief Returns the CLHEP handle of the owned random engine.
    /// @return Reference to the CLHEP wrapper, which is CLHEP's global engine.
    auto CLHEPRandomEngine() -> auto& { return fCLHEPEngine; }
    /// @brief Returns the ROOT handle of the owned random engine.
    /// @return Reference to the ROOT wrapper, which is ROOT's gRandom.
    auto ROOTRandomEngine() -> auto& { return fROOTEngine; }

private:
    RandomEngineType fEngine;           ///< The unified random engine.
    CLHEPRandomEngineType fCLHEPEngine; ///< CLHEP handle of fEngine, installed as CLHEP's global engine.
    ROOTRandomEngineType fROOTEngine;   ///< ROOT handle of fEngine, installed as ROOT's gRandom.
};

extern template class UseXoshiro<256>;
extern template class UseXoshiro<512>;

} // namespace Mustard::inline Utility
