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

namespace Mustard::Data::inline Processing {

template<std::integral T, std::signed_integral U>
SingleRDFEventInfo<T, U>::SingleRDFEventInfo(ROOT::RDF::RNode rdf, std::string eventIDColumnName, int rootNodeIdx,
                                             std::optional<std::pair<mplr::communicator, mplr::communicator>> intraInterNodeComm) :
    fIntraInterNodeComm{},
    fEventID{},
    fEntry{} {
    if (ROOT::IsImplicitMTEnabled()) {
        Throw<std::runtime_error>("ROOT IMT enabled. Cannot build RDF event info");
    }

    muc::chrono::stopwatch stopwatch;
    MasterPrintLn("Building single-RDF event info...");
    const auto _{gsl::finally([&] {
        const muc::chrono::seconds<double> elapsed{stopwatch.read()};
        MasterPrintLn("Finished building single-RDF event info in {:.2f}s.", elapsed.count());
    })};

    const auto buildData{[&rdf, &eventIDColumnName] {
        gtl::flat_hash_set<T> eventIDSet;
        gtl::vector<T> eventID;
        gtl::vector<EntryType> entry;
        eventID.reserve(1024);
        entry.reserve(1024);
        eventIDSet.reserve(1024);
        EntryType lastEntry{-1};
        const auto appendEntry{[&](T evtID, ULong64_t uEntry) {
            lastEntry = gsl::narrow_cast<EntryType>(uEntry);
            if (not eventID.empty() and evtID == eventID.back()) {
                return;
            }
            const auto [_, uniqueEventID]{eventIDSet.emplace(evtID)};
            if (not uniqueEventID) {
                Throw<std::runtime_error>(fmt::format("Event {} is not successive", evtID));
            }
            eventID.emplace_back(evtID);
            entry.emplace_back(lastEntry);
        }};
        rdf.Foreach(appendEntry, {std::move(eventIDColumnName), "rdfentry_"});
        entry.emplace_back(lastEntry + 1); // Sentinel
        return std::pair{std::move(eventID), std::move(entry)};
    }};

    // Non-MPI path: build and store locally
    if (not mplr::available() or mplr::comm_world().size() == 1) {
        auto [eventID, entry]{buildData()};
        fEventID = Parallel::SharedMemory<T>{eventID};
        fEntry = Parallel::SharedMemory<EntryType>{entry};
        return;
    }

    // Check rootNodeIdx validity before building data
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    if (rootNodeIdx < 0 or mpiEnv.ClusterSize() <= rootNodeIdx) {
        Throw<std::out_of_range>(fmt::format("Invalid root node index {}: must be in [0, {})", rootNodeIdx, mpiEnv.ClusterSize()));
    }

    // Store communicators for the lifetime of shared memory windows.
    // fIntraInterNodeComm is destroyed after fEventID/fEntry (reverse declaration order),
    // ensuring MPI windows are freed before their communicator.
    if (intraInterNodeComm) {
        fIntraInterNodeComm = std::move(intraInterNodeComm);
    }
    const auto& intraNodeComm{fIntraInterNodeComm ? fIntraInterNodeComm->first : mpiEnv.IntraNodeComm()};
    const auto& interNodeComm{fIntraInterNodeComm ? fIntraInterNodeComm->second : mpiEnv.InterNodeComm()};

    // Build data on root node leader and broadcast to other node leaders
    gtl::vector<T> eventID;
    gtl::vector<EntryType> entry;
    std::size_t nEvent;
    if (interNodeComm.is_valid()) { // Only node leaders have valid inter-node communicator
        if (mpiEnv.LocalNodeIdx() == rootNodeIdx) {
            std::tie(eventID, entry) = buildData();
            nEvent = eventID.size();
        }
        interNodeComm.ibcast(rootNodeIdx, nEvent) // Other leaders lazy-spin here to reduce resources consumption
            .wait(mplr::duty_ratio::preset::relaxed);
        eventID.resize(nEvent);
        entry.resize(nEvent + 1);
        mplr::irequest_pool bcastData;
        bcastData.push(
            interNodeComm.ibcast(rootNodeIdx, eventID.data(), mplr::vector_layout<T>{eventID.size()}));
        bcastData.push(
            interNodeComm.ibcast(rootNodeIdx, entry.data(), mplr::vector_layout<EntryType>{entry.size()}));
        bcastData.waitall(mplr::duty_ratio::preset::active);
    }
    // Broadcast nEvent from node leaders to intra-node peers;
    // non-leaders lazy-spin here to reduce resources consumption
    intraNodeComm.ibcast(0, nEvent)
        .wait(mplr::duty_ratio::preset::relaxed);

    // Build SharedMemory: rank 0 (node leader) provides data, others attach.
    fEventID = Parallel::SharedMemory<T>{eventID, 0, intraNodeComm};
    fEntry = Parallel::SharedMemory<EntryType>{entry, 0, intraNodeComm};
}

template<std::integral T, std::size_t N, std::signed_integral U>
    requires(N >= 2)
MultiRDFEventInfo<T, N, U>::MultiRDFEventInfo(std::array<ROOT::RDF::RNode, N> rdf, const std::string& eventIDColumnName) :
    MultiRDFEventInfo{
        std::move(rdf),
        [&eventIDColumnName] {
            std::array<std::string, N> columnName;
            columnName.fill(eventIDColumnName);
            return columnName;
        }()} {}

template<std::integral T, std::size_t N, std::signed_integral U>
    requires(N >= 2)
MultiRDFEventInfo<T, N, U>::MultiRDFEventInfo(std::array<ROOT::RDF::RNode, N> rdf, std::array<std::string, N> eventIDColumnName) :
    fToLocalEvtIdx{},
    fToGlobEvtIdx{},
    fMinLocalEvtIdxAfter{},
    fPerRDFEventInfo{} {
    if (ROOT::IsImplicitMTEnabled()) {
        Throw<std::runtime_error>("ROOT IMT enabled. Cannot build RDF event info");
    }
    constexpr auto nRDF{static_cast<gsl::index>(N)};

    muc::chrono::stopwatch stopwatch;
    MasterPrintLn("Building multi-RDF event info...");
    const auto _{gsl::finally([&] {
        const muc::chrono::seconds<double> elapsed{stopwatch.read()};
        MasterPrintLn("Finished building multi-RDF event info in {:.2f}s.", elapsed.count());
    })};

    if (not mplr::available() or mplr::comm_world().size() == 1) {
        // Build per-RDF event info in parallel
        std::array<std::future<Arc<SingleRDFEventInfo<T, U>>>, N> perRDFEventInfoFuture;
        for (gsl::index k{}; k < nRDF; ++k) {
            perRDFEventInfoFuture[k] = std::async([&, k] {
                return MakeArc<SingleRDFEventInfo<T, U>>(std::move(rdf[k]), std::move(eventIDColumnName[k]));
            });
        }
        for (gsl::index k{}; k < nRDF; ++k) {
            fPerRDFEventInfo[k] = perRDFEventInfoFuture[k].get();
        }
        // Build event index data and store in SharedMemory
        auto [toLocalEvtIdx, toGlobEvtIdx, minLocalEvtIdxAfter]{BuildData()};
        fToLocalEvtIdx = Parallel::SharedMemory<std::array<U, N>>{toLocalEvtIdx};
        for (gsl::index k{}; k < nRDF; ++k) {
            fToGlobEvtIdx[k] = Parallel::SharedMemory<U>{toGlobEvtIdx[k]};
        }
        fMinLocalEvtIdxAfter = Parallel::SharedMemory<std::array<U, N>>{minLocalEvtIdxAfter};
        return;
    }

    // We will duplicate communicators for thread safety
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    const auto& intraNodeComm{mpiEnv.IntraNodeComm()};
    const auto& interNodeComm{mpiEnv.InterNodeComm()};
    mplr::info commInfo;
    commInfo.set("mpi_assert_no_any_tag", "true");
    commInfo.set("mpi_assert_no_any_source", "true");

    // Build and broadcast single-RDF event infos
    std::array<std::future<Arc<SingleRDFEventInfo<T, U>>>, N> perRDFEventInfoFuture;
    // Node k%nNode build per-RDF event info for RDF k
    for (gsl::index k{}; k < nRDF; ++k) {
        std::pair<mplr::communicator, mplr::communicator> dupIntraInterNodeComm;
        dupIntraInterNodeComm.first = mplr::communicator{intraNodeComm, commInfo};
        if (interNodeComm.is_valid()) {
            dupIntraInterNodeComm.second = mplr::communicator{interNodeComm, commInfo};
        }
        perRDFEventInfoFuture[k] = std::async([&, k, comm = std::move(dupIntraInterNodeComm)]() mutable {
            const auto rootNodeIdx{k % mpiEnv.ClusterSize()};
            return MakeArc<SingleRDFEventInfo<T, U>>(std::move(rdf[k]), std::move(eventIDColumnName[k]),
                                                     rootNodeIdx, std::move(comm));
        });
    }
    // Wait until per-RDF event info is available and then stores the pointer
    for (gsl::index k{}; k < nRDF; ++k) {
        fPerRDFEventInfo[k] = perRDFEventInfoFuture[k].get();
    }

    // Build event index data on every node leader
    gtl::vector<std::array<U, N>> toLocalEvtIdx;
    std::array<gtl::vector<U>, N> toGlobEvtIdx;
    gtl::vector<std::array<U, N>> minLocalEvtIdxAfter;
    std::size_t nEvent;
    if (interNodeComm.is_valid()) { // Only node leaders have valid inter-node communicator
        std::tie(toLocalEvtIdx, toGlobEvtIdx, minLocalEvtIdxAfter) = BuildData();
        nEvent = toLocalEvtIdx.size();
    }
    // Broadcast nEvent from node leaders to intra-node peers;
    // non-leaders lazy-spin here to reduce resources consumption
    intraNodeComm.ibcast(0, nEvent)
        .wait(mplr::duty_ratio::preset::moderate);

    // Build SharedMemory: rank 0 (node leader) provides data, others attach.
    fToLocalEvtIdx = Parallel::SharedMemory<std::array<U, N>>{toLocalEvtIdx};
    for (gsl::index k{}; k < nRDF; ++k) {
        fToGlobEvtIdx[k] = Parallel::SharedMemory<U>{toGlobEvtIdx[k]};
    }
    fMinLocalEvtIdxAfter = Parallel::SharedMemory<std::array<U, N>>{minLocalEvtIdxAfter};
}

template<std::integral T, std::size_t N, std::signed_integral U>
    requires(N >= 2)
auto MultiRDFEventInfo<T, N, U>::BuildData() const -> std::tuple<gtl::vector<std::array<U, N>>,
                                                                 std::array<gtl::vector<U>, N>,
                                                                 gtl::vector<std::array<U, N>>> {
    Ensures(std::ranges::all_of(fPerRDFEventInfo, [](auto&& eventInfo) { return eventInfo != nullptr; }));
    constexpr auto nRDF{static_cast<gsl::index>(N)};

    // Build event-local-index map for each RDF
    std::array<gtl::flat_hash_map<T, U>, N> evtIDToLocalIdx;
    for (gsl::index k{}; k < nRDF; ++k) {
        evtIDToLocalIdx[k].reserve(RDF(k).NEvent());
        for (U localIdx{}; localIdx < muc::to_signed(RDF(k).NEvent()); ++localIdx) {
            evtIDToLocalIdx[k].emplace(RDF(k).EventID(localIdx), localIdx);
        }
    }

    // Result data
    gtl::vector<std::array<U, N>> toLocalEvtIdx;
    std::array<gtl::vector<U>, N> toGlobEvtIdx;
    gtl::vector<std::array<U, N>> minLocalEvtIdxAfter;

    // Build alignment table from event-index maps
    std::array<std::size_t, N> nEvent;
    std::ranges::transform(evtIDToLocalIdx, nEvent.begin(), [](auto&& map) { return map.size(); });
    toLocalEvtIdx.reserve(*std::ranges::max_element(nEvent));
    for (gsl::index k{}; k < nRDF; ++k) {
        for (auto&& [eventID, evtIdx] : std::as_const(evtIDToLocalIdx[k])) {
            if (evtIdx == -1) {
                continue;
            }
            auto& idx{toLocalEvtIdx.emplace_back()};
            for (gsl::index j{}; j < k; ++j) {
                idx[j] = -1;
            }
            idx[k] = evtIdx;
            for (gsl::index j{k + 1}; j < nRDF; ++j) {
                if (const auto iIdx{evtIDToLocalIdx[j].find(eventID)};
                    iIdx != evtIDToLocalIdx[j].cend() and iIdx->second != -1) {
                    idx[j] = iIdx->second;
                    iIdx->second = -1;
                    continue;
                }
                idx[j] = -1;
            }
        }
        evtIDToLocalIdx[k] = {};
    }

    // Build priority order of RDFs by descending NEntry
    std::array<gsl::index, N> priority;
    muc::ranges::iota(priority, 0);
    muc::timsort(priority, [&](auto k1, auto k2) {
        return fPerRDFEventInfo[k1]->NEntry() > fPerRDFEventInfo[k2]->NEntry();
    });

    // Sort toLocalEvtIdx with priority-projection
    muc::timsort(toLocalEvtIdx, [&](const auto& lhs, const auto& rhs) {
        auto k1{*std::ranges::find_if(priority, [&lhs](auto k) { return lhs[k] != -1; })};
        auto k2{*std::ranges::find_if(priority, [&rhs](auto k) { return rhs[k] != -1; })};
        return lhs[k1] < rhs[k2];
    });

    /* // Print toLocalEvtIdx as CSV for debugging
    fmt::print("toLocalEvtIdx:\n");
    for (auto&& idx : toLocalEvtIdx) {
        for (gsl::index k{}; k < nRDF; ++k) {
            fmt::print("{}{}", idx[k], k == nRDF - 1 ? "" : ",");
        }
        fmt::print("\n");
    } */

    // Build toGlobEvtIdx for each RDF
    for (gsl::index k{}; k < nRDF; ++k) {
        toGlobEvtIdx[k].resize(RDF(k).NEvent());
        for (U globIdx{}; globIdx < ssize(toLocalEvtIdx); ++globIdx) {
            const auto localIdx{toLocalEvtIdx[globIdx][k]};
            if (localIdx == -1) {
                continue;
            }
            toGlobEvtIdx[k][localIdx] = globIdx;
        }
    }

    // Build minLocalEvtIdxAfter for each RDF
    minLocalEvtIdxAfter.resize(toLocalEvtIdx.size());
    if (not toLocalEvtIdx.empty()) {
        auto minLocalEvtIdx{toLocalEvtIdx.back()};
        for (gsl::index k{}; k < nRDF; ++k) {
            if (minLocalEvtIdx[k] == -1) {
                minLocalEvtIdx[k] = RDF(k).NEvent();
            }
        }
        minLocalEvtIdxAfter.back() = minLocalEvtIdx;
        for (auto globIdx{gsl::narrow_cast<U>(ssize(toLocalEvtIdx) - 2)}; globIdx >= 0; --globIdx) {
            for (gsl::index k{}; k < nRDF; ++k) {
                const auto localIdx{toLocalEvtIdx[globIdx][k]};
                if (localIdx == -1) {
                    continue;
                }
                minLocalEvtIdx[k] = std::min(minLocalEvtIdx[k], localIdx);
            }
            minLocalEvtIdxAfter[globIdx] = minLocalEvtIdx;
        }
    }

    /* // Print minLocalEvtIdxAfter for debugging
    fmt::print("minLocalEvtIdxAfter:\n");
    for (auto&& idx : minLocalEvtIdxAfter) {
        for (gsl::index k{}; k < nRDF; ++k) {
            fmt::print("{}{}", idx[k], k == nRDF - 1 ? "" : ",");
        }
        fmt::print("\n");
    } */

    return {std::move(toLocalEvtIdx), std::move(toGlobEvtIdx), std::move(minLocalEvtIdxAfter)};
}

} // namespace Mustard::Data::inline Processing
