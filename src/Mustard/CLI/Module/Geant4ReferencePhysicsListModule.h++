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
#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/IO/PrettyLog.h++"

#include "G4PhysListFactory.hh"
#include "G4VModularPhysicsList.hh"

#include "muc/ceta_string"
#include "muc/utility"

#include "gsl/gsl"

#include "fmt/core.h"

#include <stdexcept>
#include <string>

namespace Mustard::CLI::inline Module {

/// @brief CLI module for Geant4 reference physics list selection.
///
/// Registers the --physics-list argument and provides PhysicsList() to
/// create (and cache) a G4VModularPhysicsList via G4PhysListFactory.
/// The selected list is validated against Geant4's reference physics lists.
///
/// @tparam ADefault  Compile-time string specifying the default physics list name.
template<muc::ceta_string ADefault>
class Geant4ReferencePhysicsListModule : public ModuleBase {
public:
    /// @brief Construct and register --physics-list argument with a default.
    /// @param cli  Owning CLI instance.
    Geant4ReferencePhysicsListModule(gsl::not_null<CLI<>*> cli);

    /// @brief Get or create the physics list from the --physics-list argument.
    /// @return Pointer to the constructed G4VModularPhysicsList.
    /// @throws std::runtime_error if the selected list is not a valid reference physics list.
    ///
    /// The result is cached; subsequent calls return the same pointer.
    auto PhysicsList() -> G4VModularPhysicsList*;

private:
    G4VModularPhysicsList* fReferencePhysicsList; ///< Cached physics list, or nullptr if not yet created.
};

} // namespace Mustard::CLI::inline Module

#include "Mustard/CLI/Module/Geant4ReferencePhysicsListModule.inl"
