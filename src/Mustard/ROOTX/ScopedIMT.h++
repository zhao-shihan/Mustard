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

#include "Mustard/Utility/NonCopyableBase.h++"

namespace Mustard::ROOTX {

/// @brief RAII guard that enables ROOT's Implicit Multi-Threading (IMT) within a scope.
///
/// @details
/// When constructed, this class checks whether ROOT IMT is currently enabled. If it is not,
/// IMT is enabled with the default configuration (i.e., using the default number of threads
/// determined by ROOT, typically @c std::thread::hardware_concurrency()). Upon destruction,
/// IMT is disabled again — but only if it was originally disabled at construction time.
///
/// This allows a code block that requires IMT to temporarily enable it without interfering
/// with the broader application state: if IMT was already enabled before entering the scope,
/// it remains enabled after the scope exits.
///
/// @note **Limitation: default configuration only.**
/// This class always enables IMT via the parameter-less overload of @c ROOT::EnableImplicitMT().
/// It does **not** provide a way to specify a custom IMT configuration (e.g., a specific
/// number of threads via @c ROOT::EnableImplicitMT(int)).
///
/// @note **Limitation: non-default prior configuration is not preserved.**
/// This implementation tracks only whether IMT was enabled or disabled beforehand (a boolean).
/// If a non-default IMT configuration was previously active (e.g., IMT was enabled with 4
/// threads instead of the default), that specific configuration is lost if this class
/// disables and then re-enables IMT. The re-enabled IMT will use the default configuration,
/// not the original non-default one.
///
/// @note This class is non-copyable and non-movable.
class ScopedIMT : public NonCopyableBase {
public:
    /// @brief Constructs the guard. If IMT is not currently enabled, enables it with
    ///        the default configuration and records that it should be disabled later.
    ScopedIMT();

    /// @brief Destroys the guard. If IMT was disabled at construction time, disables it.
    ~ScopedIMT();

private:
    const bool fIMTWasDisabled; ///< Whether IMT was disabled before this guard was created.
};

} // namespace Mustard::ROOTX
