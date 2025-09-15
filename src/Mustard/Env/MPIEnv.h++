// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

#include "mplr/mplr.hpp"

#include "muc/optional"

#include <concepts>
#include <functional>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

namespace Mustard::Env {

class MPIEnv : virtual public BasicEnv,
               public mplr::environment,
               public Memory::PassiveSingleton<MPIEnv> {
protected:
    MPIEnv(NoBanner, int argc, char* argv[],
           muc::optional_ref<CLI::CLI<>> cli,
           enum VerboseLevel verboseLevel,
           bool showBannerHint);

public:
    MPIEnv(int argc, char* argv[],
           muc::optional_ref<CLI::CLI<>> cli = {},
           enum VerboseLevel verboseLevel = {},
           bool showBannerHint = true);

    virtual ~MPIEnv();

    using PassiveSingleton<MPIEnv>::Instance;
    using PassiveSingleton<MPIEnv>::Available;
    using PassiveSingleton<MPIEnv>::Expired;
    using PassiveSingleton<MPIEnv>::Instantiated;

    auto IntraNodeComm() const -> const auto& { return fIntraNodeComm; }
    auto InterNodeComm() const -> const auto& { return fInterNodeComm; }

    auto LocalNodeID() const -> const auto& { return fLocalNodeID; }
    auto LocalNode() const -> const auto& { return fNodeList[fLocalNodeID]; }
    auto NodeList() const -> const auto& { return fNodeList; }
    auto Node(int id) const -> const auto& { return fNodeList.at(id); }
    auto ClusterSize() const -> int { return fNodeList.size(); }
    auto OnSingleNode() const -> auto { return ClusterSize() == 1; }
    auto OnCluster() const -> auto { return ClusterSize() != 1; }

protected:
    auto PrintStartBannerBody(int argc, char* argv[]) const -> void;

private:
    struct Node {
        std::string name;
        int size;
        std::vector<int> worldRank;
    };

private:
    mplr::communicator fIntraNodeComm;
    mplr::communicator fInterNodeComm;

    int fLocalNodeID;
    std::vector<struct Node> fNodeList;
};

} // namespace Mustard::Env
