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
#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Env/MonteCarloEnv.h++"
#include "Mustard/Env/ObjectRegistry/PassiveSingleton.h++"

#include "muc/random"

#include <optional>

namespace Mustard::Env {

/// @brief Environment combining MPI distribution with Monte Carlo random engines.
/// @details Uses diamond virtual inheritance to combine MPI distributed computing
/// (`MPIEnv`) with pseudo-random number generation (`MonteCarloEnv`), which
/// registers this environment as a passive singleton.
class MPIMonteCarloEnv : virtual public MPIEnv,
                         virtual public MonteCarloEnv,
                         public PassiveSingleton<MPIMonteCarloEnv> {
protected:
    /// @brief Constructs the MPI Monte Carlo environment without printing the start banner.
    /// @param argc  Argument count passed to `main`.
    /// @param argv  Argument vector passed to `main`.
    /// @param cli   CLI object for argument processing.
    /// @param verboseLevel  Logging verbosity threshold.
    /// @param showBannerHint  Whether banner hint should be tracked despite suppression.
    /// @param engine  Random engine to install; overridden when --rng is given.
    MPIMonteCarloEnv(NoBanner, int argc, char* argv[], CLI::CLI<>& cli,
                     enum VerboseLevel verboseLevel,
                     bool showBannerHint,
                     std::optional<muc::any_rng> engine);

public:
    /// @brief Constructs the MPI Monte Carlo environment and optionally prints banners.
    /// @param argc  Argument count passed to `main`.
    /// @param argv  Argument vector passed to `main`.
    /// @param cli   CLI object for argument processing.
    /// @param verboseLevel  Logging verbosity threshold.
    /// @param showBannerHint  Whether start/exit banner behavior is enabled.
    /// @param engine  Random engine to install; overridden when --rng is given.
    MPIMonteCarloEnv(int argc, char* argv[], CLI::CLI<>& cli,
                     enum VerboseLevel verboseLevel = {},
                     bool showBannerHint = true,
                     std::optional<muc::any_rng> engine = {});

    using PassiveSingleton<MPIMonteCarloEnv>::Instance;
    using PassiveSingleton<MPIMonteCarloEnv>::Available;
    using PassiveSingleton<MPIMonteCarloEnv>::Expired;
    using PassiveSingleton<MPIMonteCarloEnv>::Instantiated;
};

} // namespace Mustard::Env
