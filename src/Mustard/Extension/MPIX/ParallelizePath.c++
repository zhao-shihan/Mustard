// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
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

#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Extension/MPIX/ParallelizePath.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "mplr/mplr.hpp"

#include "fmt/format.h"

#include <cstddef>
#include <exception>

namespace Mustard::inline Extension::MPIX {

auto ParallelizePath(const std::filesystem::path& path) -> std::filesystem::path {
    auto stem{path.stem()};
    if (stem.empty()) {
        Throw<std::invalid_argument>("Empty file name");
    }
    if (stem == "." or stem == "..") {
        Throw<std::invalid_argument>(fmt::format("Invalid file name '{}'", stem.c_str()));
    }

    const auto& worldComm{mplr::comm_world()};
    if (worldComm.size() == 1) {
        return path;
    }

    const auto& mpiEnv{Env::MPIEnv::Instance()};
    // parent directory
    auto parent{std::filesystem::path{path}.replace_extension()};
    if (mpiEnv.OnCluster()) {
        parent /= mpiEnv.LocalNode().name;
    }
    // create parent directory
    const auto& intraNodeComm{mpiEnv.IntraNodeComm()};
    if (intraNodeComm.rank() == 0) {
        std::filesystem::create_directories(parent);
    }
    // wait for create_directories
    intraNodeComm.barrier();
    // construct full path
    return parent / stem.concat(fmt::format("_mpi{}.", worldComm.rank())).replace_extension(path.extension());
}

} // namespace Mustard::inline Extension::MPIX
