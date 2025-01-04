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

template<TupleModelizable... Ts>
auto SeqProcessor::Process(ROOT::RDF::RNode rdf,
                           std::invocable<std::shared_ptr<Tuple<Ts...>>&> auto&& F) -> Index {
    const auto nEntry{gsl::narrow<Index>(*rdf.Count())};
    if (nEntry == 0) {
        PrintWarning("Empty dataset");
        return 0;
    }

    const auto nBatch{std::max(static_cast<Index>(1), nEntry / this->fBatchSizeProposal)};
    const auto nEPBQuot{nEntry / nBatch};
    const auto nEPBRem{nEntry % nBatch};

    Index nEntryProcessed{};
    for (Index k{}; k < nBatch; ++k) {                                               // k is batch index
        const auto [iFirst, iLast]{this->CalculateIndexRange(k, nEPBQuot, nEPBRem)}; // entry index
        const auto data{Take<Ts...>::From(rdf.Range(iFirst, iLast))};

        for (auto&& entry : data) {
            std::invoke(std::forward<decltype(F)>(F), entry);
        }
        nEntryProcessed += data.size();
    }
    return nEntryProcessed;
}

template<TupleModelizable... Ts>
auto SeqProcessor::Process(ROOT::RDF::RNode rdf, std::string eventIDBranchName,
                           std::invocable<muc::shared_ptrvec<Tuple<Ts...>>&> auto&& F) -> Index {
    auto es{RDFEventSplit(rdf, std::move(eventIDBranchName))};
    return Process<Ts...>(std::move(rdf), std::move(es), std::forward<decltype(F)>(F));
}

template<TupleModelizable... Ts>
auto SeqProcessor::Process(ROOT::RDF::RNode rdf, const std::vector<Index>& eventSplit,
                           std::invocable<muc::shared_ptrvec<Tuple<Ts...>>&> auto&& F) -> Index {
    const auto& es{eventSplit};

    const auto nEvent{gsl::narrow<Index>(es.size() - 1)};
    if (nEvent == 0) {
        PrintWarning("Empty dataset");
        return 0;
    }
    const auto nEntry{es.back()};
    if (const auto nEntryRDF{*rdf.Count()};
        nEntry != nEntryRDF) {
        PrintError(fmt::format("Entries of provided event split ({}) is inconsistent with the dataset ({})",
                               nEntry, nEntryRDF));
        return 0;
    }
    if (nEntry == 0) {
        PrintWarning("Empty dataset");
        return 0;
    }

    const auto nBatch{std::clamp(nEntry / fBatchSizeProposal, static_cast<Index>(1), nEvent)};
    const auto nEPBQuot{nEvent / nBatch};
    const auto nEPBRem{nEvent % nBatch};

    Index nEventProcessed{};
    for (Index k{}; k < nBatch; ++k) {                                         // k is batch index
        const auto [iFirst, iLast]{CalculateIndexRange(k, nEPBQuot, nEPBRem)}; // event index
        const auto data{Take<Ts...>::From(rdf.Range(es[iFirst], es[iLast]))};

        muc::shared_ptrvec<Tuple<Ts...>> event;
        for (auto i{iFirst}; i < iLast; ++i) {
            const std::ranges::subrange eventData{data.cbegin() + (es[i] - es[iFirst]),
                                                  data.cbegin() + (es[i + 1] - es[iFirst])};
            event.clear();
            event.resize(eventData.size());
            std::ranges::copy(eventData, event.begin());
            std::invoke(std::forward<decltype(F)>(F), event);
        }
        nEventProcessed += iLast - iFirst;
    }
    return nEventProcessed;
}

template<muc::instantiated_from<TupleModel>... Ts>
auto SeqProcessor::Process(std::array<ROOT::RDF::RNode, sizeof...(Ts)> rdf,
                           std::string eventIDBranchName,
                           std::invocable<muc::shared_ptrvec<Tuple<Ts>>&...> auto&& F) -> Index {
    auto es{RDFEventSplit(rdf, std::move(eventIDBranchName))};
    return Process<Ts...>(std::move(rdf), std::move(es), std::forward<decltype(F)>(F));
}

template<muc::instantiated_from<TupleModel>... Ts>
auto SeqProcessor::Process(std::array<ROOT::RDF::RNode, sizeof...(Ts)> rdf,
                           std::vector<std::string> eventIDBranchName,
                           std::invocable<muc::shared_ptrvec<Tuple<Ts>>&...> auto&& F) -> Index {
    auto es{RDFEventSplit(rdf, std::move(eventIDBranchName))};
    return Process<Ts...>(std::move(rdf), std::move(es), std::forward<decltype(F)>(F));
}

template<muc::instantiated_from<TupleModel>... Ts>
auto SeqProcessor::Process(std::array<ROOT::RDF::RNode, sizeof...(Ts)> rdf,
                           const std::vector<std::array<RDFEntryRange, sizeof...(Ts)>>& eventSplit,
                           std::invocable<muc::shared_ptrvec<Tuple<Ts>>&...> auto&& F) -> Index {
    const auto& es{eventSplit};

    const auto nEvent{gsl::narrow<Index>(es.size())};
    if (nEvent == 0) {
        PrintWarning("Empty dataset");
        return 0;
    }
    constexpr auto nRDF{static_cast<gsl::index>(sizeof...(Ts))};
    std::array<Index, nRDF> nEntry;
    nEntry.fill(std::numeric_limits<Index>::lowest());
    for (gsl::index iRDF{}; iRDF < nRDF; ++iRDF) {
        for (auto&& s : es) {
            if (s[iRDF].last > nEntry[iRDF]) {
                nEntry[iRDF] = s[iRDF].last;
            }
        }
    }

    for (gsl::index i{}; i < nRDF; ++i) {
        if (const auto nEntryRDF{*rdf[i].Count()};
            nEntry[i] != nEntryRDF) {
            PrintError(fmt::format("Entries of provided event split {} ({}) is inconsistent with the dataset {} ({})",
                                   i, nEntry[i], i, nEntryRDF));
            return 0;
        }
    }
    if (muc::ranges::reduce(nEntry) == 0) {
        PrintWarning("All datasets are empty");
        return 0;
    }

    const auto nEntryMax{*std::ranges::max_element(nEntry)};
    const auto nBatch{std::clamp(nEntryMax / fBatchSizeProposal, static_cast<Index>(1), nEvent)};
    const auto nEPBQuot{nEvent / nBatch};
    const auto nEPBRem{nEvent % nBatch};

    Index nEventProcessed{};
    for (Index k{}; k < nBatch; ++k) {                                         // k is batch index
        const auto [iFirst, iLast]{CalculateIndexRange(k, nEPBQuot, nEPBRem)}; // event index
        // entry range for each dataframe in this batch
        std::array<RDFEntryRange, nRDF> takeRange;
        takeRange.fill({std::numeric_limits<Index>::max(), std::numeric_limits<Index>::lowest()});
        for (Index i{iFirst}; i < iLast; ++i) {
            for (gsl::index iRDF{}; iRDF < nRDF; ++iRDF) {
                auto& [first, last]{takeRange[iRDF]};
                if (es[iFirst][iRDF].first < first) {
                    first = es[iFirst][iRDF].first;
                }
                if (es[iLast][iRDF].last > last) {
                    last = es[iLast][iRDF].last;
                }
            }
        }
        // data taken according to the entry range of this batch
        std::tuple<muc::shared_ptrvec<Tuple<Ts>>...> data;
        [&]<gsl::index... Is>(gslx::index_sequence<Is...>) {
            (..., (get<Is>(data) = Take<std::tuple_element_t<Is, std::tuple<Ts...>>>::
                       From(rdf[Is].Range(takeRange[Is].first, takeRange[Is].last))));
        }(gslx::make_index_sequence<nRDF>());
        // the event data
        std::tuple<muc::shared_ptrvec<Tuple<Ts>>...> event;
        for (auto i{iFirst}; i < iLast; ++i) {
            [&]<gsl::index... Is>(gslx::index_sequence<Is...>) {
                (...,
                 [&]<gsl::index I>(std::integral_constant<gsl::index, I>) {
                     get<I>(event).clear();
                     const auto entryRange{es[i][I]};
                     if (entryRange.last == 0) { return; }
                     const std::ranges::subrange eventData{
                         get<I>(data).cbegin() + (entryRange.first - es[iFirst][I].first),
                         get<I>(data).cbegin() + (entryRange.last - es[iFirst][I].first)};
                     get<I>(event).resize(eventData.size());
                     std::ranges::copy(eventData, get<I>(event).begin());
                 }(std::integral_constant<gsl::index, Is>{}));
            }(gslx::make_index_sequence<nRDF>());
            std::apply(std::forward<decltype(F)>(F), event);
        }
        nEventProcessed += iLast - iFirst;
    }
    return nEventProcessed;
}

} // namespace Mustard::Data
