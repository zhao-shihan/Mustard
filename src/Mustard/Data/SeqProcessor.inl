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
                           std::invocable<std::shared_ptr<Tuple<Ts...>>> auto&& F) -> Index {
    const auto nEntry{gsl::narrow<Index>(*rdf.Count())};
    if (nEntry == 0) {
        return 0;
    }

    AsyncEntryReader<Ts...> asyncReader{std::move(rdf)};
    return ProcessImpl(asyncReader, nEntry, std::forward<decltype(F)>(F));
}

template<TupleModelizable... Ts, std::integral AEventIDType>
auto SeqProcessor::Process(ROOT::RDF::RNode rdf, muc::type_tag<AEventIDType>, std::string eventIDColumnName,
                           std::invocable<muc::shared_ptrvec<Tuple<Ts...>>> auto&& F) -> Index {
    auto es{RDFEventSplit<AEventIDType>(rdf, std::move(eventIDColumnName))};
    return Process<Ts...>(std::move(rdf), AEventIDType{}, std::move(es), std::forward<decltype(F)>(F));
}

template<TupleModelizable... Ts, std::integral AEventIDType>
auto SeqProcessor::Process(ROOT::RDF::RNode rdf, muc::type_tag<AEventIDType>, std::vector<gsl::index> eventSplit,
                           std::invocable<muc::shared_ptrvec<Tuple<Ts...>>> auto&& F) -> Index {
    Expects(std::ranges::is_sorted(eventSplit));

    if (eventSplit.empty()) {
        return 0;
    }
    const auto nEntry{eventSplit.back()};
    if (const auto nEntryRDF{gsl::narrow<gsl::index>(*rdf.Count())};
        nEntry != nEntryRDF) [[unlikely]] {
        PrintError(fmt::format("Entries of provided event split ({}) is inconsistent with the dataset ({})",
                               nEntry, nEntryRDF));
        return 0;
    }
    const auto nEvent{gsl::narrow<Index>(eventSplit.size() - 1)};

    AsyncEventReader<AEventIDType, TupleModel<Ts...>> asyncReader{std::move(rdf), std::move(eventSplit)};
    return ProcessImpl(asyncReader, nEvent, std::forward<decltype(F)>(F));
}

template<muc::instantiated_from<TupleModel>... Ts, std::integral AEventIDType>
auto SeqProcessor::Process(std::array<ROOT::RDF::RNode, sizeof...(Ts)> rdf,
                           muc::type_tag<AEventIDType>, std::string eventIDColumnName,
                           std::invocable<muc::shared_ptrvec<Tuple<Ts>>...> auto&& F) -> Index {
    auto es{RDFEventSplit<AEventIDType>(rdf, std::move(eventIDColumnName))};
    return Process<Ts...>(std::move(rdf), AEventIDType{}, std::move(es), std::forward<decltype(F)>(F));
}

template<muc::instantiated_from<TupleModel>... Ts, std::integral AEventIDType>
auto SeqProcessor::Process(std::array<ROOT::RDF::RNode, sizeof...(Ts)> rdf,
                           muc::type_tag<AEventIDType>, std::vector<std::string> eventIDColumnName,
                           std::invocable<muc::shared_ptrvec<Tuple<Ts>>...> auto&& F) -> Index {
    auto es{RDFEventSplit<AEventIDType>(rdf, std::move(eventIDColumnName))};
    return Process<Ts...>(std::move(rdf), AEventIDType{}, std::move(es), std::forward<decltype(F)>(F));
}

template<muc::instantiated_from<TupleModel>... Ts, std::integral AEventIDType>
auto SeqProcessor::Process(std::array<ROOT::RDF::RNode, sizeof...(Ts)> rdf,
                           muc::type_tag<AEventIDType>, const std::vector<std::array<RDFEntryRange, sizeof...(Ts)>>& eventSplit,
                           std::invocable<muc::shared_ptrvec<Tuple<Ts>>...> auto&& F) -> Index {
    const auto& es{eventSplit};

    const auto nEvent{gsl::narrow<Index>(es.size())};
    if (nEvent == 0) {
        return 0;
    }
    constexpr auto nRDF{static_cast<gsl::index>(sizeof...(Ts))};
    std::array<Index, nRDF> nEntry;
    nEntry.fill(std::numeric_limits<Index>::lowest());
    for (auto&& s : es) {
        for (gsl::index iRDF{}; iRDF < nRDF; ++iRDF) {
            nEntry[iRDF] = std::max(nEntry[iRDF], s[iRDF].last);
        }
    }
    for (gsl::index i{}; i < nRDF; ++i) {
        if (const auto nEntryRDF{gsl::narrow<Index>(*rdf[i].Count())};
            nEntry[i] != nEntryRDF) [[unlikely]] {
            PrintError(fmt::format("Entries of provided event split {} ({}) is inconsistent with the dataset {} ({})",
                                   i, nEntry[i], i, nEntryRDF));
            return 0;
        }
    }

    Index nEventProcessed{};
    const auto ProcessBatch{[&](Index iFirst, Index iLast, std::tuple<muc::shared_ptrvec<Tuple<Ts>>...> data) {
        for (auto i{iFirst}; i < iLast; ++i) {
            std::tuple<muc::shared_ptrvec<Tuple<Ts>>...> event;
            [&]<gsl::index... Is>(gslx::index_sequence<Is...>) {
                (...,
                 [&]<gsl::index I>(std::integral_constant<gsl::index, I>) {
                     const auto entryRange{es[i][I]};
                     if (entryRange.last == 0) {
                         return;
                     }
                     std::ranges::subrange eventData{
                         get<I>(data).begin() + (entryRange.first - es[iFirst][I].first),
                         get<I>(data).begin() + (entryRange.last - es[iFirst][I].first)};
                     get<I>(event).resize(eventData.size());
                     std::ranges::move(eventData, get<I>(event).begin());
                 }(std::integral_constant<gsl::index, Is>{}));
            }(gslx::make_index_sequence<nRDF>());
            std::apply(std::forward<decltype(F)>(F), std::move(event));
            ++nEventProcessed;
            IterationEndAction();
        }
    }};
    std::future<void> async;

    const auto batch{CalculateBatchConfiguration(1, nEvent)};
    LoopBeginAction(nEvent);
    for (Index k{}; k < batch.nBatch; ++k) {                       // k is batch index
        const auto [iFirst, iLast]{CalculateIndexRange(k, batch)}; // event index
        // entry range for each dataframe in this batch
        std::array<RDFEntryRange, nRDF> takeRange;
        takeRange.fill({std::numeric_limits<Index>::max(), std::numeric_limits<Index>::lowest()});
        for (Index i{iFirst}; i < iLast; ++i) {
            for (gsl::index iRDF{}; iRDF < nRDF; ++iRDF) {
                auto& [first, last]{takeRange[iRDF]};
                if (es[i][iRDF].first < first) {
                    first = es[i][iRDF].first;
                }
                if (es[i][iRDF].last > last) {
                    last = es[i][iRDF].last;
                }
            }
        }
        // data taken according to the entry range of this batch
        auto data{[&]<gsl::index... Is>(gslx::index_sequence<Is...>) {
            return std::tuple{Take<std::tuple_element_t<Is, std::tuple<Ts...>>>::
                                  From(rdf[Is].Range(takeRange[Is].first, takeRange[Is].last))...};
        }(gslx::make_index_sequence<nRDF>())};
        // async process
        if (async.valid()) {
            async.get();
        }
        async = std::async(ProcessBatch, iFirst, iLast, std::move(data));
    }
    async.get();
    LoopEndAction();
    return nEventProcessed;
}

template<typename AData>
auto SeqProcessor::ProcessImpl(AsyncReader<AData>& asyncReader, Index n,
                               std::invocable<typename AData::value_type> auto&& F) -> Index {
    AData batchData;

    Index nProcessed{};
    const auto ProcessBatch{[&] {
        for (auto&& data : batchData) {
            std::invoke(std::forward<decltype(F)>(F), std::move(data));
            ++nProcessed;
            IterationEndAction();
        }
    }};
    std::future<void> asyncProcess;

    const auto batch{CalculateBatchConfiguration(1, n)};
    LoopBeginAction(n);
    for (Index k{}; k < batch.nBatch; ++k) { // k is batch index
        const auto [iFirst, iLast]{CalculateIndexRange(k, batch)};
        if (asyncReader.Reading()) {
            batchData = asyncReader.Acquire();
        }
        asyncReader.Read(iFirst, iLast);
        if (asyncProcess.valid()) {
            asyncProcess.get();
        }
        asyncProcess = std::async(std::launch::deferred, ProcessBatch);
    }
    batchData = asyncReader.Acquire();
    asyncProcess.get();
    LoopEndAction();

    return nProcessed;
}

} // namespace Mustard::Data
