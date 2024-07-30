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
auto SeqProcessor::Process(ROOTX::RDataFrame auto&& rdf, std::string_view eventIDBranchName,
                           std::invocable<std::vector<std::shared_ptr<Tuple<Ts...>>>&> auto&& F) -> Index {
    return Process<Ts...>(std::forward<decltype(rdf)>(rdf),
                          RDFEventSplitPoint(std::forward<decltype(rdf)>(rdf), eventIDBranchName),
                          std::forward<decltype(F)>(F));
}

template<TupleModelizable... Ts>
auto SeqProcessor::Process(ROOTX::RDataFrame auto&& rdf,
             std::invocable<bool, std::shared_ptr<Tuple<Ts...>>&> auto&& F) -> Index {
    const auto nEntry{static_cast<Index>(*rdf.Count())};
    const auto nBatch{std::max(static_cast<Index>(1), nEntry / this->fBatchSizeProposal)};
    const auto nEPBQuot{nEntry / nBatch};
    const auto nEPBRem{nEntry % nBatch};

    Index nEntryProcessed{};
    for (Index k{}; k < nBatch; ++k) {                                               // k is batch index
        const auto [iFirst, iLast]{this->CalculateIndexRange(k, nEPBQuot, nEPBRem)}; // entry index
        const auto data{Take<Ts...>::From(rdf.Range(iFirst, iLast))};

        for (auto&& entry : data) {
            std::invoke(std::forward<decltype(F)>(F), /*byPass =*/false, entry);
        }
        nEntryProcessed += data.size();
    }
    return nEntryProcessed;
}

template<TupleModelizable... Ts>
auto SeqProcessor::Process(ROOTX::RDataFrame auto&& rdf, const std::vector<Index>& eventSplitPoint,
                           std::invocable<std::vector<std::shared_ptr<Tuple<Ts...>>>&> auto&& F) -> Index {
    const auto& esp{eventSplitPoint};

    const auto nEntry{static_cast<Index>(esp.back() - esp.front())};
    const auto nEvent{static_cast<Index>(esp.size() - 1)};
    const auto nBatch{std::clamp(nEntry / fBatchSizeProposal, static_cast<Index>(1), nEvent)};
    const auto nEPBQuot{nEvent / nBatch};
    const auto nEPBRem{nEvent % nBatch};

    Index nEventProcessed{};
    for (Index k{}; k < nBatch; ++k) {                                         // k is batch index
        const auto [iFirst, iLast]{CalculateIndexRange(k, nEPBQuot, nEPBRem)}; // event index
        const auto data{Take<Ts...>::From(rdf.Range(esp[iFirst], esp[iLast]))};

        std::vector<std::shared_ptr<Tuple<Ts...>>> event;
        for (auto i{iFirst}; i < iLast; ++i) {
            const std::ranges::subrange eventData{data.cbegin() + (esp[i] - esp[iFirst]),
                                                  data.cbegin() + (esp[i + 1] - esp[iFirst])};
            event.clear();
            event.resize(eventData.size());
            std::ranges::copy(eventData, event.begin());
            std::invoke(std::forward<decltype(F)>(F), event);
        }
        nEventProcessed += iLast - iFirst;
    }
    return nEventProcessed;
}

} // namespace Mustard::Data
