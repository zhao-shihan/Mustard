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

#include "Mustard/CLI/BasicCLI.h++"
#include "Mustard/CLI/Module/ModuleBase.h++"
#include "Mustard/CLI/Module/MonteCarloModule.h++"

#include <concepts>

namespace Mustard::CLI {

/// @brief CLI alias for Monte Carlo applications.
///
/// MonteCarloCLI builds on BasicCLI by adding MonteCarloModule, which
/// provides random seed configuration via the --seed flag. Useful as a
/// base for any simulation that requires configurable seed from command line.
///
/// @tparam AExtraModules  Additional ModuleBase-derived types (must be unique).
///
/// Equivalent to CLI<BasicModule, MonteCarloModule, AExtraModules...>.
template<std::derived_from<ModuleBase>... AExtraModules>
using MonteCarloCLI = BasicCLI<MonteCarloModule,
                               AExtraModules...>;

} // namespace Mustard::CLI
