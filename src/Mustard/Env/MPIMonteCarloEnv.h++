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
#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Env/MonteCarloEnv.h++"
#include "Mustard/Env/ObjectRegistry/PassiveSingleton.h++"
#include "Mustard/Utility/UseXoshiro.h++"

namespace Mustard::Env {

template<unsigned AXoshiroWidth>
class MPIMonteCarloEnv : virtual public MPIEnv,
                         virtual public MonteCarloEnv<AXoshiroWidth>,
                         public PassiveSingleton<MPIMonteCarloEnv<AXoshiroWidth>> {
protected:
    MPIMonteCarloEnv(NoBanner, int argc, char* argv[], CLI::CLI<>& cli,
                     enum VerboseLevel verboseLevel,
                     bool showBannerHint);

public:
    MPIMonteCarloEnv(int argc, char* argv[], CLI::CLI<>& cli,
                     enum VerboseLevel verboseLevel = {},
                     bool showBannerHint = true);

    using PassiveSingleton<MPIMonteCarloEnv>::Instance;
    using PassiveSingleton<MPIMonteCarloEnv>::Available;
    using PassiveSingleton<MPIMonteCarloEnv>::Expired;
    using PassiveSingleton<MPIMonteCarloEnv>::Instantiated;
};

extern template class MPIMonteCarloEnv<256>;
extern template class MPIMonteCarloEnv<512>;

} // namespace Mustard::Env
