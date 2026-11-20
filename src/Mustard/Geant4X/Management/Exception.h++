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

#include "G4Exception.hh"

#include <source_location>
#include <string>

namespace Mustard::Geant4X::inline Management {

/// @brief Issue a G4Exception with Mustard-level logging.
///
/// Calls the appropriate Mustard print function (`PrintWarning` for `JustWarning`,
/// `PrintError` for all other severities except `IgnoreTheIssue`) before invoking
/// `G4Exception`. All parameters after `message` are optional.
///
/// @param severity The G4Exception severity.
/// @param message  The exception message.
/// @param loc      Source location of the call site. Default is current.
/// @param code     Optional exception code. If empty, a hex hash of the call site is generated.
auto Exception(G4ExceptionSeverity severity,
               const std::string& message,
               const std::source_location& loc = std::source_location::current(),
               const std::string& code = "") -> void;

/// @brief Issue a G4Exception with Mustard-level logging, severity fixed at compile time.
///
/// Equivalent to `Exception(S, message, loc, code)`, but the severity is a template
/// argument rather than a runtime parameter. Useful when the severity is known statically.
///
/// @tparam S     The G4Exception severity (compile-time constant).
/// @param message The exception message.
/// @param loc     Source location of the call site. Default is current.
/// @param code    Optional exception code. If empty, a hex hash of the call site is generated.
template<G4ExceptionSeverity S>
auto Exception(const std::string& message,
               const std::source_location& loc = std::source_location::current(),
               const std::string& code = "") -> void { Exception(S, message, loc, code); }

/// @brief Issue a G4Exception with Mustard-level logging, only from the master process.
///
/// Identical to `Exception`, but the call (both the Mustard print and the `G4Exception`
/// invocation) only executes when MPI is unavailable or the current process has rank 0.
/// All parameters after `message` are optional.
///
/// @param severity The G4Exception severity.
/// @param message  The exception message.
/// @param loc      Source location of the call site. Default is current.
/// @param code     Optional exception code. If empty, a hex hash of the call site is generated.
auto MasterException(G4ExceptionSeverity severity,
                     const std::string& message,
                     const std::source_location& loc = std::source_location::current(),
                     const std::string& code = "") -> void;

/// @brief Issue a G4Exception with Mustard-level logging, only from the master process,
///        severity fixed at compile time.
///
/// Equivalent to `MasterException(S, message, loc, code)`, but the severity is a template
/// argument. Combines the master-process guard with a compile-time severity.
///
/// @tparam S     The G4Exception severity (compile-time constant).
/// @param message The exception message.
/// @param loc     Source location of the call site. Default is current.
/// @param code    Optional exception code. If empty, a hex hash of the call site is generated.
template<G4ExceptionSeverity S>
auto MasterException(const std::string& message,
                     const std::source_location& loc = std::source_location::current(),
                     const std::string& code = "") -> void { MasterException(S, message, loc, code); }

} // namespace Mustard::Geant4X::inline Management
