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

#include "Mustard/CLI/CLI.h++"
#include "Mustard/CLI/Module/Geant4Module.h++"
#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Env/Memory/PassiveSingleton.h++"
#include "Mustard/Env/MonteCarloEnv.h++"

#include <concepts>

namespace Mustard::Env {

class Geant4Env : virtual public MPIEnv,
                  virtual public MonteCarloEnv<512>,
                  public Memory::PassiveSingleton<Geant4Env> {
protected:
    Geant4Env(NoBanner, int argc, char* argv[], CLI::CLI<>& cli,
              enum VerboseLevel verboseLevel,
              bool showBannerHint);

public:
    Geant4Env(int argc, char* argv[], CLI::CLI<>& cli,
              enum VerboseLevel verboseLevel = {},
              bool showBannerHint = true);

    using PassiveSingleton<Geant4Env>::Instance;
    using PassiveSingleton<Geant4Env>::Available;
    using PassiveSingleton<Geant4Env>::Expired;
    using PassiveSingleton<Geant4Env>::Instantiated;
};

} // namespace Mustard::Env
