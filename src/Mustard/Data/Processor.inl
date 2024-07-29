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
Processor<AExecutor>::Processor(AExecutor executor, typename AExecutor::Index batchSize) :
    fExecutor{std::move(executor)},
    fBatchSize{batchSize} {
    fExecutor.ExecutionName("Event loop");
    fExecutor.TaskName("Batch");
}

template<muc::instantiated_from<MPIX::Executor> AExecutor>
template<TupleModelizable... Ts>
auto Processor<AExecutor>::Process(ROOTX::RDataFrame auto&& rdf, std::string_view eventIDBranchName,
                                   std::invocable<std::vector<std::shared_ptr<Tuple<Ts...>>>&> auto&& F) -> typename AExecutor::Index {
    return Process<Ts...>(std::forward<decltype(rdf)>(rdf),
                          RDFEventSplitPoint(std::forward<decltype(rdf)>(rdf), eventIDBranchName),
                          std::forward<decltype(F)>(F));
}

template<muc::instantiated_from<MPIX::Executor> AExecutor>
template<TupleModelizable... Ts>
auto Processor<AExecutor>::Process(ROOTX::RDataFrame auto&& rdf, const std::vector<unsigned>& eventSplitPoint,
                                   std::invocable<std::vector<std::shared_ptr<Tuple<Ts...>>>&> auto&& F) -> typename AExecutor::Index {
    const auto& esp{eventSplitPoint};
    const auto nEvent{static_cast<typename AExecutor::Index>(esp.size() - 1)};
    const auto nBatch{nEvent / fBatchSize + 1};

    typename AExecutor::Index nEventProcessed{};
    fExecutor.Execute(
        nBatch,
        [&](auto k) {
            const auto iFirst{k * fBatchSize};
            if (iFirst >= nEvent) { return; }
            const auto iLast{std::min(iFirst + fBatchSize, nEvent)};
            const auto data{Take<Ts...>::From(rdf.Range(esp[iFirst], esp[iLast]))};

            std::vector<std::shared_ptr<Tuple<Ts...>>> event;
            for (auto i{iFirst}; i < iLast; ++i) {
                const std::ranges::subrange eventData{data.cbegin() + (esp[i] - esp[iFirst]),
                                                      data.cbegin() + (esp[i + 1] - esp[iFirst])};
                event.clear();
                event.resize(eventData.size());
                std::ranges::copy(eventData, event.begin());
                F(event);
            }
            nEventProcessed += iLast - iFirst;
        });
    return nEventProcessed;
}

} // namespace Mustard::Data
