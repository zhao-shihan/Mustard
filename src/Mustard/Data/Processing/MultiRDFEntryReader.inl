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
    Base{*std::ranges::max_element(nRDFEntry), rdf},
    fSubReader{} {
    // Initialize sub-readers for each RDF
    [&]<gsl::index... Ks>(gslx::index_sequence<Ks...>) {
        (..., get<Ks>(fSubReader).emplace(std::move(rdf[Ks]), nRDFEntry[Ks]));
    }(gslx::make_index_sequence<N{}>{});
}

template<Modelized... Ms>
auto RDFEntryReader<Ms...>::Reset() -> void {
    Base::Reset();
    std::apply([](auto&... rd) { (..., rd->Reset()); }, fSubReader);
}

template<Modelized... Ms>
auto RDFEntryReader<Ms...>::NextEntry() const -> Entry {
    if (this->Reading()) [[unlikely]] {
        PrintError("Cannot fetch next entry while reading. Returning -1.");
        return -1;
    }
    return this->fNext;
}

template<Modelized... Ms>
auto RDFEntryReader<Ms...>::ReaderKernel() -> void {
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
                 auto&& entryData : get<K>(fSubReader)->UncheckedAsyncRead(first, last).get()) {
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

} // namespace Mustard::Data::inline Processing
