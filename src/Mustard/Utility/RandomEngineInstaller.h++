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
#include "Mustard/ROOTX/Math/AsTRandom.h++"
#include "Mustard/Utility/ClassSemanticHelper.h++"

#include "muc/optional"
#include "muc/random"

#include <optional>

namespace Mustard::inline Utility {

/// @brief Makes one single Mustard random engine the random engine of both
/// CLHEP and ROOT.
///
/// RAII wrapper that owns a type-erased random engine and installs it as
/// CLHEP's global engine and ROOT's gRandom. Since both frameworks then work on
/// the very same engine, there is exactly one random number stream in the
/// process, whose state is reachable through three handles:
///   - the owned engine itself, a muc::any_rng,
///   - a CLHEPX::Random::AsHepRandomEngine referring to it, as CLHEP's global engine,
///   - a ROOTX::Math::AsTRandom referring to it, as ROOT's gRandom.
///
/// The engine to install is @p engine, a Xoshiro256** engine if it is empty,
/// unless the Monte Carlo CLI module is present and its --rng option is given,
/// in which case the option selects the engine. Then the engine is seeded from
/// that module's --seed option, if the option is given, and is finally reseeded
/// so that MPI ranks continue with distinct streams.
///
/// @note The owned engine is type-erased but owned: swapping the engine behind
/// a live installer is possible, and the concrete engine type is recoverable
/// from the owned engine with `target()`.
/// @note Not copyable nor movable: the CLHEP and ROOT handles refer to the
/// engine owned by this object.
/// @warning This class should be instantiated once at application startup.
/// @see CLHEPX::Random::AsHepRandomEngine, ROOTX::Math::AsTRandom,
/// Parallel::ReseedRandomEngine
class RandomEngineInstaller : public NoncopyableBase {
public:
    /// @brief Initialize with the default engine, or the one --rng selects.
    /// @param cli CLI object whose Monte Carlo module configures the engine (optional)
    explicit RandomEngineInstaller(muc::optional_ref<const CLI::CLI<>> cli = {});
    /// @brief Initialize with a given engine, or the one --rng selects.
    /// @param engine Engine to install; a Xoshiro256** engine if empty, and
    /// overridden when the --rng option is given
    /// @param cli CLI object whose Monte Carlo module configures the engine (optional)
    explicit RandomEngineInstaller(std::optional<muc::any_rng> engine, muc::optional_ref<const CLI::CLI<>> cli = {});
    /// @brief Uninstall the engine from CLHEP and ROOT.
    ~RandomEngineInstaller();

    /// @brief Returns the owned random engine.
    /// @return Reference to the unified random engine.
    auto Engine() -> auto& { return fEngine; }
    /// @brief Returns the CLHEP handle of the owned random engine.
    /// @return Reference to the CLHEP wrapper, which is CLHEP's global engine.
    auto CLHEPEngine() -> auto& { return fCLHEPEngine; }
    /// @brief Returns the ROOT handle of the owned random engine.
    /// @return Reference to the ROOT wrapper, which is ROOT's gRandom.
    auto ROOTEngine() -> auto& { return fROOTEngine; }

private:
    muc::any_rng fEngine;                                          ///< The unified random engine.
    CLHEPX::Random::AsHepRandomEngine<muc::any_rng&> fCLHEPEngine; ///< CLHEP handle of fEngine, installed as CLHEP's global engine.
    ROOTX::Math::AsTRandom<muc::any_rng&> fROOTEngine;             ///< ROOT handle of fEngine, installed as ROOT's gRandom.
};

} // namespace Mustard::inline Utility
