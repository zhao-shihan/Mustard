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
                          Is, typename M::StdTuple>::Type>... args) {
            this->fData->back().emplace_back(MakeArcTuple<M>(std::move(args)...)); // Append to current event
        };
    }(gslx::make_index_sequence<M::Size()>{})};
    // Apply fence and read functions to RDF
    this->fRDF.Filter(fence, {"rdfentry_"}).Foreach(read, M::NameVector());
}

} // namespace Mustard::Data::inline Processing
