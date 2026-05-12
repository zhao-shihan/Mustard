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

#pragma once

#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Memory/Allocator.h++"
#include "Mustard/Memory/Arc.h++"
#include "Mustard/Utility/NonCopyableBase.h++"

#include "ROOT/RDataFrame.hxx"
#include "TROOT.h"

#include "muc/future"

#include "gsl/gsl"

#include "fmt/format.h"

#include <array>
#include <atomic>
#include <concepts>
#include <cstddef>
#include <functional>
#include <semaphore>
#include <stdexcept>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace Mustard::Data::inline Processing {

/// @brief Asynchronous RDataFrame reader framework providing thread-safe access to ROOT RDF data.
/// @details
/// This template class serves as the base for all RDataFrame readers in Mustard. It implements
/// an asynchronous reading pattern using a worker thread, allowing non-blocking access to RDF
/// segments while maintaining synchronization through binary semaphores. The reader supports:
/// - Asynchronous and synchronous read operations with future support
/// - Sequential cursor-based reading with skip and exhaust operations
/// - Arbitrary range-based reading via Read(first, last)
/// - Thread-safe state management and cleanup on destruction
///
/// @tparam T Signed integral type used for segment indices (typically gsl::index)
/// @tparam D Data container type (std::vector with Allocator, or tuple of such containers)
/// @tparam N Number of RDataFrame sources (1 for single RDF, N>1 for multiple RDF)
///
/// @note ROOT's implicit multi-threading (IMT) must be disabled. The reader will reject
///       construction if IMT is enabled.
/// @note Not copy-assignable or copy-constructible (inherits from NonCopyableBase).
template<std::signed_integral T, typename D, std::size_t N>
class RDFReader : public NonCopyableBase {
public:
    using Entry = gsl::index;
    using Index = T;
    using Data = std::vector<D, Allocator<D>>;

protected:
    using DataFrameType = std::conditional_t<N == 1, ROOT::RDF::RNode, std::array<ROOT::RDF::RNode, N>>;

protected:
    /// @brief Construct a reader for a specific size and data frame.
    /// @param size Maximum number of entries or grouped events to read.
    /// @param rdf The underlying RDataFrame structure (single node or node array).
    /// @pre ROOT IMT is disabled, otherwise construction throws.
    explicit RDFReader(Index size, DataFrameType rdf);
    /// @brief Virtual destructor. Derived classes are responsible for calling `DestructorAction()` before teardown.
    virtual ~RDFReader() = default;

public:
    /// @brief Asynchronously read a batch of segments in the range `[first, last)`.
    /// @param first Inclusive begin index of the requested range.
    /// @param last Exclusive end index of the requested range.
    /// @return A future that resolves to the requested data block.
    /// @note Returns an empty block when range checks fail or state is invalid.
    /// @pre Must be called from non-reader thread.
    [[nodiscard]] auto AsyncRead(Index first, Index last) -> std::future<Data>;
    /// @brief Asynchronously read the next `n` segments.
    /// @param n Number of consecutive segments to read from current cursor.
    /// @return A future that resolves to the requested data block.
    /// @note Returns an empty block if `fNext + n` exceeds size.
    [[nodiscard]] auto AsyncReadNext(Index n) -> std::future<Data>;
    /// @brief Asynchronously read exactly one segment from the next position.
    /// @return A future that resolves to one segment value.
    /// @note Returns a default-constructed value if no data is produced.
    [[nodiscard]] auto AsyncReadNext() -> std::future<typename Data::value_type>;
    /// @brief Asynchronously skip to a given absolute segment index.
    /// @param idx Destination cursor index after skipping.
    /// @return A future that resolves when skipping is completed.
    /// @note Out-of-range or backward skip requests become no-op.
    [[nodiscard]] auto AsyncSkipTo(Index idx) -> std::future<void>;
    /// @brief Asynchronously skip the next `n` segments.
    /// @param n Number of segments to skip from current cursor.
    /// @return A future that resolves when skipping is completed.
    /// @note Overrun requests become no-op.
    [[nodiscard]] auto AsyncSkipNext(Index n = 1) -> std::future<void>;
    /// @brief Asynchronously exhaust all remaining segments in the reader.
    /// @return A future that resolves when reader reaches exhausted state.
    /// @note Repeated calls after exhaustion are no-op.
    [[nodiscard]] auto AsyncExhaust() -> std::future<void>;

    /// @brief Synchronously read a batch of segments in the range `[first, last)`.
    [[nodiscard]] auto Read(Index first, Index last) -> Data { return AsyncRead(first, last).get(); }
    /// @brief Synchronously read the next `n` segments.
    [[nodiscard]] auto ReadNext(Index n) -> Data { return AsyncReadNext(n).get(); }
    /// @brief Synchronously read exactly one segment from the next position.
    [[nodiscard]] auto ReadNext() -> typename Data::value_type { return AsyncReadNext().get(); }
    /// @brief Synchronously skip to a given absolute segment index.
    auto SkipTo(Index idx) -> void { AsyncSkipTo(idx).get(); }
    /// @brief Synchronously skip the next `n` segments.
    auto SkipNext(Index n = 1) -> void { AsyncSkipNext(n).get(); }
    /// @brief Synchronously exhaust all remaining segments in the reader.
    auto Exhaust() -> void { AsyncExhaust().get(); }

    /// @brief Reset the reader state completely to the beginning.
    /// @details
    /// If the reader has not been exhausted, it will first be exhausted (blocking until
    /// the worker reaches terminal state) to let the worker thread exit cleanly. The
    /// prior reader thread is joined if joinable, internal state is reset, and a new
    /// worker thread is created. Calling while a read is in progress is rejected and
    /// does nothing.
    virtual auto Reset() -> void;

    /// @brief True if the reader is currently processing an async request.
    /// @return True when a request is active. This remains true from the point the
    ///         request is started until the associated future is consumed by the
    ///         caller (i.e. until the completion handshake completes and the main
    ///         thread clears the reading state).
    auto Reading() const -> auto { return fReading; }
    /// @brief True if the entire source length has been parsed and exhausted.
    /// @return True when the worker loop has reached terminal state.
    auto Exhausted() const -> auto { return fExhausted.test(); }

protected:
    /// @brief Core unchecked asynchronous reader backend.
    /// @param first Inclusive begin index of request.
    /// @param last Exclusive end index of request.
    /// @return A future resolved by the producer-consumer handshake.
    /// @pre Must not be called from reader thread.
    /// @warning Caller must ensure range/state validity; this function only enforces thread/state guards.
    [[nodiscard]] auto UncheckedAsyncRead(Index first, Index last) -> std::future<Data>;

    /// @brief Get the first segment of the currently active request.
    /// @return Inclusive begin index in current handshake.
    auto First() const -> auto { return fFirst; };
    /// @brief Get the last segment bound of the currently active request.
    /// @return Exclusive end index in current handshake.
    auto Last() const -> auto { return fLast; };
    /// @brief Get the overall size of segments.
    /// @return Upper bound of legal segment index range.
    auto Size() const -> auto { return fSize; }
    /// @brief Callback notifying the main thread that current reading has ended.
    /// @pre Must be called only by the reader thread while `Reading()==true`.
    /// @details Releases completion semaphore, waits next start semaphore, and clears staging buffer.
    auto CompleteRead() -> void;

    /// @brief Called by all derived classes to ensure thread-safety on destruction.
    /// @details
    /// Synchronizes pending reads and forces exhaustion when needed so the reader thread can
    /// terminate without racing object destruction. This may block: if a read is active it
    /// waits for the reader thread to finish the request, and if the reader is not yet
    /// exhausted it calls Exhaust() (which itself blocks until the worker reaches
    /// terminal state) to allow the background thread to exit safely.
    /// @pre Must be called from non-reader thread.
    auto DestructorAction() -> void;

private:
    /// @brief Background main loop for the reading thread.
    /// @details Waits for start signal, runs @ref ReaderKernel, then marks exhausted and notifies completion.
    auto ReaderThreadFunction() -> void;
    /// @brief Pure virtual hook acting as the reading engine for each specific subclass.
    /// @details Implementations should repeatedly fill @ref fData, update @ref fNext and call @ref CompleteRead.
    virtual auto ReaderKernel() -> void = 0;

protected:
    DataFrameType fRDF; ///< Underlying RDataFrame node(s) consumed by the reader kernel.
    Index fNext;        ///< Next global segment index to be produced after current operation.
    Arc<Data> fData;    ///< Shared staging buffer exchanged between worker and caller through futures.

private:
    Index fFirst; ///< Inclusive begin index for the active request.
    Index fLast;  ///< Exclusive end index for the active request.
    Index fSize;  ///< Total readable segment count (entry count or event block count).

    std::jthread fReader;                         ///< Background worker thread running @ref ReaderThreadFunction.
    std::binary_semaphore fStartReadSemaphore;    ///< Main->worker signal: start processing a prepared request.
    std::binary_semaphore fCompleteReadSemaphore; ///< Worker->main signal: current request has been completed.
    std::atomic_flag fExhausted;                  ///< Terminal flag set when worker loop has reached end-of-input.
    bool fReading;                                ///< Main-thread state guard indicating one in-flight request.
};

} // namespace Mustard::Data::inline Processing

#include "Mustard/Data/Processing/RDFReader.inl"
