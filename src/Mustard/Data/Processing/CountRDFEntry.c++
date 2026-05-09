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

#include "Mustard/Data/Processing/CountRDFEntry.h++"
#include "Mustard/Env/MPIEnv.h++"

#include "RtypesCore.h"

#include "mplr/mplr.hpp"
#include "gsl/gsl"


#include <future>

namespace Mustard::Data::inline Processing {

auto CountRDFEntry(ROOT::RDF::RNode rdf) -> long long {
    if (not mplr::available()) {
        return gsl::narrow<long long>(*rdf.Count());
    }
    const auto& worldComm{mplr::comm_world()};
    if (worldComm.size() == 1) {
        return gsl::narrow<long long>(*rdf.Count());
    }
    // Process 0 counts the number of entries and broadcast to other processes
    long long nEntry;
    if (worldComm.rank() == 0) {
        nEntry = gsl::narrow<long long>(*rdf.Count());
    }
    worldComm.ibcast(0, nEntry)
        .wait(mplr::duty_ratio::preset::relaxed);
    return nEntry;
}

auto CountRDFEntry(std::vector<ROOT::RDF::RNode> rdf) -> gtl::vector<long long> {
    const auto nRDF{ssize(rdf)};
    std::vector<std::future<long long>> nEntryFuture(nRDF);
    gtl::vector<long long> nEntry(nRDF);
    if (not mplr::available() or mplr::comm_world().size() == 1) {
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
        mplr::irequest_pool bcastNEntry;
        for (gsl::index k{}; k < nRDF; ++k) {
            const auto rootNodeIdx{k % mpiEnv.ClusterSize()};
            if (mpiEnv.LocalNodeIdx() == rootNodeIdx) {
                nEntry[k] = nEntryFuture[k].get();
            }
            bcastNEntry.push(interNodeComm.ibcast(rootNodeIdx, nEntry[k]));
        }
        bcastNEntry.waitall(mplr::duty_ratio::preset::relaxed);
    }
    // Node leaders broadcast the number of entries to other processes in the same node
    intraNodeComm.ibcast(0, nEntry.data(), mplr::vector_layout<long long>(nRDF))
        .wait(mplr::duty_ratio::preset::relaxed);
    return nEntry;
}

} // namespace Mustard::Data::inline Processing
