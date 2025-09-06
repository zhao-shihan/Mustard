// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

template<typename AData>
AsyncReader<AData>::AsyncReader(gsl::index sentinel, std::function<void(ROOT::RDF::RNode)> ReadLoop, ROOT::RDF::RNode rdf) :
    NonCopyableBase{},
    fData{},
    fFirst{},
    fLast{},
    fSentinel{sentinel},
    fReaderThread{},
    fStartReadSemaphore{0},
    fCompleteReadSemaphore{0},
    fExhausted{},
    fReading{} {
    if (ROOT::IsImplicitMTEnabled()) {
        Throw<std::logic_error>("Async RDataFrame reader cannot be used with IMT enabled");
    }
    fReaderThread = std::jthread{
        [this](std::function<void(ROOT::RDF::RNode)> ReadLoop, ROOT::RDF::RNode rdf) {
            if (fSentinel == 0) {
                fExhausted = true;
                return;
            }
            fStartReadSemaphore.acquire();
            std::invoke(std::move(ReadLoop), std::move(rdf));
            fExhausted = true;
            fCompleteReadSemaphore.release();
        },
        std::move(ReadLoop), std::move(rdf)};
}

template<typename AData>
AsyncReader<AData>::~AsyncReader() {
    if (fReading) {
        Throw<std::logic_error>("Last read data not acquired");
    }
    if (not fExhausted) {
        PrintWarning("Data have not been exhausted");
    }
}

template<typename AData>
auto AsyncReader<AData>::Read(gsl::index first, gsl::index last) -> void {
    if (fReading) {
        Throw<std::logic_error>("Try to start another read while reading");
    }
    if (fExhausted) {
        Throw<std::logic_error>("Data have been exhausted");
    }
    if (first < 0) {
        Throw<std::out_of_range>("first < 0");
    }
    if (last < 0) {
        Throw<std::out_of_range>("last < 0");
    }
    if (first > last) {
        Throw<std::out_of_range>("first > last");
    }
    fFirst = first;
    fLast = last;
    fData.reserve(last - first);
    fReading = true;
    fStartReadSemaphore.release();
}

template<typename AData>
[[nodiscard]] auto AsyncReader<AData>::Acquire() -> AData {
    if (not fReading) {
        Throw<std::logic_error>("Try to acquire result while not reading");
    }
    fCompleteReadSemaphore.acquire();
    fReading = false;
    return std::move(fData);
}

template<typename AData>
auto AsyncReader<AData>::Exhaust() -> void {
    Read(fSentinel, fSentinel);
    Ensures(Acquire().empty());
}

template<typename AData>
auto AsyncReader<AData>::CompleteRead() -> void {
    fCompleteReadSemaphore.release();
    fStartReadSemaphore.acquire();
    fData.clear();
}

template<TupleModelizable... Ts>
AsyncEntryReader<Ts...>::AsyncEntryReader(ROOT::RDF::RNode rdf) :
    AsyncReader<muc::shared_ptrvec<Tuple<Ts...>>>{
        *rdf.Count(),
        [this](ROOT::RDF::RNode rdf) {
            rdf.Filter([this](ULong64_t uEntry) {
                   const auto entry{muc::to_signed(uEntry)};
                   if (entry == this->Last()) {
                       this->CompleteRead();
                       if (entry > this->First()) [[unlikely]] {
                           Throw<std::logic_error>(fmt::format("Current entry ({}) is larger than the specified first entry ({})", entry, this->First()));
                       }
                   }
                   if (entry < this->First()) {
                       return false;
                   }
                   return true;
               },
                       {"rdfentry_"})
                .Foreach([this]<gsl::index... Is>(gslx::index_sequence<Is...>) {
                    return [this](const typename internal::ReadHelper<Ts...>::template ReadType<Is>&... value) {
                        this->fData.emplace_back(std::make_shared<Tuple<Ts...>>(
                            internal::ReadHelper<Ts...>::template As<
                                typename internal::ReadHelper<Ts...>::template TargetType<Is>>(value)...));
                    };
                }(gslx::make_index_sequence<Tuple<Ts...>::Size()>{}),
                         Tuple<Ts...>::NameVector());
        },
        rdf} {}

// template<std::integral AEventIDType, muc::instantiated_from<TupleModel>... Ts>
// AsyncEventReader<AEventIDType, Ts...>::AsyncEventReader(std::array<ROOT::RDF::RNode, sizeof...(Ts)> rdf, std::string eventIDColumnName) :
//     AsyncEventReader{rdf, RDFEventSplit<AEventIDType>(rdf, std::move(eventIDColumnName))} {}

// template<std::integral AEventIDType, muc::instantiated_from<TupleModel>... Ts>
// AsyncEventReader<AEventIDType, Ts...>::AsyncEventReader(std::array<ROOT::RDF::RNode, sizeof...(Ts)> rdf, std::array<std::string, sizeof...(Ts)> eventIDColumnName) :
//     AsyncEventReader{rdf, RDFEventSplit<AEventIDType>(rdf, std::move(eventIDColumnName))} {}

// template<std::integral AEventIDType, muc::instantiated_from<TupleModel>... Ts>
// AsyncEventReader<AEventIDType, Ts...>::AsyncEventReader(std::array<ROOT::RDF::RNode, sizeof...(Ts)> rdf, std::vector<std::array<RDFEntryRange, sizeof...(Ts)>> eventSplit) :
//     AsyncReader<std::vector<muc::shared_ptrvec<Tuple<Ts...>>>>{},
//     fEventSplit{std::move(eventSplit)} {
// }

template<std::integral AEventIDType, TupleModelizable... Ts>
AsyncEventReader<AEventIDType, TupleModel<Ts...>>::AsyncEventReader(ROOT::RDF::RNode rdf, std::string eventIDColumnName) :
    AsyncEventReader{rdf, RDFEventSplit<AEventIDType>(rdf, std::move(eventIDColumnName))} {}

template<std::integral AEventIDType, TupleModelizable... Ts>
AsyncEventReader<AEventIDType, TupleModel<Ts...>>::AsyncEventReader(ROOT::RDF::RNode rdf, std::vector<gsl::index> eventSplit) :
    AsyncReader<std::vector<muc::shared_ptrvec<Tuple<Ts...>>>>{
        ssize(eventSplit) - 1,
        [this](ROOT::RDF::RNode rdf) {
            const auto& es{fEventSplit};
            auto nextEvent{this->First()};
            rdf.Filter([&](ULong64_t uEntry) {
                   const auto entry{muc::to_signed(uEntry)};
                   if (entry == es[this->Last()]) {
                       this->CompleteRead();
                       if (entry > es[this->First()]) {
                           Throw<std::logic_error>(fmt::format("Current entry ({}) is larger than the specified first entry ({})", entry, es[this->First()]));
                       }
                       nextEvent = this->First();
                   }
                   if (entry < es[this->First()]) {
                       return false;
                   }
                   if (entry == es[nextEvent]) {
                       this->fData.emplace_back().reserve(es[nextEvent + 1] - es[nextEvent]);
                       ++nextEvent;
                   }
                   return true;
               },
                       {"rdfentry_"})
                .Foreach([this]<gsl::index... Is>(gslx::index_sequence<Is...>) {
                    return [this](const typename internal::ReadHelper<Ts...>::template ReadType<Is>&... value) {
                        this->fData.back().emplace_back(std::make_shared<Tuple<Ts...>>(
                            internal::ReadHelper<Ts...>::template As<
                                typename internal::ReadHelper<Ts...>::template TargetType<Is>>(value)...));
                    };
                }(gslx::make_index_sequence<Tuple<Ts...>::Size()>{}),
                         Tuple<Ts...>::NameVector());
        },
        rdf},
    fEventSplit{std::move(eventSplit)} {
    Expects(not fEventSplit.empty());
    Expects(*rdf.Count() == muc::to_unsigned(fEventSplit.back()));
    Expects(std::ranges::is_sorted(fEventSplit));
}

template<std::integral AEventIDType, TupleModelizable... Ts>
auto AsyncEventReader<AEventIDType, TupleModel<Ts...>>::Read(gsl::index first, gsl::index last) -> void {
    const auto nEvent{ssize(fEventSplit) - 1};
    if (first > nEvent) {
        Throw<std::out_of_range>("first > #event");
    }
    if (last > nEvent) {
        Throw<std::out_of_range>("last > #event");
    }
    AsyncReader<std::vector<muc::shared_ptrvec<Tuple<Ts...>>>>::Read(first, last);
}

} // namespace Mustard::Data
