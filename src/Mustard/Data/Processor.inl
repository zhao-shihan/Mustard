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
Processor<AExecutor>::Processor(AExecutor executor, Index batchSizeProposal) :
    Base{batchSizeProposal},
    fExecutor{std::move(executor)} {
    fExecutor.ExecutionName("Event loop");
    fExecutor.TaskName("Batch");
}

template<muc::instantiated_from<MPIX::Executor> AExecutor>
template<TupleModelizable... Ts>
auto Processor<AExecutor>::Process(ROOT::RDF::RNode rdf,
                                   std::invocable<bool, std::shared_ptr<Tuple<Ts...>>&> auto&& F) -> Index {
    const auto nEntry{static_cast<Index>(*rdf.Count())};
    if (nEntry == 0) {
        PrintWarning("Empty dataset");
        return 0;
    }

    const auto nProc{static_cast<Index>(Env::MPIEnv::Instance().CommWorldSize())};
    const auto byPass{ByPassCheck(nEntry, "entries")};

    const auto nBatch{std::max(nProc, nEntry / this->fBatchSizeProposal)};
    const auto nEPBQuot{nEntry / nBatch};
    const auto nEPBRem{nEntry % nBatch};

    Index nEntryProcessed{};
    fExecutor.Execute(
        nBatch,
        [&](auto k) {                     // k is batch index
            if (byPass and k >= nEntry) { // by pass when there are too many processors
                std::invoke(std::forward<decltype(F)>(F), /*byPass =*/true, std::shared_ptr<Tuple<Ts...>>{});
                return;
            }

            const auto [iFirst, iLast]{this->CalculateIndexRange(k, nEPBQuot, nEPBRem)}; // entry index
            const auto data{Take<Ts...>::From(rdf.Range(iFirst, iLast))};

            for (auto&& entry : data) {
                std::invoke(std::forward<decltype(F)>(F), /*byPass =*/false, entry);
            }
            nEntryProcessed += data.size();
        });
    return nEntryProcessed;
}

template<muc::instantiated_from<MPIX::Executor> AExecutor>
template<TupleModelizable... Ts>
auto Processor<AExecutor>::Process(ROOT::RDF::RNode rdf, std::string eventIDBranchName,
                                   std::invocable<bool, std::vector<std::shared_ptr<Tuple<Ts...>>>&> auto&& F) -> Index {
    return Process<Ts...>(rdf, RDFEventSplit(rdf, std::move(eventIDBranchName)), std::forward<decltype(F)>(F));
}

template<muc::instantiated_from<MPIX::Executor> AExecutor>
template<TupleModelizable... Ts>
auto Processor<AExecutor>::Process(ROOT::RDF::RNode rdf, const std::vector<unsigned>& eventSplit,
                                   std::invocable<bool, std::vector<std::shared_ptr<Tuple<Ts...>>>&> auto&& F) -> Index {
    const auto& es{eventSplit};

    const auto nEvent{static_cast<Index>(es.size() - 1)};
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

    const auto nProc{static_cast<Index>(Env::MPIEnv::Instance().CommWorldSize())};
    const auto byPass{ByPassCheck(nEvent, "events")};

    const auto nBatch{std::max(nProc, nEntry / this->fBatchSizeProposal)};
    const auto nEPBQuot{nEvent / nBatch};
    const auto nEPBRem{nEvent % nBatch};

    Index nEventProcessed{};
    fExecutor.Execute(
        nBatch,
        [&](auto k) { // k is batch index
            using Event = std::vector<std::shared_ptr<Tuple<Ts...>>>;
            if (byPass and k >= nEvent) { // by pass when there are too many processors
                std::invoke(std::forward<decltype(F)>(F), /*byPass =*/true, Event{});
                return;
            }

            const auto [iFirst, iLast]{this->CalculateIndexRange(k, nEPBQuot, nEPBRem)}; // event index
            const auto data{Take<Ts...>::From(rdf.Range(es[iFirst], es[iLast]))};

            Event event;
            for (auto i{iFirst}; i < iLast; ++i) {
                const std::ranges::subrange eventData{data.cbegin() + (es[i] - es[iFirst]),
                                                      data.cbegin() + (es[i + 1] - es[iFirst])};
                event.clear();
                event.resize(eventData.size());
                std::ranges::copy(eventData, event.begin());
                std::invoke(std::forward<decltype(F)>(F), /*byPass =*/false, event);
            }
            nEventProcessed += iLast - iFirst;
        });
    return nEventProcessed;
}

template<muc::instantiated_from<MPIX::Executor> AExecutor>
auto Processor<AExecutor>::ByPassCheck(Index n, std::string_view what) -> bool {
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    const auto byPass{static_cast<Index>(mpiEnv.CommWorldSize()) > n};
    if (mpiEnv.OnCommWorldMaster() and byPass) {
        PrintWarning(fmt::format("#processors ({}) are more than #{} ({})",
                                 mpiEnv.CommWorldSize(), what, n));
    }
    return byPass;
}

} // namespace Mustard::Data
