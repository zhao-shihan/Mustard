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
        return 0;
    }

    AsyncEntryReader<Ts...> asyncReader{std::move(rdf)};
    return ProcessImpl(asyncReader, nEntry, "entries", std::forward<decltype(F)>(F));
}

template<muc::instantiated_from<MPIX::Executor> AExecutor>
template<TupleModelizable... Ts, std::integral AEventIDType>
auto Processor<AExecutor>::Process(ROOT::RDF::RNode rdf, AEventIDType, std::string eventIDColumnName,
                                   std::invocable<bool, muc::shared_ptrvec<Tuple<Ts...>>> auto&& F) -> Index {
    auto es{RDFEventSplit<AEventIDType>(rdf, std::move(eventIDColumnName))};
    return Process<Ts...>(std::move(rdf), AEventIDType{}, std::move(es), std::forward<decltype(F)>(F));
}

template<muc::instantiated_from<MPIX::Executor> AExecutor>
template<TupleModelizable... Ts, std::integral AEventIDType>
auto Processor<AExecutor>::Process(ROOT::RDF::RNode rdf, AEventIDType, std::vector<gsl::index> eventSplit,
                                   std::invocable<bool, muc::shared_ptrvec<Tuple<Ts...>>> auto&& F) -> Index {
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
    return ProcessImpl(asyncReader, nEvent, "events", std::forward<decltype(F)>(F));
}

template<muc::instantiated_from<MPIX::Executor> AExecutor>
template<typename AData>
auto Processor<AExecutor>::ProcessImpl(AsyncReader<AData>& asyncReader, Index n, std::string_view what,
                                       std::invocable<bool, typename AData::value_type> auto&& F) -> Index {
    AData batchData;

    Index nProcessed{};
    const auto ProcessBatch{[&] {
        for (auto&& data : batchData) {
            std::invoke(std::forward<decltype(F)>(F), /*byPass =*/false, std::move(data));
        }
        nProcessed += batchData.size();
    }};
    std::future<void> asyncProcess;

    const auto byPassWillOccur{ByPassOccurrenceCheck(n, what)};
    const auto& worldComm{mpl::environment::comm_world()};
    const auto batch{this->CalculateBatchConfiguration(worldComm.size(), n)};
    fExecutor.Execute(
        std::max(static_cast<Index>(worldComm.size()), batch.nBatch),
        [&](auto k) { // k is batch index
            if (byPassWillOccur) [[unlikely]] {
                if (k >= n) { // by pass when there are too many processors
                    std::invoke(std::forward<decltype(F)>(F), /*byPass =*/true, typename AData::value_type{});
                    return;
                }
            }
            const auto [iFirst, iLast]{this->CalculateIndexRange(k, batch)};
            if (asyncReader.Reading()) { batchData = asyncReader.Acquire(); }
            asyncReader.Read(iFirst, iLast);
            if (asyncProcess.valid()) { asyncProcess.get(); }
            asyncProcess = std::async(std::launch::deferred, ProcessBatch);
        });
    batchData = asyncReader.Acquire();
    asyncProcess.get();
    if (not asyncReader.Exhausted()) { asyncReader.Exhaust(); }

    return nProcessed;
}

template<muc::instantiated_from<MPIX::Executor> AExecutor>
auto Processor<AExecutor>::ByPassOccurrenceCheck(Index n, std::string_view what) -> bool {
    const auto& worldComm{mpl::environment::comm_world()};
    const auto byPassWillOccur{static_cast<Index>(worldComm.size()) > n};
    if (worldComm.rank() == 0 and byPassWillOccur) [[unlikely]] {
        PrintWarning(fmt::format("#processors ({}) are more than #{} ({})",
                                 worldComm.size(), what, n));
    }
    return byPassWillOccur;
}

} // namespace Mustard::Data
