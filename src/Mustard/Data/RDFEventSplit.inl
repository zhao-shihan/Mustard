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
    const auto& commWorld{mpl::environment::comm_world()};
    auto eventSplit{commWorld.rank() == 0 ?
                        internal::MakeFlatRDFEventSplitPoint<T>(std::move(rdf), std::move(eventIDColumnName)).second :
                        std::vector<gsl::index>{}};
    auto eventSplitSize{eventSplit.size()};
    commWorld.bcast(0, eventSplitSize);
    eventSplit.resize(eventSplitSize);
    commWorld.bcast(0, eventSplit.data(), mpl::vector_layout<gsl::index>{eventSplit.size()});
    return eventSplit;
}

template<std::integral T, std::size_t N>
auto RDFEventSplit(std::array<ROOT::RDF::RNode, N> rdf,
                   const std::string& eventIDColumnName) -> std::vector<std::array<RDFEntryRange, N>> {
    std::array<std::string, N> columnName;
    columnName.fill(eventIDColumnName);
    return RDFEventSplit<T>(rdf, columnName);
}

template<std::integral T, std::size_t N>
auto RDFEventSplit(std::array<ROOT::RDF::RNode, N> rdf,
                   const std::array<std::string, N>& eventIDColumnName) -> std::vector<std::array<RDFEntryRange, N>> {
    constexpr auto nRDF{static_cast<gsl::index>(N)};
    std::array<std::pair<std::vector<T>, std::vector<gsl::index>>, N> flatES;
    // Build all RDF event split
    const auto& commWorld{mpl::environment::comm_world()};
    for (gsl::index i{}; i < nRDF; ++i) {
        if (commWorld.rank() == i % commWorld.size()) {
            flatES[i] = internal::MakeFlatRDFEventSplitPoint<T>(
                std::move(rdf[i]), eventIDColumnName[i]);
        }
    }
    // Broadcast to all processes
    mpl::irequest_pool bcastFlatES;
    for (gsl::index i{}; i < nRDF; ++i) {
        auto& [eventID, es]{flatES[i]};
        const auto rootRank{i % commWorld.size()};
        auto esSize{es.size()};
        commWorld.bcast(rootRank, esSize);
        eventID.resize(esSize - 1);
        es.resize(esSize);
        bcastFlatES.push(commWorld.ibcast(rootRank, eventID.data(), mpl::vector_layout<T>{eventID.size()}));
        bcastFlatES.push(commWorld.ibcast(rootRank, es.data(), mpl::vector_layout<gsl::index>{es.size()}));
    }
    bcastFlatES.waitall();

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
