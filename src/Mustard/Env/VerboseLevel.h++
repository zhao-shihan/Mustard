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

namespace Mustard::Env {

/// @brief Logging verbosity levels for Mustard environments.
/// @details Verbosity is checked via `VerboseLevelReach<'L'>()`, which
/// returns true when the configured level reaches or exceeds level `L`.
/// Levels are ordered from most quiet to most verbose, with `Quiet` set
/// to -2 to place it below the default-initialized `Error` (0).
enum struct VerboseLevel {
    Quiet = -2,  ///< Suppresses all output.
    Error,       ///< Only error messages.
    Warning,     ///< Errors and warnings.
    Informative, ///< Errors, warnings, and informational messages.
    Verbose      ///< All messages including debug-level output.
};

} // namespace Mustard::Env
