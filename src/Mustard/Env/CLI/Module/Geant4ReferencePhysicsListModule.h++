// -*- C++ -*-
//
// Copyright 2020-2025  The Mustard development team
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

#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/Env/CLI/Module/ModuleBase.h++"
#include "Mustard/IO/PrettyLog.h++"

#include "G4PhysListFactory.hh"
#include "G4VModularPhysicsList.hh"

#include "muc/ceta_string"
#include "muc/utility"

#include "fmt/core.h"

#include <stdexcept>
#include <string>

namespace Mustard::Env::CLI::inline Module {

template<muc::ceta_string ADefault>
class Geant4ReferencePhysicsListModule : public ModuleBase {
public:
    Geant4ReferencePhysicsListModule(argparse::ArgumentParser& argParser);

    auto PhysicsList() -> G4VModularPhysicsList*;

private:
    G4VModularPhysicsList* fReferencePhysicsList;
};

} // namespace Mustard::Env::CLI::inline Module

#include "Mustard/Env/CLI/Module/Geant4ReferencePhysicsListModule.inl"
