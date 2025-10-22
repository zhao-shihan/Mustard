// -*- C++ -*-
//
// Copyright (C) 2020-2025  Mustard developers
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

#include "Mustard/CLI/Module/ModuleBase.h++"

#include "gsl/gsl"

namespace Mustard::CLI::inline Module {

/// @brief CLI module for Monte Carlo program
class MonteCarloModule : public ModuleBase {
public:
    /// @brief Construct Monte Carlo CLI module
    /// @param cli CLI instance to register commands with
    MonteCarloModule(gsl::not_null<CLI<>*> cli);

    /// @brief Apply seed configuration if --seed flag was provided
    /// @return true if seeding was performed, false if no --seed flag provided
    /// @warning If this returns true, one MUST call Parallel::ReseedRandomEngine()
    ///          to ensure proper seeding in parallel environments
    auto SeedRandomIfFlagged() const -> bool;
};

} // namespace Mustard::CLI::inline Module
