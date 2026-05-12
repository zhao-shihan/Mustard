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

template<std::signed_integral T, typename D, std::size_t N>
RDFReader<T, D, N>::RDFReader(Index size, DataFrameType rdf) :
    NonCopyableBase{},
    fRDF{std::move(rdf)},
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
    fReader = std::jthread{std::mem_fn(&RDFReader::ReaderThreadFunction), this};
}

template<std::signed_integral T, typename D, std::size_t N>
[[nodiscard]] auto RDFReader<T, D, N>::AsyncRead(Index first, Index last) -> std::future<Data> {
    if (first > fSize) [[unlikely]] {
        PrintError(fmt::format("First index ({}) > size ({}). Returning empty data.", first, fSize));
        return muc::defer([] { return Data{}; });
    }
    if (last > fSize) [[unlikely]] {
        PrintError(fmt::format("Last index ({}) > size ({}). Returning empty data.", last, fSize));
        return muc::defer([] { return Data{}; });
    }
    if (first > last) [[unlikely]] {
        PrintError(fmt::format("First index ({}) > last index ({}). Returning empty data.", first, last));
        return muc::defer([] { return Data{}; });
    }
    if (first < fNext) [[unlikely]] {
        PrintError(fmt::format("First index ({}) < next index ({}). Returning empty data.", first, fNext));
        return muc::defer([] { return Data{}; });
    }
    return UncheckedAsyncRead(first, last);
}

template<std::signed_integral T, typename D, std::size_t N>
[[nodiscard]] auto RDFReader<T, D, N>::AsyncReadNext(Index n) -> std::future<Data> {
    const auto last{fNext + n};
    if (last > fSize) [[unlikely]] {
        PrintError(fmt::format("Reading next {} entries would exceed size ({}). Returning empty data.", n, fSize));
        return muc::defer([] { return Data{}; });
    }
    return UncheckedAsyncRead(fNext, last);
}

template<std::signed_integral T, typename D, std::size_t N>
[[nodiscard]] auto RDFReader<T, D, N>::AsyncReadNext() -> std::future<typename Data::value_type> {
    return muc::defer([asyncRead = AsyncReadNext(1)]() mutable {
        auto data{asyncRead.get()};
        if (data.empty()) [[unlikely]] {
            return typename Data::value_type{};
        }
        return std::move(data.front());
    });
}

template<std::signed_integral T, typename D, std::size_t N>
[[nodiscard]] auto RDFReader<T, D, N>::AsyncSkipTo(Index idx) -> std::future<void> {
    if (idx > fSize) [[unlikely]] {
        PrintError(fmt::format("Target index ({}) > size ({}). Doing nothing.", idx, fSize));
        return muc::defer([] {});
    }
    if (idx < fNext) [[unlikely]] {
        PrintError(fmt::format("Target index ({}) < next index ({}). Doing nothing.", idx, fNext));
        return muc::defer([] {});
    }
    return muc::defer([asyncRead = UncheckedAsyncRead(idx, idx)]() mutable {
        auto data{asyncRead.get()};
        Ensures(data.empty()); // Sentinel read should return empty data
    });
}

template<std::signed_integral T, typename D, std::size_t N>
[[nodiscard]] auto RDFReader<T, D, N>::AsyncSkipNext(Index n) -> std::future<void> {
    const auto dest{fNext + n};
    if (dest > fSize) [[unlikely]] {
        PrintError(fmt::format("Skipping next {} entries would exceed size ({}). Doing nothing.", n, fSize));
        return muc::defer([] {});
    }
    return muc::defer([asyncRead = UncheckedAsyncRead(dest, dest)]() mutable {
        auto data{asyncRead.get()};
        Ensures(data.empty()); // Sentinel read should return empty data
    });
}

template<std::signed_integral T, typename D, std::size_t N>
[[nodiscard]] auto RDFReader<T, D, N>::AsyncExhaust() -> std::future<void> {
    if (fExhausted.test()) [[unlikely]] {
        PrintWarning("RDF have already been exhausted. Doing nothing.");
        return muc::defer([] {});
    }
    return muc::defer([this, asyncRead = UncheckedAsyncRead(fSize, fSize)]() mutable {
        auto data{asyncRead.get()};
        Ensures(data.empty());      // Sentinel read should return empty data
        Ensures(fExhausted.test()); // Ensures that data is exhausted
    });
}

template<std::signed_integral T, typename D, std::size_t N>
auto RDFReader<T, D, N>::Reset() -> void {
    if (fReading) [[unlikely]] {
        PrintError("Try to reset while reading. Doing nothing.");
        return;
    }
    if (not fExhausted.test()) [[unlikely]] {
        PrintWarning("RDF have not been exhausted. Exhausting now to ensure reader thread can restart properly.");
        Exhaust(); // Ensure reader thread can restart properly
    }
    if (fReader.joinable()) {
        fReader.join();
    }
    // Reset state
    fNext = 0;
    fData->clear();
    fExhausted.clear();
    // Restart reader thread
    fReader = std::jthread{std::mem_fn(&RDFReader::ReaderThreadFunction), this};
}

template<std::signed_integral T, typename D, std::size_t N>
[[nodiscard]] auto RDFReader<T, D, N>::UncheckedAsyncRead(Index first, Index last) -> std::future<Data> {
    Expects(std::this_thread::get_id() != fReader.get_id()); // Reader thread must not call this function
    if (fReading) [[unlikely]] {
        PrintError("Try to start another read while reading. Returning empty data.");
        return muc::defer([] { return Data{}; });
    }
    if (fExhausted.test()) [[unlikely]] {
        PrintWarning("RDF have already been exhausted. Returning empty data.");
        return muc::defer([] { return Data{}; });
    }
    fFirst = first;
    fLast = last;
    if (fNext == fFirst and fFirst == fLast) {
        // Do not notify reader thread in this case, otherwise readers will be in big trouble
        return muc::defer([] { return Data{}; });
    }
    fReading = true;
    fNext = fFirst;
    fStartReadSemaphore.release(); // Notify reader thread to start reading
    // Defer acquisition until the future is waited on
    return muc::defer([this, data = fData]() -> Data {
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
auto RDFReader<T, D, N>::CompleteRead() -> void {
    Expects(std::this_thread::get_id() == fReader.get_id()); // Only reader thread can call this function
    Expects(fReading);                                       // Must be in reading state
    Ensures(fNext == fLast);
    fCompleteReadSemaphore.release(); // Notify main thread that current read is complete
    fStartReadSemaphore.acquire();    // Wait for main thread to start next read
    // ======== reader thread blocked until here ========
    fData->clear();
}

template<std::signed_integral T, typename D, std::size_t N>
auto RDFReader<T, D, N>::DestructorAction() -> void {
    Expects(std::this_thread::get_id() != fReader.get_id()); // Reader thread must not call this function
    if (fReading) {
        // Deal with early destruction while reading
        fCompleteReadSemaphore.acquire(); // Wait for reader thread to complete reading
        // ======== main thread blocked until here ========
        fReading = false;
    }
    if (not fExhausted.test()) [[unlikely]] {
        PrintWarning("RDF have not been exhausted. Exhausting now to ensure reader thread can exit properly.");
        Exhaust(); // Ensure reader thread can exit properly
    }
}

template<std::signed_integral T, typename D, std::size_t N>
auto RDFReader<T, D, N>::ReaderThreadFunction() -> void {
    Expects(fData->empty());
    Expects(not fExhausted.test());
    if (fSize == 0) {
        fExhausted.test_and_set();
        return;
    }
    fStartReadSemaphore.acquire(); // Wait for main thread to start first read
    // ======== reader thread blocked until here ========
    ReaderKernel();
    fExhausted.test_and_set();
    fCompleteReadSemaphore.release(); // Notify main thread that reading is complete
}

} // namespace Mustard::Data::inline Processing

#include "Mustard/Data/Processing/MultiRDFEntryReader.inl"
#include "Mustard/Data/Processing/MultiRDFEventReader.inl"
#include "Mustard/Data/Processing/SingleRDFEntryReader.inl"
#include "Mustard/Data/Processing/SingleRDFEventReader.inl"
