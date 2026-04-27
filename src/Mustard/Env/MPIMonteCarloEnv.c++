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

#include "Mustard/Env/MPIMonteCarloEnv.h++"
#include "Mustard/IO/PrettyLog.h++"

#include "mplr/mplr.hpp"

namespace Mustard::Env {

template<unsigned AXoshiroWidth>
MPIMonteCarloEnv<AXoshiroWidth>::MPIMonteCarloEnv(NoBanner, int argc, char* argv[], CLI::CLI<>& cli,
                                                  enum VerboseLevel verboseLevel,
                                                  bool showBannerHint) :
    BasicEnv{{}, argc, argv, cli, verboseLevel, showBannerHint},
    MPIEnv{{}, argc, argv, cli, verboseLevel, showBannerHint},
    MonteCarloEnv<AXoshiroWidth>{{}, argc, argv, cli, verboseLevel, showBannerHint},
    PassiveSingleton<MPIMonteCarloEnv>{this} {}

template<unsigned AXoshiroWidth>
MPIMonteCarloEnv<AXoshiroWidth>::MPIMonteCarloEnv(int argc, char* argv[], CLI::CLI<>& cli,
                                                  enum VerboseLevel verboseLevel,
                                                  bool showBannerHint) :
    MPIMonteCarloEnv{{}, argc, argv, cli, verboseLevel, showBannerHint} {
    if (fShowBanner and (not mplr::available() or mplr::comm_world().rank() == 0)) {
        PrintStartBannerSplitLine();
        PrintStartBannerBody(argc, argv);
        PrintStartBannerSplitLine();
    }
}

template class MPIMonteCarloEnv<256>;
template class MPIMonteCarloEnv<512>;

} // namespace Mustard::Env
