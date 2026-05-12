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

#include "Mustard/Application/Subprogram.h++"

#include <concepts>
#include <memory>

namespace argparse {
class ArgumentParser;
} // namespace argparse

namespace Mustard::Application {

/// @brief Manages registration and execution of Subprogram-derived commands.
///
/// SubprogramLauncher provides an argument-parser-based dispatch mechanism
/// for a set of Subprogram instances. Subprograms are registered via
/// AddSubprogram() and then launched via Launch(), which parses the
/// command line, selects the matching subprogram, and invokes its Main().
///
/// Usage example:
/// @code
/// SubprogramLauncher launcher;
/// launcher.AddSubprogram<MySimulationSubprogram>();
/// launcher.AddSubprogram<MyAnalysisSubprogram>();
/// // ... add more subprograms as needed
/// return launcher.Launch(argc, argv);
/// @endcode
class SubprogramLauncher {
public:
    /// @brief Construct a SubprogramLauncher with default argument parser.
    SubprogramLauncher();
    /// @brief Destructor.
    ~SubprogramLauncher();

    /// @brief Register a subprogram by type.
    /// @tparam ASubprogram A class derived from Subprogram (default-constructible).
    ///
    /// Convenience overload that constructs and registers ASubprogram in one call.
    template<std::derived_from<Subprogram> ASubprogram>
    auto AddSubprogram() -> void { AddSubprogram(std::make_unique<ASubprogram>()); }
    /// @brief Register a subprogram by pointer.
    /// @param subprogram Unique pointer to a Subprogram instance.
    ///
    /// Takes ownership of the subprogram. Throws if the same subprogram type
    /// is added more than once, or if Launch() has already been called.
    auto AddSubprogram(std::unique_ptr<Subprogram> subprogram) -> void;

    /// @brief Parse the command line and launch the selected subprogram.
    /// @param argc Argument count.
    /// @param argv Argument vector.
    /// @return Exit code returned by the subprogram's Main().
    ///
    /// The first two arguments (argc, argv) are consumed by the launcher's
    /// own parser to identify the subcommand; the remaining arguments are
    /// forwarded to the subprogram. This method may only be called once.
    auto Launch(int argc, char* argv[]) -> int;

private:
    struct State;
    std::unique_ptr<State> fState; ///< PIMPL state holding the parser and subprogram registry.
};

} // namespace Mustard::Application
