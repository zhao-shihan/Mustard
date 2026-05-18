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
        PrintError("Cannot fetch next event index while reading. Returning -1.");
        return -1;
    }
    return this->fNext;
}

template<std::integral T, Modelized M>
auto RDFEventReader<T, M>::NextEntry() const -> Entry {
    if (this->Reading()) [[unlikely]] {
        PrintError("Cannot fetch next entry while reading. Returning -1.");
        return -1;
    }
    return fEntry;
}

template<std::integral T, Modelized M>
auto RDFEventReader<T, M>::ReaderKernel() -> void {
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
                          Is, typename M::StdTuple>::PersistentType>... args) {
            this->fData->back().emplace_back(MakeArcTuple<M>(std::move(args)...)); // Append to current event
        };
    }(gslx::make_index_sequence<M::Size()>{})};
    // Apply fence and read functions to RDF
    this->fRDF.Filter(fence, {"rdfentry_"}).Foreach(read, M::NameVector());
}

template<std::integral T, Modelized... Ms>
RDFEventReader<T, Ms...>::RDFEventReader(DataFrameType rdf, const std::string& eventIDColumnName) :
    RDFEventReader{rdf, MakeArc<MultiRDFEventInfo<T, N{}>>(rdf, eventIDColumnName)} {}

template<std::integral T, Modelized... Ms>
RDFEventReader<T, Ms...>::RDFEventReader(DataFrameType rdf, std::array<std::string, N{}> eventIDColumnName) :
    RDFEventReader{rdf, MakeArc<MultiRDFEventInfo<T, N{}>>(rdf, std::move(eventIDColumnName))} {}

template<std::integral T, Modelized... Ms>
RDFEventReader<T, Ms...>::RDFEventReader(DataFrameType rdf, Arc<MultiRDFEventInfo<T, N{}>> rdfEventInfo) :
    Base{rdfEventInfo->NEvent(), rdf},
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
auto RDFEventReader<T, Ms...>::Reset() -> void {
    Base::Reset();
    std::apply([](auto&... rd) { (..., rd->Reset()); }, fSubReader);
}

template<std::integral T, Modelized... Ms>
auto RDFEventReader<T, Ms...>::NextEvtIdx() const -> Index {
    if (this->Reading()) [[unlikely]] {
        PrintError("Cannot fetch next event index while reading. Returning -1.");
        return -1;
    }
    return this->fNext;
}

template<std::integral T, Modelized... Ms>
auto RDFEventReader<T, Ms...>::MisalignedEventCountWarningThreshold(Index n) -> void {
    if (this->Reading()) [[unlikely]] {
        PrintError("Cannot set misaligned event count warning threshold while reading. Not setting it.");
        return;
    }
    fMisalignedEventCountWarningThreshold = n;
}

template<std::integral T, Modelized... Ms>
auto RDFEventReader<T, Ms...>::MisalignedEventCountFatalThreshold(Index n) -> void {
    if (this->Reading()) [[unlikely]] {
        PrintError("Cannot set misaligned event count fatal threshold while reading. Not setting it.");
        return;
    }
    fMisalignedEventCountFatalThreshold = n;
}

template<std::integral T, Modelized... Ms>
auto RDFEventReader<T, Ms...>::ReaderKernel() -> void {
    // Misaligned events storage (globIdx -> event), min-heap ordered by globIdx
    constexpr auto globIdxMinHeapComp{[](const auto& lhs, const auto& rhs) {
        return lhs.first > rhs.first;
    }};
    std::tuple<std::vector<std::pair<Index, ArcTupleVector<Ms>>>...> misalignedEvent;
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
        this->fData->resize(this->Last() - this->First());
        // Move previously passed events into current read data.
        const auto retrievePassedEvent{[&]<gsl::index K>() {
            while (not get<K>(misalignedEvent).empty()) {
                auto& [globIdx, event]{get<K>(misalignedEvent).front()};
                if (globIdx >= this->Last()) {
                    break;
                }
                if (globIdx >= this->First()) {
                    get<K>((*this->fData)[globIdx - this->First()]) = std::move(event);
                }
                std::ranges::pop_heap(get<K>(misalignedEvent), globIdxMinHeapComp);
                get<K>(misalignedEvent).pop_back();
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
            for (auto&& event : get<K>(fSubReader)->UncheckedAsyncRead(localFirst, localLast).get()) {
                const auto globIdx{fRDFEventInfo->ToGlobEvtIdx(localFirst, K)};
                if (this->First() <= globIdx and globIdx < this->Last()) {
                    get<K>((*this->fData)[globIdx - this->First()]) = std::move(event);
                } else {
                    get<K>(misalignedEvent).emplace_back(globIdx, std::move(event));
                    std::ranges::push_heap(get<K>(misalignedEvent), globIdxMinHeapComp);
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
        // Exit or wait for next read
        this->fNext = this->Last();
        if (this->Last() == NEvent()) {
            break;
        }
        this->CompleteRead();
    }
}

} // namespace Mustard::Data::inline Processing
