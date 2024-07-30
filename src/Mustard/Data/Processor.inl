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
auto Processor<AExecutor>::Process(ROOTX::RDataFrame auto&& rdf,
                                   std::invocable<bool, std::shared_ptr<Tuple<Ts...>>&> auto&& F) -> Index {
    const auto nEntry{static_cast<Index>(*rdf.Count())};
    const auto nBatch{std::max(static_cast<Index>(1), nEntry / this->fBatchSizeProposal)};
    const auto nEPBQuot{nEntry / nBatch};
    const auto nEPBRem{nEntry % nBatch};

    const auto byPass{ByPassCheck(nBatch)};

    Index nEntryProcessed{};
    fExecutor.Execute( // below: allow to by pass when there are too many processors
        std::max(static_cast<Index>(Env::MPIEnv::Instance().CommWorldSize()), nBatch),
        [&](auto k) { // k is batch index
            if (byPass) {
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
auto Processor<AExecutor>::Process(ROOTX::RDataFrame auto&& rdf, std::string_view eventIDBranchName,
                                   std::invocable<bool, std::vector<std::shared_ptr<Tuple<Ts...>>>&> auto&& F) -> Index {
    return Process<Ts...>(std::forward<decltype(rdf)>(rdf),
                          RDFEventSplitPoint(std::forward<decltype(rdf)>(rdf), eventIDBranchName),
                          std::forward<decltype(F)>(F));
}

template<muc::instantiated_from<MPIX::Executor> AExecutor>
template<TupleModelizable... Ts>
auto Processor<AExecutor>::Process(ROOTX::RDataFrame auto&& rdf, const std::vector<unsigned>& eventSplitPoint,
                                   std::invocable<bool, std::vector<std::shared_ptr<Tuple<Ts...>>>&> auto&& F) -> Index {
    const auto& esp{eventSplitPoint};

    const auto nEntry{static_cast<Index>(esp.back() - esp.front())};
    const auto nEvent{static_cast<Index>(esp.size() - 1)};
    const auto nBatch{std::clamp(nEntry / this->fBatchSizeProposal, static_cast<Index>(1), nEvent)};
    const auto nEPBQuot{nEvent / nBatch};
    const auto nEPBRem{nEvent % nBatch};

    const auto byPass{ByPassCheck(nBatch)};

    Index nEventProcessed{};
    fExecutor.Execute( // below: allow to by pass when there are too many processors
        std::max(static_cast<Index>(Env::MPIEnv::Instance().CommWorldSize()), nBatch),
        [&](auto k) { // k is batch index
            using Event = std::vector<std::shared_ptr<Tuple<Ts...>>>;
            if (byPass) {
                std::invoke(std::forward<decltype(F)>(F), /*byPass =*/true, Event{});
                return;
            }

            const auto [iFirst, iLast]{this->CalculateIndexRange(k, nEPBQuot, nEPBRem)}; // event index
            const auto data{Take<Ts...>::From(rdf.Range(esp[iFirst], esp[iLast]))};

            Event event;
            for (auto i{iFirst}; i < iLast; ++i) {
                const std::ranges::subrange eventData{data.cbegin() + (esp[i] - esp[iFirst]),
                                                      data.cbegin() + (esp[i + 1] - esp[iFirst])};
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
auto Processor<AExecutor>::ByPassCheck(Index nBatch) -> bool {
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    if (mpiEnv.OnCommWorldMaster() and mpiEnv.CommWorldSize() > nBatch) {
        Env::PrintLnWarning("Warning from Mustard::Data::Processor: #Processors ({}) are more than #batches ({})",
                            mpiEnv.CommWorldSize(), nBatch);
    }
    return mpiEnv.CommWorldRank() >= nBatch;
}

} // namespace Mustard::Data
