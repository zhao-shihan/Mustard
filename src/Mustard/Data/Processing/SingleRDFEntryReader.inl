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

template<Modelized M>
RDFEntryReader<M>::RDFEntryReader(DataFrameType rdf) :
    RDFEntryReader{rdf, CountRDFEntry(rdf)} {}

template<Modelized M>
RDFEntryReader<M>::RDFEntryReader(DataFrameType rdf, Entry nRDFEntry) :
    Base{nRDFEntry, std::move(rdf)} {}

template<Modelized M>
auto RDFEntryReader<M>::NextEntry() const -> Entry {
    if (this->Reading()) [[unlikely]] {
        PrintError("Cannot fetch next entry while reading. Returning -1.");
        return -1;
    }
    return this->fNext;
}

template<Modelized M>
auto RDFEntryReader<M>::ReaderKernel() -> void {
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
    this->fRDF.Filter(fence, {"rdfentry_"}).Foreach(read, M::NameVector());
}

} // namespace Mustard::Data::inline Processing
