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
#include "Mustard/Utility/UseXoshiro.h++"

namespace Mustard::Env {

/// @brief Environment providing pseudo-random number engines for Monte Carlo simulations.
/// @tparam AXoshiroWidth  Xoshiro state width (256 or 512).
/// @details Wraps a `UseXoshiro<AXoshiroWidth>` instance, which owns one single
/// Xoshiro engine and installs it as both CLHEP's global engine and ROOT's
/// gRandom. That unified engine is reachable in three equivalent ways: the
/// native engine, a CLHEP-compatible wrapper referring to it, and a
/// ROOT-compatible wrapper referring to it. Drawing through any of them
/// advances the very same random number stream.
/// Inherits basic environment infrastructure (CLI, verbosity, banners) from `BasicEnv`.
template<unsigned AXoshiroWidth>
class MonteCarloEnv : virtual public BasicEnv,
                      public PassiveSingleton<MonteCarloEnv<AXoshiroWidth>> {
protected:
    /// @brief Constructs the Monte Carlo environment without printing the start banner.
    /// @param argc  Argument count passed to `main`.
    /// @param argv  Argument vector passed to `main`.
    /// @param cli   CLI object for argument processing.
    /// @param verboseLevel  Logging verbosity threshold.
    /// @param showBannerHint  Whether banner hint should be tracked despite suppression.
    MonteCarloEnv(NoBanner, int argc, char* argv[], CLI::CLI<>& cli,
                  enum VerboseLevel verboseLevel,
                  bool showBannerHint);

public:
    /// @brief Constructs the Monte Carlo environment and optionally prints banners.
    /// @param argc  Argument count passed to `main`.
    /// @param argv  Argument vector passed to `main`.
    /// @param cli   CLI object for argument processing.
    /// @param verboseLevel  Logging verbosity threshold.
    /// @param showBannerHint  Whether start/exit banner behavior is enabled.
    MonteCarloEnv(int argc, char* argv[], CLI::CLI<>& cli,
                  enum VerboseLevel verboseLevel = {},
                  bool showBannerHint = true);

    using PassiveSingleton<MonteCarloEnv>::Instance;
    using PassiveSingleton<MonteCarloEnv>::Available;
    using PassiveSingleton<MonteCarloEnv>::Expired;
    using PassiveSingleton<MonteCarloEnv>::Instantiated;

    /// @brief Returns the unified random engine.
    /// @return Reference to the Xoshiro engine that backs all three interfaces.
    /// @note Not `const`: a mutable reference to the shared engine is handed out.
    auto RandomEngine() -> auto& { return fXoshiro.RandomEngine(); }
    /// @brief Returns the CLHEP handle of the unified random engine.
    /// @return Reference to a `CLHEP::HepRandomEngine` wrapper referring to it.
    auto CLHEPRandomEngine() -> auto& { return fXoshiro.CLHEPRandomEngine(); }
    /// @brief Returns the ROOT handle of the unified random engine.
    /// @return Reference to a `TRandom` wrapper referring to it.
    auto ROOTRandomEngine() -> auto& { return fXoshiro.ROOTRandomEngine(); }

private:
    UseXoshiro<AXoshiroWidth> fXoshiro; ///< Owner of the unified random engine and of its CLHEP and ROOT handles.
};

extern template class MonteCarloEnv<256>;
extern template class MonteCarloEnv<512>;

} // namespace Mustard::Env
