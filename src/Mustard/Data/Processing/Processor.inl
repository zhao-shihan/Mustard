// -*- C++ -*-
//
// Copyright (C) 2020-2026  Shihan Zhao and contributors
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

namespace Mustard::Data::inline Processing {

template<muc::instantiated_from<Executor> AExecutor>
Processor<AExecutor>::Processor(AExecutor executor) :
    fExecutor{std::move(executor)},
    fBatchSizeProposal{100} {
    fExecutor.ExecutionName("Event loop");
    fExecutor.OperationName("Dispatch");
    fExecutor.TaskName("batch");
}

template<muc::instantiated_from<Executor> AExecutor>
template<Modelized M>
auto Processor<AExecutor>::Run(RDFEntryReader<M>& reader, std::invocable<bool, ArcTuple<M>> auto&& f) -> Index {
    const auto nEntry{gsl::narrow<Index>(reader.NEntry())};
    if (nEntry == 0) {
        return 0;
    }
    return RunImpl(reader, nEntry, "entries", std::forward<decltype(f)>(f));
}

template<muc::instantiated_from<Executor> AExecutor>
template<Modelized... Ms>
auto Processor<AExecutor>::Run(RDFEntryReader<Ms...>& reader, std::invocable<bool, ArcTuple<Ms>...> auto&& f) -> Index {
    const auto nEntry{gsl::narrow<Index>(reader.NEntry())};
    if (nEntry == 0) {
        return 0;
    }
    return RunImpl(reader, nEntry, "entries", std::forward<decltype(f)>(f));
}

template<muc::instantiated_from<Executor> AExecutor>
template<Modelized M, std::integral T>
auto Processor<AExecutor>::Run(RDFEventReader<T, M>& reader,
                               std::invocable<bool, ArcTupleVector<M>> auto&& f) -> Index {
    const auto nEvent{gsl::narrow<Index>(reader.NEvent())};
    if (nEvent == 0) {
        return 0;
    }
    return RunImpl(reader, nEvent, "events", std::forward<decltype(f)>(f));
}

template<muc::instantiated_from<Executor> AExecutor>
template<Modelized... Ms, std::integral T>
auto Processor<AExecutor>::Run(RDFEventReader<T, Ms...>& reader,
                               std::invocable<bool, ArcTupleVector<Ms>...> auto&& f) -> Index {
    const auto nEvent{gsl::narrow<Index>(reader.NEvent())};
    if (nEvent == 0) {
        return 0;
    }
    return RunImpl(reader, nEvent, "events", std::forward<decltype(f)>(f));
}

template<muc::instantiated_from<Executor> AExecutor>
template<std::signed_integral T, typename D, std::size_t N>
auto Processor<AExecutor>::RunImpl(RDFReader<T, D, N>& reader, Index n, std::string_view what, auto&& f) -> Index {
    if (n == 0) {
        return 0;
    }

    const auto invokeUserFunc{[&](bool bypass, D&& data) {
        if constexpr (muc::tuple_like<std::decay_t<decltype(data)>>) {
            auto func{[&](auto&&... args) {
                std::invoke(f, bypass, std::forward<decltype(args)>(args)...);
            }};
            std::apply(std::move(func), std::move(data));
        } else {
            std::invoke(f, bypass, std::move(data));
        }
    }};

    Index nProcessed{};
    const auto processBatch{[&](typename RDFReader<T, D, N>::Data&& batchData) {
        for (auto&& data : batchData) {
            invokeUserFunc(/*bypass =*/false, std::move(data));
        }
        nProcessed += batchData.size();
    }};

    std::future<typename RDFReader<T, D, N>::Data> asyncRead;
    const auto byPassWillOccur{ByPassOccurrenceCheck(n, what)};
    const auto worldCommSize{mplr::comm_world().size()};
    const auto batch{CalculateBatchConfiguration(worldCommSize, n)};
    if (batch.remainder == 0) {
        MasterPrintLn("There are {} {} (proceeding as {}*{}).", n, what, batch.size, batch.count);
    } else {
        MasterPrintLn("There are {} {} (proceeding as {}*{} + {}*{}).",
                      n, what, batch.size + 1, batch.remainder, batch.size, batch.count - batch.remainder);
    }
    fExecutor.Run(std::max(static_cast<Index>(worldCommSize), batch.count), [&](auto k) { // k is batch index
        if (byPassWillOccur and k >= n) [[unlikely]] {
            // Bypass when there are too many processes. The purpose of invoking user function is to allow users to
            // perform collective communication. Otherwise, deadlock cannot be easily avoided by users.
            invokeUserFunc(/*bypass =*/true, D{});
            return;
        }
        const auto [iFirst, iLast]{CalculateIndexRange(k, batch)};
        if (not asyncRead.valid()) [[unlikely]] { // first batch
            asyncRead = reader.AsyncRead(iFirst, iLast);
            return;
        }
        // common case, process previous batch while reading next batch
        auto batchData{asyncRead.get()};
        asyncRead = reader.AsyncRead(iFirst, iLast);
        processBatch(std::move(batchData));
    });
    if (asyncRead.valid()) {
        processBatch(asyncRead.get()); // process last batch
    }
    // In parallel processing, only one process will exhaust the data (the one that processes the last batch);
    // others need to exhaust explicitly to mute warning from reader
    if (not reader.Exhausted()) {
        reader.Exhaust();
    }
    return nProcessed;
}

template<muc::instantiated_from<Executor> AExecutor>
auto Processor<AExecutor>::ByPassOccurrenceCheck(Index n, std::string_view what) -> bool {
    const auto worldCommSize{mplr::comm_world().size()};
    const auto byPassWillOccur{static_cast<Index>(worldCommSize) > n};
    if (byPassWillOccur) [[unlikely]] {
        MasterPrintWarning(fmt::format("Number of processes ({}) are more than number of {} ({}).", worldCommSize, what, n));
    }
    return byPassWillOccur;
}

template<muc::instantiated_from<Executor> AExecutor>
auto Processor<AExecutor>::CalculateBatchConfiguration(Index nProcess, Index nTotal) const -> BatchConfiguration {
    if (nTotal == 0) {
        return {};
    }
    const auto nBatchProposal{std::llround(static_cast<double>(nTotal) / fBatchSizeProposal)};
    const auto nBatch{std::clamp(gsl::narrow<Index>(nBatchProposal), std::min(nProcess, nTotal), nTotal)};
    const auto batchSize{std::div(nTotal, nBatch)};
    return {nBatch, batchSize.quot, batchSize.rem};
}

template<muc::instantiated_from<Executor> AExecutor>
auto Processor<AExecutor>::CalculateIndexRange(Index iBatch, BatchConfiguration batch) -> std::pair<Index, Index> {
    Expects(0 <= iBatch and iBatch < batch.count);
    Index iFirst;
    Index iLast;
    if (iBatch < batch.remainder) {
        // Distribute the remainder among the initial batches
        const auto size{batch.size + 1};
        iFirst = iBatch * size;
        iLast = iFirst + size;
    } else {
        iFirst = batch.remainder + iBatch * batch.size;
        iLast = iFirst + batch.size;
    }
    return {iFirst, iLast};
}

} // namespace Mustard::Data::inline Processing
