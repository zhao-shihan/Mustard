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

#include "muc/type_traits"

#include "gsl/gsl"

#include <concepts>

namespace Mustard::CLI {

inline namespace Module {
class ModuleBase;
} // namespace Module

template<std::derived_from<ModuleBase>... AModules>
    requires muc::is_type_set_v<AModules...>
class CLI;

inline namespace Module {

/// @brief Base class for CLI modules.
///
/// ModuleBase is the common base for all CLI modules that can be composed
/// into a CLI<AModules...> via mixin inheritance. Each module receives a
/// non-null pointer to the CLI instance it belongs to, allowing modules to
/// register arguments and query parsed values through TheCLI().
///
/// @note Constructors of derived module classes should forward the CLI
///       pointer and register their command-line arguments.
class ModuleBase {
protected:
    /// @brief Construct the module and store a reference to its CLI.
    /// @param cli  Non-null pointer to the owning CLI<> instance.
    ModuleBase(gsl::not_null<CLI<>*> cli);

    /// @brief Const access to the owning CLI instance.
    auto TheCLI() const -> const auto& { return *fTheCLI; }
    /// @brief Mutable access to the owning CLI instance.
    auto TheCLI() -> auto& { return *fTheCLI; }

private:
    CLI<>* fTheCLI;  ///< Non-owning pointer to the parent CLI.
};

} // namespace Module

} // namespace Mustard::CLI
