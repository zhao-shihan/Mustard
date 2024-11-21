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

#pragma once

#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Extension/MPIX/DataType.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "ROOT/RDataFrame.hxx"

#include "mpi.h"

#include "gsl/gsl"

#include "fmt/core.h"

#include <concepts>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace Mustard::Data {

template<std::integral T = int>
auto RDFEventSplitPoint(ROOT::RDF::RNode rdf, std::string_view eventIDBranchName) -> std::vector<unsigned>;

template<std::integral T>
struct MasterSlaveRDFEventSplitPoint {
    struct MasterEventSplitPoint {
        T eventID;
        unsigned entry;
    };

    struct SlaveEventRange {
        unsigned first;
        unsigned last;
    };

    std::vector<MasterEventSplitPoint> master;
    std::vector<std::unordered_map<T, SlaveEventRange>> slave;
};

template<std::integral T = int>
auto RDFEventSplitPoint(ROOT::RDF::RNode masterRDF,
                        std::vector<ROOT::RDF::RNode> slaveRDF,
                        std::string_view masterEventIDBranchName,
                        std::vector<std::string> slaveEventIDBranchName = {}) -> MasterSlaveRDFEventSplitPoint<T>;

} // namespace Mustard::Data

#include "Mustard/Data/RDFEventSplitPoint.inl"
