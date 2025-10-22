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
#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/Env/Memory/PassiveSingleton.h++"
#include "Mustard/Utility/UseXoshiro.h++"

#include <concepts>

namespace Mustard::Env {

template<unsigned AXoshiroWidth>
class MonteCarloEnv : virtual public BasicEnv,
                      public Memory::PassiveSingleton<MonteCarloEnv<AXoshiroWidth>> {
protected:
    MonteCarloEnv(NoBanner, int argc, char* argv[], CLI::CLI<>& cli,
                  enum VerboseLevel verboseLevel,
                  bool showBannerHint);

public:
    MonteCarloEnv(int argc, char* argv[], CLI::CLI<>& cli,
                  enum VerboseLevel verboseLevel = {},
                  bool showBannerHint = true);

    using PassiveSingleton<MonteCarloEnv>::Instance;
    using PassiveSingleton<MonteCarloEnv>::Available;
    using PassiveSingleton<MonteCarloEnv>::Expired;
    using PassiveSingleton<MonteCarloEnv>::Instantiated;

private:
    UseXoshiro<AXoshiroWidth> fXoshiro;
};

extern template class MonteCarloEnv<256>;
extern template class MonteCarloEnv<512>;

} // namespace Mustard::Env
