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

#include "Mustard/CLI/CLI.h++"
#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/Env/ObjectRegistry/PassiveSingleton.h++"
#include "Mustard/Utility/RandomEngineInstaller.h++"

#include "muc/random"

#include <optional>

namespace Mustard::Env {

/// @brief Environment providing a pseudo-random number engine for Monte Carlo simulations.
/// @details Owns a `RandomEngineInstaller`, which owns one single
/// type-erased random engine and installs it as both CLHEP's global engine and
/// ROOT's gRandom. That unified engine is reachable in three equivalent ways:
/// the installed engine, a CLHEP-compatible wrapper referring to it, and a
/// ROOT-compatible wrapper referring to it. Drawing through any of them
/// advances the very same random number stream.
/// The engine is a Xoshiro256** engine unless the caller passes another one or
/// the Monte Carlo CLI module's --rng option selects another one.
/// Inherits basic environment infrastructure (CLI, verbosity, banners) from `BasicEnv`.
class MonteCarloEnv : virtual public BasicEnv,
                      public PassiveSingleton<MonteCarloEnv> {
protected:
    /// @brief Constructs the Monte Carlo environment without printing the start banner.
    /// @param argc  Argument count passed to `main`.
    /// @param argv  Argument vector passed to `main`.
    /// @param cli   CLI object for argument processing.
    /// @param verboseLevel  Logging verbosity threshold.
    /// @param showBannerHint  Whether banner hint should be tracked despite suppression.
    /// @param engine  Random engine to install; a Xoshiro256** engine if empty,
    /// and overridden when --rng is given.
    MonteCarloEnv(NoBanner, int argc, char* argv[], CLI::CLI<>& cli,
                  enum VerboseLevel verboseLevel,
                  bool showBannerHint,
                  std::optional<muc::any_rng> engine);

public:
    /// @brief Constructs the Monte Carlo environment and optionally prints banners.
    /// @param argc  Argument count passed to `main`.
    /// @param argv  Argument vector passed to `main`.
    /// @param cli   CLI object for argument processing.
    /// @param verboseLevel  Logging verbosity threshold.
    /// @param showBannerHint  Whether start/exit banner behavior is enabled.
    /// @param engine  Random engine to install; a Xoshiro256** engine if empty,
    /// and overridden when --rng is given.
    MonteCarloEnv(int argc, char* argv[], CLI::CLI<>& cli,
                  enum VerboseLevel verboseLevel = {},
                  bool showBannerHint = true,
                  std::optional<muc::any_rng> engine = {});

    using PassiveSingleton<MonteCarloEnv>::Instance;
    using PassiveSingleton<MonteCarloEnv>::Available;
    using PassiveSingleton<MonteCarloEnv>::Expired;
    using PassiveSingleton<MonteCarloEnv>::Instantiated;

    /// @brief Returns the unified random engine.
    /// @return Reference to the installed engine, whose concrete type is
    /// recoverable with `target()`.
    /// @note Not `const`: a mutable reference to the shared engine is handed out.
    /// @note A copy of it owns an independent stream; pass a reference or
    /// `ref()` to share the stream.
    auto RandomEngine() -> auto& { return fRandomEngineInstaller.Engine(); }
    /// @brief Returns the CLHEP handle of the unified random engine.
    /// @return Reference to a `CLHEP::HepRandomEngine` wrapper referring to it.
    auto CLHEPRandomEngine() -> auto& { return fRandomEngineInstaller.CLHEPEngine(); }
    /// @brief Returns the ROOT handle of the unified random engine.
    /// @return Reference to a `TRandom` wrapper referring to it.
    auto ROOTRandomEngine() -> auto& { return fRandomEngineInstaller.ROOTEngine(); }

private:
    RandomEngineInstaller fRandomEngineInstaller; ///< Owner of the unified random engine and of its CLHEP and ROOT handles.
};

/// @brief Returns the unified random engine installed in this process.
///
/// The environment installs its engine as CLHEP's global engine, as ROOT's
/// gRandom, and owns it, so that code that does not know its type can still
/// reach the very same random number stream.
///
/// @return Reference to the unified random engine.
/// @throw std::runtime_error If the environment has not been instantiated.
/// @see MonteCarloEnv, RandomEngineInstaller
inline auto RandomEngine() -> auto& { return MonteCarloEnv::Instance().RandomEngine(); }

} // namespace Mustard::Env
