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
#include "Mustard/Env/ObjectRegistry/PassiveSingleton.h++"
#include "Mustard/Env/VerboseLevel.h++"
#include "Mustard/Env/impl/EnvBase.h++"
#include "Mustard/Utility/FunctionAttribute.h++"

#include "muc/optional"

#include "fmt/format.h"

#include <functional>
#include <optional>

namespace Mustard::Env {

/// @brief Passive-singleton environment providing CLI, logging verbosity, and banner infrastructure.
/// @details This is the root of the Mustard environment hierarchy. It holds
/// the argument vector, provides verbose-level queries, prints start and exit
/// banners, and owns the underlying singleton/weak-singleton pools via EnvBase.
/// All higher-level environment classes (MonteCarloEnv, MPIEnv, Geant4Env)
/// inherit from BasicEnv through virtual inheritance.
class BasicEnv : virtual public impl::EnvBase,
                 public PassiveSingleton<BasicEnv> {
protected:
    /// @brief Tag type passed to the protected constructor to suppress banner printing.
    struct NoBanner {};

protected:
    /// @brief Constructs the environment without printing the start banner.
    /// @param argc  Argument count passed to `main`.
    /// @param argv  Argument vector passed to `main`.
    /// @param cli   Optional CLI object for argument processing.
    /// @param verboseLevel  Logging verbosity threshold.
    /// @param showBannerHint  Whether banner hint should be tracked despite suppression.
    BasicEnv(NoBanner, int argc, char* argv[],
             muc::optional_ref<CLI::CLI<>> cli,
             enum VerboseLevel verboseLevel,
             bool showBannerHint);

public:
    /// @brief Constructs the environment and optionally prints start banners.
    /// @param argc  Argument count passed to `main`.
    /// @param argv  Argument vector passed to `main`.
    /// @param cli   Optional CLI object for argument processing.
    /// @param verboseLevel  Logging verbosity threshold.
    /// @param showBannerHint  Whether start/exit banner behavior is enabled.
    BasicEnv(int argc, char* argv[],
             muc::optional_ref<CLI::CLI<>> cli = {},
             enum VerboseLevel verboseLevel = {},
             bool showBannerHint = true);

    /// @brief Destroys the environment, printing the exit banner before teardown.
    virtual ~BasicEnv();

    /// @brief Returns the argument count from `main`.
    /// @return Number of command-line arguments.
    auto Argc() const -> auto { return fArgc; }
    /// @brief Returns the argument vector from `main`.
    /// @return Raw pointer to the command-line argument strings.
    auto Argv() const -> auto { return fArgv; }

    /// @brief Checks whether the current verbose level reaches threshold @p L.
    /// @tparam L  Verbosity threshold character: `'E'` (Error), `'W'` (Warning),
    ///            `'I'` (Informative), or `'V'` (Verbose).
    /// @return True if messages at level @p L should be emitted.
    template<char L>
        requires(L == 'E' or L == 'W' or L == 'I' or L == 'V')
    MUSTARD_ALWAYS_INLINE auto VerboseLevelReach() const -> bool;
    /// @brief Returns the current verbosity level of this environment.
    /// @return The `VerboseLevel` value configured at construction.
    auto VerboseLevel() const -> auto { return fVerboseLevel; }

protected:
    /// @brief Prints a decorative split line above the start banner.
    auto PrintStartBannerSplitLine() const -> void;
    /// @brief Prints the main body of the start banner.
    /// @param argc  Argument count passed to `main`.
    /// @param argv  Argument vector passed to `main`.
    auto PrintStartBannerBody(int argc, char* argv[]) const -> void;
    /// @brief Prints the environment exit/teardown banner.
    auto PrintExitBanner() const -> void;

protected:
    bool fShowBanner; ///< Whether start/exit banners are enabled.

private:
    int fArgc;                       ///< Argument count from `main`.
    char** fArgv;                    ///< Argument vector from `main`.
    enum VerboseLevel fVerboseLevel; ///< Configured logging verbosity threshold.
};

/// @brief Convenience function that queries the BasicEnv singleton's verbosity.
/// @tparam L  Verbosity threshold character: `'E'` (Error), `'W'` (Warning),
///            `'I'` (Informative), or `'V'` (Verbose).
/// @return True if the current global verbose level reaches threshold @p L.
template<char L>
    requires(L == 'E' or L == 'W' or L == 'I' or L == 'V')
MUSTARD_ALWAYS_INLINE auto VerboseLevelReach() -> bool;

} // namespace Mustard::Env

#include "Mustard/Env/BasicEnv.inl"
