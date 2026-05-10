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
    fEventID{},
    fEntry{},
    fShmWinOrData{MPI_WIN_NULL},
    fIntraInterNodeComm{} {
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
        Data data;
        gtl::flat_hash_set<T> eventIDSet;
        data.eventID.reserve(1024); // Do not reserve(*rdf.Count()) because event count
        data.entry.reserve(1024);   // may be much smaller than entry count, and *rdf.Count()
        eventIDSet.reserve(1024);   // can be expensive to compute for rdf with filters.
        EntryType entry{-1};
        const auto appendEntry{[&](T evtID, ULong64_t uEntry) {
            entry = gsl::narrow_cast<EntryType>(uEntry);
            if (not data.eventID.empty() and evtID == data.eventID.back()) {
                return;
            }
            const auto [_, uniqueEventID]{eventIDSet.emplace(evtID)};
            if (not uniqueEventID) {
                Throw<std::runtime_error>(fmt::format("Event {} is not successive", evtID));
            }
            data.eventID.emplace_back(evtID);
            data.entry.emplace_back(entry);
        }};
        rdf.Foreach(appendEntry, {std::move(eventIDColumnName), "rdfentry_"});
        data.entry.emplace_back(entry + 1); // Sentinel
        return data;
    }};

    // Build data directly in sequential run
    if (not mplr::available() or mplr::comm_world().size() == 1) {
        const auto& data{fShmWinOrData.template emplace<Data>(buildData())};
        fEventID = std::span(data.eventID);
        fEntry = std::span(data.entry);
        return;
    }

    // Check rootNodeIdx validity before building data
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    if (rootNodeIdx < 0 or mpiEnv.ClusterSize() <= rootNodeIdx) {
        Throw<std::out_of_range>(fmt::format("Invalid root node index {}: must be in [0, {})", rootNodeIdx, mpiEnv.ClusterSize()));
    }

    // Use communicators passed in or from environment
    if (intraInterNodeComm) {
        fIntraInterNodeComm = std::move(*intraInterNodeComm);
    }
    const auto& intraNodeComm{fIntraInterNodeComm ? fIntraInterNodeComm->first : mpiEnv.IntraNodeComm()};
    const auto& interNodeComm{fIntraInterNodeComm ? fIntraInterNodeComm->second : mpiEnv.InterNodeComm()};

    // Build data on a specific node leader and broadcast to other node leaders
    Data data;
    std::size_t nEvent;
    if (interNodeComm.is_valid()) { // Only node leaders have valid inter-node communicator
        auto& [eventID, entry]{data};
        if (mpiEnv.LocalNodeIdx() == rootNodeIdx) {
            data = buildData();
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

    // We will allocate a single window for both eventID and entry.
    // Layout of shared memory: [eventID (aligned)][padding (if necessary)][entry (aligned)]
    const auto eventIDSize{nEvent};
    const auto eventIDSizeByte{eventIDSize * sizeof(T)};
    const auto entrySize{nEvent + 1};
    const auto entrySizeByte{entrySize * sizeof(EntryType)};
    const auto entryWinSizeByte{entrySizeByte + alignof(EntryType)}; // Reserve space for alignment
    const auto totalWinSizeByte{gsl::narrow<MPI_Aint>(eventIDSizeByte + entryWinSizeByte)};
    // Set some window info
    mplr::info winInfo;
    winInfo.set("no_locks", "true");
    winInfo.set("same_disp_unit", "true");
    winInfo.set("alloc_shared_noncontig", "true");
    winInfo.set("mpi_minimum_memory_alignment", std::to_string(alignof(T)));
    // Create shared-memory window.
    // Node leaders allocate shared memory for data; others allocate zero-size window.
    std::byte* shm;
    auto& shmWin{get<MPI_Win>(fShmWinOrData)};
    if (interNodeComm.is_valid()) {
        MPI_Win_allocate_shared(totalWinSizeByte, 1, winInfo.native_handle(), intraNodeComm.native_handle(), &shm, &shmWin);
    } else {
        MPI_Win_allocate_shared(0, 1, winInfo.native_handle(), intraNodeComm.native_handle(), &shm, &shmWin);
        MPI_Aint winSizeByte;
        int winDispUnit;
        MPI_Win_shared_query(shmWin, 0, &winSizeByte, &winDispUnit, &shm);
        Ensures(winSizeByte >= totalWinSizeByte);
        Ensures(winDispUnit == 1);
    }

    // Place eventID in shared memory; content will be filled by node leaders later
    // Memory are aligned to T so no extra padding is needed before eventID
    fEventID = std::span(new (shm) T[eventIDSize], eventIDSize);
    // Place entry in shared memory; content will be filled by node leaders later
    // Align shared memory to EntryType or throw if alignment fails
    void* shmEntry{shm + eventIDSizeByte};
    auto shmEntrySpace{entryWinSizeByte};
    if (std::align(alignof(EntryType), entrySizeByte, shmEntry, shmEntrySpace) == nullptr) {
        Throw<std::runtime_error>("Failed to align shared memory");
    }
    fEntry = std::span(new (shmEntry) EntryType[entrySize], entrySize);

    // Copy entry data to shared-memory window on node leaders;
    // others will see the data after MPI_Win_sync
    if (interNodeComm.is_valid()) {
        std::memcpy(shm, data.eventID.data(), eventIDSizeByte);
        std::memcpy(shmEntry, data.entry.data(), entrySizeByte);
        MPI_Win_sync(shmWin);
        intraNodeComm.barrier();
    } else {
        intraNodeComm.barrier();
        MPI_Win_sync(shmWin);
    }
    // DO NOT modify data hereafter!
}

template<std::integral T, std::signed_integral U>
SingleRDFEventInfo<T, U>::~SingleRDFEventInfo() {
    const auto shmWin{get_if<MPI_Win>(&fShmWinOrData)};
    if (shmWin and *shmWin != MPI_WIN_NULL) {
        MPI_Win_free(shmWin);
    }
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
    fPerRDFEventInfo{},
    fShmWinOrData{MPI_WIN_NULL} {
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
        // Build event index data
        const auto& data{fShmWinOrData.template emplace<Data>(BuildData())};
        fToLocalEvtIdx = std::span(data.toLocalEvtIdx);
        for (gsl::index k{}; k < nRDF; ++k) {
            fToGlobEvtIdx[k] = std::span(data.toGlobEvtIdx[k]);
        }
        fMinLocalEvtIdxAfter = std::span(data.minLocalEvtIdxAfter);
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
    Data data;
    std::size_t nEvent;
    if (interNodeComm.is_valid()) { // Only node leaders have valid inter-node communicator
        data = BuildData();
        nEvent = data.toLocalEvtIdx.size();
    }
    // Broadcast nEvent from node leaders to intra-node peers;
    // non-leaders lazy-spin here to reduce resources consumption
    intraNodeComm.ibcast(0, nEvent)
        .wait(mplr::duty_ratio::preset::moderate);

    // We will allocate a single window for toLocalEvtIdx, toGlobEvtIdx and minLocalEvtIdxAfter.
    // Layout of shared memory: [toLocalEvtIdx][toGlobEvtIdx for RDF 0]...[toGlobEvtIdx for RDF N-1][minLocalEvtIdxAfter]
    const auto toLocalEvtIdxSize{nEvent};
    const auto toLocalEvtIdxSizeByte{toLocalEvtIdxSize * sizeof(std::array<U, N>)};
    std::array<std::size_t, N> toGlobEvtIdxSize;
    std::ranges::transform(fPerRDFEventInfo, toGlobEvtIdxSize.begin(), [](auto&& rdf) { return rdf->NEvent(); });
    std::array<std::size_t, N> toGlobEvtIdxSizeByte;
    std::ranges::transform(toGlobEvtIdxSize, toGlobEvtIdxSizeByte.begin(), [](auto size) { return size * sizeof(U); });
    const auto minLocalEvtIdxAfterSize{nEvent};
    const auto minLocalEvtIdxAfterSizeByte{minLocalEvtIdxAfterSize * sizeof(std::array<U, N>)};
    const auto totalWinSizeByte{gsl::narrow<MPI_Aint>(toLocalEvtIdxSizeByte + muc::ranges::reduce(toGlobEvtIdxSizeByte) + minLocalEvtIdxAfterSizeByte)};
    // Set some window info
    mplr::info winInfo;
    winInfo.set("no_locks", "true");
    winInfo.set("same_disp_unit", "true");
    winInfo.set("alloc_shared_noncontig", "true");
    winInfo.set("mpi_minimum_memory_alignment", std::to_string(alignof(U)));
    // Create shared-memory window.
    // Node leaders allocate shared memory for data; others allocate zero-size window.
    std::byte* shm;
    auto& shmWin{get<MPI_Win>(fShmWinOrData)};
    if (interNodeComm.is_valid()) {
        MPI_Win_allocate_shared(totalWinSizeByte, sizeof(U), winInfo.native_handle(), intraNodeComm.native_handle(), &shm, &shmWin);
    } else {
        MPI_Win_allocate_shared(0, sizeof(U), winInfo.native_handle(), intraNodeComm.native_handle(), &shm, &shmWin);
        MPI_Aint winSizeByte;
        int winDispUnit;
        MPI_Win_shared_query(shmWin, 0, &winSizeByte, &winDispUnit, &shm);
        Ensures(winSizeByte >= totalWinSizeByte);
        Ensures(winDispUnit == sizeof(U));
    }

    std::byte* shmPtr{shm};
    // Place toLocalEvtIdx in shared memory; content will be filled by node leaders later
    fToLocalEvtIdx = std::span(new (shmPtr) std::array<U, N>[toLocalEvtIdxSize], toLocalEvtIdxSize);
    // Place toGlobEvtIdx in shared memory; content will be filled by node leaders later
    shmPtr += toLocalEvtIdxSizeByte;
    for (gsl::index k{}; k < nRDF; ++k) {
        const auto size{toGlobEvtIdxSize[k]};
        fToGlobEvtIdx[k] = std::span(new (shmPtr) U[size], size);
        shmPtr += toGlobEvtIdxSizeByte[k];
    }
    // Place minLocalEvtIdxAfter in shared memory; content will be filled by node leaders later
    fMinLocalEvtIdxAfter = std::span(new (shmPtr) std::array<U, N>[minLocalEvtIdxAfterSize], minLocalEvtIdxAfterSize);

    // Copy data to shared-memory window on node leaders;
    // others will see the data after MPI_Win_sync
    if (interNodeComm.is_valid()) {
        shmPtr = shm;
        std::memcpy(shmPtr, data.toLocalEvtIdx.data(), toLocalEvtIdxSizeByte);
        shmPtr += toLocalEvtIdxSizeByte;
        for (gsl::index k{}; k < nRDF; ++k) {
            const auto sizeByte{toGlobEvtIdxSizeByte[k]};
            std::memcpy(shmPtr, data.toGlobEvtIdx[k].data(), sizeByte);
            shmPtr += sizeByte;
        }
        std::memcpy(shmPtr, data.minLocalEvtIdxAfter.data(), minLocalEvtIdxAfterSizeByte);
        MPI_Win_sync(shmWin);
        intraNodeComm.barrier();
    } else {
        intraNodeComm.barrier();
        MPI_Win_sync(shmWin);
    }
    // DO NOT modify data hereafter!
}

template<std::integral T, std::size_t N, std::signed_integral U>
    requires(N >= 2)
MultiRDFEventInfo<T, N, U>::~MultiRDFEventInfo() {
    const auto shmWin{get_if<MPI_Win>(&fShmWinOrData)};
    if (shmWin and *shmWin != MPI_WIN_NULL) {
        MPI_Win_free(shmWin);
    }
}

template<std::integral T, std::size_t N, std::signed_integral U>
    requires(N >= 2)
auto MultiRDFEventInfo<T, N, U>::BuildData() const -> Data {
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
    Data data;
    auto& [toLocalEvtIdx, toGlobEvtIdx, minLocalEvtIdxAfter]{data};

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

    return data;
}

} // namespace Mustard::Data::inline Processing
