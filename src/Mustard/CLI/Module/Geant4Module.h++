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
#include "Mustard/CLI/Module/ModuleBase.h++"

#include "gsl/gsl"

namespace Mustard::CLI::inline Module {

/// @brief CLI module for Geant4 programs.
///
/// Geant4Module registers the --macro and -i (interactive) arguments.
/// It provides helper methods to query whether a macro file is specified
/// and whether the session should run in interactive mode.
class Geant4Module : public ModuleBase {
public:
    /// @brief Construct and register Geant4 CLI arguments.
    /// @param cli  Owning CLI instance.
    Geant4Module(gsl::not_null<CLI<>*> cli);

    /// @brief Check if a macro file was specified via --macro.
    /// @return The macro file path if present, std::nullopt otherwise.
    auto Macro() const -> auto { return TheCLI()->present("macro"); }

    /// @brief Determine whether the session is interactive.
    ///
    /// A session is considered interactive if no macro file was provided,
    /// or if the -i flag was explicitly given even with a macro file.
    /// @return true if interactive mode should be used.
    auto IsInteractive() const -> auto { return not Macro().has_value() or TheCLI()->get<bool>("-i"); }
};

} // namespace Mustard::CLI::inline Module
