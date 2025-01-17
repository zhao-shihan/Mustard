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

template<muc::instantiated_from<MPIX::Executor> AExecutor>
Processor<AExecutor>::Processor(AExecutor executor) :
    Base{},
    fExecutor{std::move(executor)} {
    fExecutor.ExecutionName("Event loop");
    fExecutor.TaskName("Batch");
}

template<muc::instantiated_from<MPIX::Executor> AExecutor>
template<TupleModelizable... Ts>
auto Processor<AExecutor>::Process(ROOT::RDF::RNode rdf,
                                   std::invocable<bool, std::shared_ptr<Tuple<Ts...>>> auto&& F) -> Index {
    const auto nEntry{gsl::narrow<Index>(*rdf.Count())};
    if (nEntry == 0) {
        PrintWarning("Empty dataset");
        return 0;
    }

    Index nEntryProcessed{};
    const auto ProcessBatch{[&](muc::shared_ptrvec<Tuple<Ts...>> data) {
        for (auto&& entry : data) {
            std::invoke(std::forward<decltype(F)>(F), /*byPass =*/false, std::move(entry));
        }
        nEntryProcessed += data.size();
    }};
    std::future<void> async;

    const auto byPassWillOccur{ByPassOccurrenceCheck(nEntry, "entries")};
    const auto batch{this->CalculateBatchConfiguration(Env::MPIEnv::Instance().CommWorldSize(), nEntry)};
    fExecutor.Execute(
        batch.nBatch,
        [&](auto k) {                                           // k is batch index
            if (byPassWillOccur and k >= nEntry) [[unlikely]] { // by pass when there are too many processors
                std::invoke(std::forward<decltype(F)>(F), /*byPass =*/true, std::shared_ptr<Tuple<Ts...>>{});
                return;
            }
            // load data
            const auto [iFirst, iLast]{this->CalculateIndexRange(k, batch)}; // entry index
            auto data{Take<Ts...>::From(rdf.Range(iFirst, iLast))};
            // async process
            if (async.valid()) { async.wait(); }
            async = std::async(this->fAsyncPolicy, ProcessBatch, std::move(data));
        });
    return nEntryProcessed;
}

template<muc::instantiated_from<MPIX::Executor> AExecutor>
template<TupleModelizable... Ts>
auto Processor<AExecutor>::Process(ROOT::RDF::RNode rdf, std::string eventIDBranchName,
                                   std::invocable<bool, muc::shared_ptrvec<Tuple<Ts...>>> auto&& F) -> Index {
    return Process<Ts...>(rdf, RDFEventSplit(rdf, std::move(eventIDBranchName)), std::forward<decltype(F)>(F));
}

template<muc::instantiated_from<MPIX::Executor> AExecutor>
template<TupleModelizable... Ts>
auto Processor<AExecutor>::Process(ROOT::RDF::RNode rdf, const std::vector<gsl::index>& eventSplit,
                                   std::invocable<bool, muc::shared_ptrvec<Tuple<Ts...>>> auto&& F) -> Index {
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

    Index nEventProcessed{};
    using Event = muc::shared_ptrvec<Tuple<Ts...>>;
    const auto ProcessBatch{[&](Index iFirst, Index iLast, muc::shared_ptrvec<Tuple<Ts...>> data) {
        for (auto i{iFirst}; i < iLast; ++i) {
            std::ranges::subrange eventData{data.begin() + (es[i] - es[iFirst]),
                                            data.begin() + (es[i + 1] - es[iFirst])};
            Event event(eventData.size());
            std::ranges::move(eventData, event.begin());
            std::invoke(std::forward<decltype(F)>(F), /*byPass =*/false, std::move(event));
        }
        nEventProcessed += iLast - iFirst;
    }};
    std::future<void> async;

    const auto byPassWillOccur{ByPassOccurrenceCheck(nEvent, "events")};
    const auto batch{this->CalculateBatchConfiguration(Env::MPIEnv::Instance().CommWorldSize(), nEvent)};
    fExecutor.Execute(
        batch.nBatch,
        [&](auto k) {                                           // k is batch index
            if (byPassWillOccur and k >= nEvent) [[unlikely]] { // by pass when there are too many processors
                std::invoke(std::forward<decltype(F)>(F), /*byPass =*/true, Event{});
                return;
            }
            // load data
            const auto [iFirst, iLast]{this->CalculateIndexRange(k, batch)}; // event index
            auto data{Take<Ts...>::From(rdf.Range(es[iFirst], es[iLast]))};
            // async process
            if (async.valid()) { async.wait(); }
            async = std::async(this->fAsyncPolicy, ProcessBatch, iFirst, iLast, std::move(data));
        });
    return nEventProcessed;
}

template<muc::instantiated_from<MPIX::Executor> AExecutor>
auto Processor<AExecutor>::ByPassOccurrenceCheck(Index n, std::string_view what) -> bool {
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    const auto byPassWillOccur{static_cast<Index>(mpiEnv.CommWorldSize()) > n};
    if (mpiEnv.OnCommWorldMaster() and byPassWillOccur) [[unlikely]] {
        PrintWarning(fmt::format("#processors ({}) are more than #{} ({})",
                                 mpiEnv.CommWorldSize(), what, n));
    }
    return byPassWillOccur;
}

} // namespace Mustard::Data
