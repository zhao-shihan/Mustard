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

namespace Mustard::Data {

namespace internal {

template<std::integral T>
auto MakeFlatRDFEventSplitPoint(ROOT::RDF::RNode rdf,
                                std::string eventIDColumnName) -> std::pair<std::vector<T>, std::vector<gsl::index>> {
    std::vector<T> eventIDList;
    std::vector<gsl::index> eventSplit;

    gsl::index index{};
    muc::flat_hash_set<T> eventIDSet;
    rdf.Foreach(
        [&](T eventID) {
            if (eventIDList.empty() or eventID != eventIDList.back()) {
                const auto [_, uniqueEventID]{eventIDSet.emplace(eventID)};
                if (not uniqueEventID) [[unlikely]] {
                    PrintError(fmt::format("There are more than one event {})", eventID));
                }
                eventIDList.emplace_back(eventID);
                eventSplit.emplace_back(index);
            }
            ++index;
        },
        {std::move(eventIDColumnName)});
    eventSplit.emplace_back(index);

    return {std::move(eventIDList), std::move(eventSplit)};
}

} // namespace internal

template<std::integral T>
auto RDFEventSplit(ROOT::RDF::RNode rdf,
                   std::string eventIDColumnName) -> std::vector<gsl::index> {
    auto eventSplit{not Env::MPIEnv::Available() or Env::MPIEnv::Instance().OnCommWorldMaster() ?
                        internal::MakeFlatRDFEventSplitPoint<T>(std::move(rdf), std::move(eventIDColumnName)).second :
                        std::vector<gsl::index>{}};
    if (Env::MPIEnv::Available()) {
        auto eventSplitSize{gsl::narrow<int>(eventSplit.size())};
        MPI_Bcast(&eventSplitSize, 1, MPIX::DataType(eventSplitSize), 0, MPI_COMM_WORLD);
        eventSplit.resize(eventSplitSize);
        MPI_Bcast(eventSplit.data(), eventSplitSize, MPIX::DataType(eventSplit.data()), 0, MPI_COMM_WORLD);
    }
    return eventSplit;
}

template<std::integral T, std::size_t N>
auto RDFEventSplit(std::array<ROOT::RDF::RNode, N> rdf,
                   const std::string& eventIDColumnName) -> std::vector<std::array<RDFEntryRange, N>> {
    std::array<std::string, N> columnName;
    columnName.fill(eventIDColumnName);
    return RDFEventSplit(rdf, columnName);
}

template<std::integral T, std::size_t N>
auto RDFEventSplit(std::array<ROOT::RDF::RNode, N> rdf,
                   const std::array<std::string, N>& eventIDColumnName) -> std::vector<std::array<RDFEntryRange, N>> {
    constexpr auto nRDF{static_cast<gsl::index>(N)};
    std::array<std::pair<std::vector<T>, std::vector<gsl::index>>, N> flatES;
    // Build all RDF event split
    if (Env::MPIEnv::Available()) { // Parallel
        const auto& mpiEnv{Env::MPIEnv::Instance()};
        const auto nProcess{mpiEnv.CommWorldSize()};
        for (gsl::index i{}; i < nRDF; ++i) {
            if (mpiEnv.CommWorldRank() == i % nProcess) {
                flatES[i] = internal::MakeFlatRDFEventSplitPoint<T>(
                    std::move(rdf[i]), eventIDColumnName[i]);
            }
        }
        // Broadcast to all processes
        for (gsl::index i{}; i < nRDF; ++i) {
            auto& [eventID, es]{flatES[i]};
            auto esSize{gsl::narrow<int>(es.size())};
            MPI_Bcast(&esSize, 1, MPIX::DataType(esSize), i % nProcess, MPI_COMM_WORLD);
            eventID.resize(esSize - 1);
            es.resize(esSize);
            MPI_Bcast(&eventID, esSize - 1, MPIX::DataType<T>(), i % nProcess, MPI_COMM_WORLD);
            MPI_Bcast(&es, esSize, MPIX::DataType<gsl::index>(), i % nProcess, MPI_COMM_WORLD);
        }
    } else { // Sequential
        for (gsl::index i{}; i < nRDF; ++i) {
            flatES[i] = internal::MakeFlatRDFEventSplitPoint<T>(
                std::move(rdf[i]), std::move(eventIDColumnName[i]));
        }
    }

    std::array<muc::flat_hash_map<T, RDFEntryRange>, N> eventMap;
    for (gsl::index iRDF{}; iRDF < nRDF; ++iRDF) {
        const auto& [eventID, es]{flatES[iRDF]};
        const auto nEvent{ssize(eventID)};
        eventMap[iRDF].reserve(nEvent);
        for (gsl::index i{}; i < nEvent; ++i) {
            eventMap[iRDF].try_emplace(eventID[i], RDFEntryRange{es[i], es[i + 1]});
        }
    }
    flatES = {};

    std::vector<std::array<RDFEntryRange, N>> result;
    result.reserve(std::ranges::max_element(eventMap, {}, [](auto&& m) { return m.size(); })->size());
    for (gsl::index i{}; i < nRDF; ++i) {
        for (auto&& [eventID, thisEntryRange] : std::as_const(eventMap[i])) {
            auto& entryRange{result.emplace_back()};
            for (gsl::index j{}; j < nRDF; ++j) {
                if (i == j) {
                    entryRange[i] = thisEntryRange;
                    continue;
                }
                if (const auto iEntryRange{eventMap[j].find(eventID)};
                    iEntryRange != eventMap[j].cend()) {
                    entryRange[j] = iEntryRange->second;
                    eventMap[j].erase(iEntryRange);
                }
            }
        }
        eventMap[i].clear();
    }

    muc::timsort(result,
                 [](auto&& lhs, auto&& rhs) {
                     for (gsl::index i{}; i < nRDF; ++i) {
                         if (lhs[i].first == rhs[i].first or
                             lhs[i].last == 0 or
                             rhs[i].last == 0) { continue; }
                         return lhs[i].first < rhs[i].first;
                     }
                     return false;
                 });
    /* for (auto&& rr : result) {
        for (auto&& r : rr) {
            fmt::print("({} {})", r.first, r.last);
        }
        fmt::print("\n");
    } */

    return result;
}

} // namespace Mustard::Data
