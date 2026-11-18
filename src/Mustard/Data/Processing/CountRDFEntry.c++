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

#include "Mustard/Data/Processing/CountRDFEntry.h++"
#include "Mustard/Env/MPIEnv.h++"

#include "RtypesCore.h"

#include "mp1/mp1.hpp"

#include "gsl/gsl"

#include <future>
#include <vector>

namespace Mustard::Data::inline Processing {

auto CountRDFEntry(ROOT::RDF::RNode rdf) -> long long {
    if (not mp1::available()) {
        return gsl::narrow<long long>(*rdf.Count());
    }
    const auto& worldComm{mp1::comm_world()};
    if (worldComm.size() == 1) {
        return gsl::narrow<long long>(*rdf.Count());
    }
    // Process 0 counts the number of entries and broadcast to other processes
    long long nEntry{};
    if (worldComm.rank() == 0) {
        nEntry = gsl::narrow<long long>(*rdf.Count());
    }
    mp1::bcast(worldComm, 0, nEntry);
    return nEntry;
}

auto CountRDFEntry(std::vector<ROOT::RDF::RNode> rdf) -> gtl::vector<long long> {
    const auto nRDF{ssize(rdf)};
    std::vector<std::future<long long>> nEntryFuture(nRDF);
    gtl::vector<long long> nEntry(nRDF);
    if (not mp1::available() or mp1::comm_world().size() == 1) {
        for (gsl::index k{}; k < nRDF; ++k) {
            nEntryFuture[k] = std::async([&rdf, k] {
                return gsl::narrow<long long>(*rdf[k].Count());
            });
        }
        for (gsl::index k{}; k < nRDF; ++k) {
            nEntry[k] = nEntryFuture[k].get();
        }
        return nEntry;
    }
    // Process k%nNode counts the number of entries and broadcast
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    const auto& intraNodeComm{mpiEnv.IntraNodeComm()};
    const auto& interNodeComm{mpiEnv.InterNodeComm()};
    // Process k%nNode-node leaders count the number of entries and broadcast to other node leaders
    if (interNodeComm.is_valid()) {
        for (gsl::index k{}; k < nRDF; ++k) {
            if (mpiEnv.LocalNodeIdx() == k % mpiEnv.ClusterSize()) {
                nEntryFuture[k] = std::async([&rdf, k] {
                    return gsl::narrow<long long>(*rdf[k].Count());
                });
            }
        }
        std::vector<mp1::request> bcastNEntry;
        bcastNEntry.reserve(nRDF);
        for (gsl::index k{}; k < nRDF; ++k) {
            const auto rootNodeIdx{k % mpiEnv.ClusterSize()};
            if (mpiEnv.LocalNodeIdx() == rootNodeIdx) {
                nEntry[k] = nEntryFuture[k].get();
            }
            bcastNEntry.emplace_back(mp1::ibcast(interNodeComm, rootNodeIdx, nEntry[k]));
        }
        mp1::wait_all(bcastNEntry);
    }
    // Node leaders broadcast the number of entries to other processes in the same node
    mp1::bcast(intraNodeComm, 0, nEntry);
    return nEntry;
}

} // namespace Mustard::Data::inline Processing
