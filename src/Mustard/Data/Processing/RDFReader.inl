// -*- C++ -*-
//
// Copyright (C) 2020-2025  Mustard developers
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

// ===========================================================================
// ======== RDFReader implementation
// ===========================================================================

template<std::signed_integral T, typename D, std::size_t N>
RDFReader<T, D, N>::RDFReader(Index size, std::optional<DataFrameType> rdf) :
    NonCopyableBase{},
    fNext{},
    fData{MakeArc<Data>()},
    fFirst{},
    fLast{},
    fSize{size},
    fReader{},
    fStartReadSemaphore{0},
    fCompleteReadSemaphore{0},
    fExhausted{},
    fReading{} {
    if (ROOT::IsImplicitMTEnabled()) {
        Throw<std::runtime_error>("RDataFrame reader cannot be used with IMT enabled");
    }
    // Start reader thread
    fReader = std::jthread{[this, rdf = std::move(rdf)]() mutable {
        if (fSize == 0) {
            fExhausted.test_and_set();
            return;
        }
        fStartReadSemaphore.acquire(); // Wait for main thread to start first read
        // ======== reader thread blocked until here ========
        ReaderKernel(std::move(rdf));
        fExhausted.test_and_set();
        fCompleteReadSemaphore.release(); // Notify main thread that reading is complete
    }};
}

template<std::signed_integral T, typename D, std::size_t N>
[[nodiscard]] auto RDFReader<T, D, N>::AsyncRead(Index first, Index last) -> std::future<Data> {
    Expects(std::this_thread::get_id() != fReader.get_id()); // RDFReader thread must not call this function
    if (fReading) [[unlikely]] {
        PrintError("Try to start another read while reading");
        return {};
    }
    if (fExhausted.test()) [[unlikely]] {
        PrintError("RDF have been exhausted");
        return {};
    }
    if (first > fSize) [[unlikely]] {
        PrintError(fmt::format("first index ({}) > size ({})", first, fSize));
        return {};
    }
    if (last > fSize) [[unlikely]] {
        PrintError(fmt::format("last index ({}) > size ({})", last, fSize));
        return {};
    }
    if (first > last) [[unlikely]] {
        PrintError(fmt::format("first index ({}) > last index ({})", first, last));
        return {};
    }
    if (first < fNext) [[unlikely]] {
        PrintError(fmt::format("first index ({}) < next index ({})", first, fNext));
        return {};
    }
    fFirst = first;
    fLast = last;
    if (fNext == fFirst and fFirst == fLast) {
        // Do not notify reader thread in this case, otherwise readers will be in big trouble
        return std::async(std::launch::deferred, [] { return Data{}; });
    }
    fReading = true;
    fNext = fFirst;
    fStartReadSemaphore.release(); // Notify reader thread to start reading
    // Use std::async in deferred mode to defer acquisition until the future is waited on
    return std::async(std::launch::deferred, [this, data = fData]() -> Data {
        if (data->use_count() == 1) {
            // RDFReader has been destroyed. Inherit its estate.
            return std::move(*data); // Move data out to avoid copy
        }
        Expects(fReading);                // Must be in reading state
        fCompleteReadSemaphore.acquire(); // Wait for reader thread to complete reading
        // ======== main thread blocked until here ========
        fReading = false;
        return std::move(*data); // Move data out to avoid copy
    });
}

template<std::signed_integral T, typename D, std::size_t N>
[[nodiscard]] auto RDFReader<T, D, N>::Read(Index first, Index last) -> Data {
    auto future{AsyncRead(first, last)};
    if (not future.valid()) {
        return {};
    }
    return future.get();
}

template<std::signed_integral T, typename D, std::size_t N>
auto RDFReader<T, D, N>::Exhaust() -> void {
    if (fExhausted.test()) {
        return;
    }
    const auto data{Read(fSize, fSize)}; // Skip until size to exhaust data
    Ensures(data.empty());               // Sentinel read should return empty data
    Ensures(fExhausted.test());          // Ensures that data is exhausted
}

template<std::signed_integral T, typename D, std::size_t N>
auto RDFReader<T, D, N>::CompleteRead() -> void {
    Expects(std::this_thread::get_id() == fReader.get_id()); // Only reader thread can call this function
    Expects(fReading);                                       // Must be in reading state
    if (fNext != fLast) {
        PrintError(fmt::format("Read incomplete: next index ({}) != last index ({})", fNext, fLast));
    }
    Ensures(fNext == fLast);
    fCompleteReadSemaphore.release(); // Notify main thread that current read is complete
    fStartReadSemaphore.acquire();    // Wait for main thread to start next read
    // ======== reader thread blocked until here ========
    fData->clear();
}

template<std::signed_integral T, typename D, std::size_t N>
auto RDFReader<T, D, N>::DestructorAction() -> void {
    if (fReading) {
        // Deal with early destruction while reading
        fCompleteReadSemaphore.acquire(); // Wait for reader thread to complete reading
        // ======== main thread blocked until here ========
        fReading = false;
    }
    if (not fExhausted.test()) {
        PrintWarning("RDF have not been exhausted");
        Exhaust(); // Ensure reader thread can exit properly
    }
}

// ===========================================================================
// ======== Single-RDF RDFEntryReader implementation
// ===========================================================================

template<Modelized M>
RDFEntryReader<M>::RDFEntryReader(DataFrameType rdf) :
    RDFEntryReader{rdf, CountRDFEntry(rdf)} {}

template<Modelized M>
RDFEntryReader<M>::RDFEntryReader(DataFrameType rdf, Entry nRDFEntry) :
    Base{nRDFEntry, std::move(rdf)} {}

template<Modelized M>
auto RDFEntryReader<M>::NextEntry() const -> Entry {
    if (this->Reading()) [[unlikely]] {
        PrintError("Cannot fetch next entry while reading");
        return -1;
    }
    return this->fNext;
}

template<Modelized M>
auto RDFEntryReader<M>::ReaderKernel(std::optional<DataFrameType>&& rdf) -> void {
    // Fence function to determine when to complete current read based on entry
    const auto fence{[this](ULong64_t uEntry) {
        const auto entry{gsl::narrow_cast<Entry>(uEntry)};
        if (entry == this->Last()) {
            this->CompleteRead();
            this->fData->reserve(this->Last() - this->First()); // Reserve space for next read
        }
        if (entry < this->First()) {
            return false;
        }
        return true;
    }};
    // Read function to read data into result vector
    const auto read{[this]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        return [this](std::add_lvalue_reference_t<typename std::tuple_element_t<
                          Is, typename M::StdTuple>::Type>... args) {
            this->fData->emplace_back(MakeArcTuple<M>(std::move(args)...)); // Add a new entry
            ++this->fNext;
        };
    }(gslx::make_index_sequence<M::Size()>{})};
    // Apply fence and read functions to RDF
    rdf.value().Filter(fence, {"rdfentry_"}).Foreach(read, M::NameVector());
}

// ===========================================================================
// ======== Multi-RDF RDFEntryReader implementation
// ===========================================================================

template<Modelized... Ms>
RDFEntryReader<Ms...>::RDFEntryReader(DataFrameType rdf) :
    RDFEntryReader{
        rdf, [&rdf] {
            const auto nEntry{CountRDFEntry({rdf.cbegin(), rdf.cend()})};
            std::array<Entry, N{}> result;
            std::ranges::copy(nEntry, result.begin());
            return result;
        }()} {}

template<Modelized... Ms>
RDFEntryReader<Ms...>::RDFEntryReader(DataFrameType rdf, const std::array<Entry, N{}>& nRDFEntry) :
    Base{*std::ranges::max_element(nRDFEntry)},
    fSubReader{} {
    // Initialize sub-readers for each RDF
    [&]<gsl::index... Ks>(gslx::index_sequence<Ks...>) {
        (..., get<Ks>(fSubReader).emplace(std::move(rdf[Ks]), nRDFEntry[Ks]));
    }(gslx::make_index_sequence<N{}>{});
}

template<Modelized... Ms>
auto RDFEntryReader<Ms...>::NextEntry() const -> Entry {
    if (this->Reading()) [[unlikely]] {
        PrintError("Cannot fetch next entry while reading");
        return -1;
    }
    return this->fNext;
}

template<Modelized... Ms>
auto RDFEntryReader<Ms...>::ReaderKernel(std::optional<DataFrameType>&&) -> void {
    while (true) {
        // Get data from sub-readers and move into current read data
        this->fData->resize(this->Last() - this->First());
        const auto getReadData{[&]<gsl::index K>() {
            if (get<K>(fSubReader)->Exhausted()) {
                return;
            }
            const auto nEntry{get<K>(fSubReader)->NEntry()};
            const auto first{std::min(this->First(), nEntry)};
            const auto last{std::min(this->Last(), nEntry)};
            for (auto iData{this->fData->begin()};
                 auto&& entryData : get<K>(fSubReader)->Read(first, last)) {
                get<K>(*iData++) = std::move(entryData);
            }
        }};
        [&]<gsl::index... Ks>(gslx::index_sequence<Ks...>) {
            (..., getReadData.template operator()<Ks>());
        }(gslx::make_index_sequence<N{}>{});
        // Exit or wait for next read
        this->fNext = this->Last();
        if (this->Last() == NEntry()) {
            break;
        }
        this->CompleteRead();
    }
}

// ===========================================================================
// ======== Single-RDF RDFEventReader implementation
// ===========================================================================

template<std::integral T, Modelized M>
RDFEventReader<T, M>::RDFEventReader(DataFrameType rdf, std::string eventIDColumnName) :
    RDFEventReader{rdf, MakeArc<SingleRDFEventInfo<T>>(rdf, std::move(eventIDColumnName))} {}

template<std::integral T, Modelized M>
RDFEventReader<T, M>::RDFEventReader(DataFrameType rdf, Arc<SingleRDFEventInfo<T>> rdfEventInfo) :
    Base{rdfEventInfo->NEvent(), std::move(rdf)},
    fEntry{},
    fRDFEventInfo{std::move(rdfEventInfo)} {}

template<std::integral T, Modelized M>
auto RDFEventReader<T, M>::NextEvtIdx() const -> Index {
    if (this->Reading()) [[unlikely]] {
        PrintError("Cannot fetch next event index while reading");
        return -1;
    }
    return this->fNext;
}

template<std::integral T, Modelized M>
auto RDFEventReader<T, M>::NextEntry() const -> Entry {
    if (this->Reading()) [[unlikely]] {
        PrintError("Cannot fetch next entry while reading");
        return -1;
    }
    return fEntry;
}

template<std::integral T, Modelized M>
auto RDFEventReader<T, M>::ReaderKernel(std::optional<DataFrameType>&& rdf) -> void {
    const auto& event{fRDFEventInfo};
    // Fence function to determine when to complete current read and start next read based on event ID column
    const auto fence{[&](ULong64_t entry) {
        fEntry = gsl::narrow_cast<Entry>(entry);
        if (fEntry == event->Entry(this->Last())) {
            this->CompleteRead();
            this->fData->reserve(this->Last() - this->First()); // Reserve space for next read
        }
        if (fEntry < event->Entry(this->First())) {
            return false;
        }
        if (fEntry == event->Entry(this->fNext)) {
            this->fData->emplace_back().reserve(event->Size(this->fNext)); // Add a new event
            ++this->fNext;
        }
        return true;
    }};
    // Read function to read data into result vector of current event
    const auto read{[this]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        return [this](std::add_lvalue_reference_t<typename std::tuple_element_t<
                          Is, typename M::StdTuple>::Type>... args) {
            this->fData->back().emplace_back(MakeArcTuple<M>(std::move(args)...)); // Append to current event
        };
    }(gslx::make_index_sequence<M::Size()>{})};
    // Apply fence and read functions to RDF
    rdf.value().Filter(fence, {"rdfentry_"}).Foreach(read, M::NameVector());
}

// ===========================================================================
// ======== Multi-RDF RDFEventReader implementation
// ===========================================================================

template<std::integral T, Modelized... Ms>
RDFEventReader<T, Ms...>::RDFEventReader(DataFrameType rdf, const std::string& eventIDColumnName) :
    RDFEventReader{rdf, MakeArc<MultiRDFEventInfo<T, N{}>>(rdf, eventIDColumnName)} {}

template<std::integral T, Modelized... Ms>
RDFEventReader<T, Ms...>::RDFEventReader(DataFrameType rdf, std::array<std::string, N{}> eventIDColumnName) :
    RDFEventReader{rdf, MakeArc<MultiRDFEventInfo<T, N{}>>(rdf, std::move(eventIDColumnName))} {}

template<std::integral T, Modelized... Ms>
RDFEventReader<T, Ms...>::RDFEventReader(DataFrameType rdf, Arc<MultiRDFEventInfo<T, N{}>> rdfEventInfo) :
    Base{rdfEventInfo->NEvent()},
    fRDFEventInfo{std::move(rdfEventInfo)},
    fSubReader{},
    fMisalignedEventCountWarningEmitted{},
    fMisalignedEventCountWarningThreshold{1000},
    fMisalignedEventCountFatalThreshold{10000} {
    // Initialize sub-readers for each RDF
    [&]<gsl::index... Ks>(gslx::index_sequence<Ks...>) {
        (..., get<Ks>(fSubReader).emplace(std::move(rdf[Ks]), fRDFEventInfo->PerRDFEventInfo(Ks)));
    }(gslx::make_index_sequence<N{}>{});
}

template<std::integral T, Modelized... Ms>
auto RDFEventReader<T, Ms...>::NextEvtIdx() const -> Index {
    if (this->Reading()) [[unlikely]] {
        PrintError("Cannot fetch next event index while reading");
        return -1;
    }
    return this->fNext;
}

template<std::integral T, Modelized... Ms>
auto RDFEventReader<T, Ms...>::MisalignedEventCountWarningThreshold(Index n) -> void {
    if (this->Reading()) [[unlikely]] {
        PrintError("Cannot set misaligned event count warning threshold while reading");
        return;
    }
    fMisalignedEventCountWarningThreshold = n;
}

template<std::integral T, Modelized... Ms>
auto RDFEventReader<T, Ms...>::MisalignedEventCountFatalThreshold(Index n) -> void {
    if (this->Reading()) [[unlikely]] {
        PrintError("Cannot set misaligned event count fatal threshold while reading");
        return;
    }
    fMisalignedEventCountFatalThreshold = n;
}

template<std::integral T, Modelized... Ms>
auto RDFEventReader<T, Ms...>::ReaderKernel(std::optional<DataFrameType>&&) -> void {
    // Misaligned events storage (globIdx -> event)
    std::tuple<gtl::flat_hash_map<Index, ArcTupleVector<Ms>>...> misalignedEvent;
    // Main loop to read events one by one by global event index (globIdx)
    while (true) {
        // Find event-block index range to read
        struct {
            Index first;
            Index last;
        } subReadRange[N{}];
        const auto initSubReadRange{[&]<gsl::index K>() {
            auto& [first, last]{subReadRange[K]};
            first = this->First() < NEvent() ?
                        fRDFEventInfo->MinLocalEvtIdxAfter(this->First(), K) :
                        fRDFEventInfo->RDF(K).NEvent();
            last = -1;
        }};
        [&]<gsl::index... Ks>(gslx::index_sequence<Ks...>) {
            (..., initSubReadRange.template operator()<Ks>());
        }(gslx::make_index_sequence<N{}>{});
        for (auto globIdx{this->First()}; globIdx < this->Last(); ++globIdx) {
            const auto adjustSubReadRangeLast{[&]<gsl::index K>() {
                if (not fRDFEventInfo->HasEvent(globIdx, K)) {
                    return;
                }
                const auto onePastLocalIdx{fRDFEventInfo->ToLocalEvtIdx(globIdx, K) + 1};
                subReadRange[K].last = std::max(subReadRange[K].last, onePastLocalIdx);
            }};
            [&]<gsl::index... Ks>(gslx::index_sequence<Ks...>) {
                (..., adjustSubReadRangeLast.template operator()<Ks>());
            }(gslx::make_index_sequence<N{}>{});
        }
        // Data for current read (globIdx -> event)
        gtl::flat_hash_map<Index, typename Data::value_type> data;
        data.reserve(this->Last() - this->First()); // Reserve space for current read data to avoid rehashing during insertion
        // Move previously passed events into current read data.
        const auto retrievePassedEvent{[&]<gsl::index K>() {
            // Move previouly passed events for this RDF into current read data,
            gtl::vector<Index> globIdxToErase; // Store global event indices to erase after iteration to avoid invalidating iterators
            globIdxToErase.reserve(get<K>(misalignedEvent).size());
            for (auto&& [globIdx, event] : get<K>(misalignedEvent)) {
                if (globIdx >= this->Last()) {
                    continue; // Not passed, keep it in misaligned event storage for future reads.
                }
                if (globIdx >= this->First()) {
                    get<K>(data[globIdx]) = std::move(event);
                }
                globIdxToErase.emplace_back(globIdx);
            }
            for (auto globIdx : std::as_const(globIdxToErase)) {
                get<K>(misalignedEvent).erase(globIdx);
            }
            // fmt::print(stderr, "{}{}", get<K>(misalignedEvent).size(), K == 2 ? '\n' : ',');
        }};
        [&]<gsl::index... Ks>(gslx::index_sequence<Ks...>) {
            (..., retrievePassedEvent.template operator()<Ks>());
        }(gslx::make_index_sequence<N{}>{});
        // Move newly read events into current read data or passed events.
        const auto getSubReadData{[&]<gsl::index K>() {
            auto& [localFirst, localLast]{subReadRange[K]};
            localFirst = std::max(localFirst, get<K>(fSubReader)->NextEvtIdx());
            localLast = std::max(localLast, localFirst);
            if (get<K>(fSubReader)->Exhausted()) {
                return; // No more event for this RDF, skip reading.
            }
            for (auto&& event : get<K>(fSubReader)->Read(localFirst, localLast)) {
                const auto globIdx{fRDFEventInfo->ToGlobEvtIdx(localFirst, K)};
                if (this->First() <= globIdx and globIdx < this->Last()) {
                    get<K>(data[globIdx]) = std::move(event);
                } else {
                    get<K>(misalignedEvent).emplace(globIdx, std::move(event));
                }
                ++localFirst;
            }
            Ensures(localFirst == localLast);
            // Check misaligned event count and emit warning or throw exception if necessary.
            const auto nMisalignedEvent{ssize(get<K>(misalignedEvent))};
            if (not fMisalignedEventCountWarningEmitted[K]) [[likely]] {
                if (nMisalignedEvent < fMisalignedEventCountWarningThreshold) [[likely]] {
                    return;
                }
                PrintWarning(fmt::format(
                    "Number of misaligned events for RDF {} reaches {}. Please be aware of memory usage. "
                    "Set the warning threshold (now {}) to a larger value to delay or suppress this warning.",
                    K, nMisalignedEvent, fMisalignedEventCountWarningThreshold));
                fMisalignedEventCountWarningEmitted[K] = true;
                return;
            }
            if (nMisalignedEvent < fMisalignedEventCountFatalThreshold) [[likely]] {
                return;
            }
            Throw<std::runtime_error>(fmt::format(
                "Number of misaligned events for RDF {} reaches {}. Cannot continue. "
                "Set the fatal threshold (now {}) to a larger value to delay or suppress this exception.",
                K, nMisalignedEvent, fMisalignedEventCountFatalThreshold));
        }};
        [&]<gsl::index... Ks>(gslx::index_sequence<Ks...>) {
            (..., getSubReadData.template operator()<Ks>());
        }(gslx::make_index_sequence<N{}>{});
        // Move data into result vector in global event index order.
        this->fData->reserve(this->Last() - this->First());
        for (auto globIdx{this->First()}; globIdx < this->Last(); ++globIdx) {
            const auto it{data.find(globIdx)};
            Ensures(it != data.cend());
            this->fData->emplace_back(std::move(it->second));
        }
        // Exit or wait for next read
        this->fNext = this->Last();
        if (this->Last() == NEvent()) {
            break;
        }
        this->CompleteRead();
    }
}

} // namespace Mustard::Data::inline Processing
