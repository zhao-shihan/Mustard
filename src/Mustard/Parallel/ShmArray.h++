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

#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Utility/MoveOnlyBase.h++"

#include "gtl/vector.hpp"

#include "mplr/mplr.hpp"

#include "mpi.h"

#include "muc/optional"

#include "gsl/gsl"

#include "fmt/format.h"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace Mustard::Parallel {

/// @brief Tag type for constructing a @ref ShmArray on a non-root rank
/// that does not provide source data.
struct NonRootTag {};

/// @brief Tag type for constructing a @ref ShmArray with inter-node broadcast.
///
/// When used as the first constructor argument, enables the inter-node broadcast
/// path: data is broadcast from the root node leader to all node leaders via the
/// inter-node communicator, then shared within each node via MPI shared memory.
struct BcastTag {};

/// @brief Read-only shared-memory array of trivially-copyable elements.
/// @tparam T Element type; must be trivially copyable.
/// @details
/// @ref ShmArray stores a contiguous array of @p T in MPI shared memory
/// within a single compute node, providing read-only access via the standard
/// @c std::span API. When MPI is unavailable or only one process exists, the
/// data is stored locally in a @c gtl::vector.
///
/// When an inter-node communicator is provided and the job runs on multiple
/// nodes, data is first broadcast from the root node leader to all node
/// leaders via the inter-node communicator, then shared within each node
/// via MPI shared memory.
///
/// Storage model:
/// @li When an inter-node communicator is provided and valid, the process
///     with rank @p root in the inter-node communicator broadcasts data to
///     all node leaders. Each node leader then allocates an intra-node
///     shared memory window and copies the received data into it.
/// @li Without inter-node broadcast, the process with rank @p root in the
///     intra-node communicator allocates the shared memory window and copies
///     data into it.
/// @li Other processes allocate zero-size windows and attach via
///     @c MPI_Win_shared_query.
/// @li After construction, the shared-memory window is synchronized and all
///     processes in the communicator can read the data concurrently.
///
/// Lifetime:
/// @li @ref ShmArray is movable but not copyable (inherits @ref MoveOnlyBase).
/// @li The moved-from object is left in a valid but unspecified state.
/// @li Destruction is collective: all processes in the communicator must destroy
///     their @ref ShmArray objects together.
///
/// Thread safety:
/// @li Construct and destroy must not overlap.
/// @li Concurrent reads from multiple threads are safe after construction
///     completes.
///
/// Empty storage:
/// @li @c count == 0 is valid; all accessors behave as on a zero-length span.
template<typename T>
    requires std::is_trivially_copyable_v<T>
class ShmArray : public MoveOnlyBase {
public:
    /// @brief Default-construct an empty shared-memory array.
    ///
    /// Creates an empty container with no elements and no shared-memory window.
    /// The object can receive a value later via move-assignment.
    ShmArray() noexcept;
    /// @brief Construct a shared-memory array from an iterator range, with
    ///        intra-node sharing only.
    /// @param first Iterator to the first element to copy.
    /// @param count Number of elements to copy. Only meaningful on @p root;
    ///        broadcast to all ranks in the intra-node communicator.
    /// @param root Rank in the intra-node communicator that allocates the shared
    ///        memory window. Default 0.
    /// @param intraNodeComm Optional intra-node communicator.
    ///        If absent, @ref Env::MPIEnv::IntraNodeComm() is used.
    ///        Provide an explicit communicator duplicated with
    ///        @c mplr::communicator(comm, info) when calling from a non-main
    ///        thread.
    /// @pre @p first points to at least @p count valid elements.
    /// @pre @p root is in <tt>[0, intraNodeComm.size())</tt>.
    /// @warning In MPI mode this constructor is collective over the intra-node
    ///          communicator.
    /// @note In MPI mode, only the root rank is required to provide meaningful
    ///       data via @p first and @p count; both are ignored on other ranks.
    explicit ShmArray(std::input_iterator auto first, std::size_t count, int root = 0,
                      muc::optional_ref<const mplr::communicator> intraNodeComm = {});
    /// @brief Construct a shared-memory array from a sized range, with
    ///        intra-node sharing only.
    /// @param range A sized range whose elements are copied into shared memory.
    /// @param root Rank in the intra-node communicator that allocates the shared
    ///        memory window. Default 0.
    /// @param intraNodeComm Optional intra-node communicator.
    ///        If absent, @ref Env::MPIEnv::IntraNodeComm() is used.
    ///        Provide an explicit communicator duplicated with
    ///        @c mplr::communicator(comm, info) when calling from a non-main
    ///        thread.
    /// @pre @p root is in <tt>[0, intraNodeComm.size())</tt>.
    /// @warning In MPI mode this constructor is collective over the intra-node
    ///          communicator.
    /// @note Only the root rank needs to provide a meaningful @p range;
    ///       the range is ignored on other ranks.
    explicit ShmArray(std::ranges::sized_range auto&& range, int root = 0,
                      muc::optional_ref<const mplr::communicator> intraNodeComm = {});
    /// @brief Construct a shared-memory array on a non-root rank without source
    ///        data, with intra-node sharing only.
    /// @param root Rank in the intra-node communicator that allocates the shared
    ///        memory window. Default 0. The calling process must <em>not</em> be @p root.
    /// @param intraNodeComm Optional intra-node communicator.
    ///        If absent, @ref Env::MPIEnv::IntraNodeComm() is used.
    ///        Provide an explicit communicator duplicated with
    ///        @c mplr::communicator(comm, info) when calling from a non-main
    ///        thread.
    /// @pre In MPI mode, the calling process must <em>not</em> be @p root.
    /// @warning This constructor is collective. All ranks must specify the same
    ///          @p root.
    /// @note The element count is broadcast from @p root and does not need to
    ///       be provided on this rank.
    explicit ShmArray(NonRootTag, int root = 0,
                      muc::optional_ref<const mplr::communicator> intraNodeComm = {});

    /// @brief Construct a shared-memory array from an iterator range, with
    ///        inter-node broadcast.
    /// @param first Iterator to the first element to copy.
    /// @param count Number of elements to copy. Only meaningful on @p root;
    ///        broadcast to all ranks in the inter-node communicator.
    /// @param root Rank in the inter-node communicator of the node holding
    ///        the source data. Default 0.
    /// @param intraNodeComm Optional intra-node communicator.
    ///        If absent, @ref Env::MPIEnv::IntraNodeComm() is used.
    ///        Provide an explicit communicator duplicated with
    ///        @c mplr::communicator(comm, info) when calling from a non-main
    ///        thread.
    /// @param interNodeComm Optional inter-node communicator.
    ///        If absent, @ref Env::MPIEnv::InterNodeComm() is used.
    ///        When provided and valid on the calling process, data is
    ///        broadcast from @p root to all node leaders before shared-memory
    ///        allocation. Otherwise only intra-node sharing occurs.
    /// @pre @p first points to at least @p count valid elements.
    /// @pre When @p interNodeComm is valid, @p root is in
    ///      <tt>[0, Env::MPIEnv::ClusterSize())</tt>; otherwise @p root is
    ///      in <tt>[0, intraNodeComm.size())</tt>.
    /// @warning In MPI mode this constructor is collective over the intra-node
    ///          communicator. When @p interNodeComm is valid, it is additionally
    ///          collective over the inter-node communicator.
    /// @note In MPI mode, only the root rank is required to provide meaningful
    ///       data via @p first and @p count; both are ignored on other ranks.
    explicit ShmArray(BcastTag, std::input_iterator auto first, std::size_t count, int root = 0,
                      muc::optional_ref<const mplr::communicator> intraNodeComm = {},
                      muc::optional_ref<const mplr::communicator> interNodeComm = {});
    /// @brief Construct a shared-memory array from a sized range, with
    ///        inter-node broadcast.
    /// @param range A sized range whose elements are copied into shared memory.
    /// @param root Rank in the inter-node communicator of the node holding
    ///        the source data. Default 0.
    /// @param intraNodeComm Optional intra-node communicator.
    ///        If absent, @ref Env::MPIEnv::IntraNodeComm() is used.
    ///        Provide an explicit communicator duplicated with
    ///        @c mplr::communicator(comm, info) when calling from a non-main
    ///        thread.
    /// @param interNodeComm Optional inter-node communicator.
    ///        If absent, @ref Env::MPIEnv::InterNodeComm() is used.
    ///        When provided and valid on the calling process, data is
    ///        broadcast from @p root to all node leaders before shared-memory
    ///        allocation. Otherwise only intra-node sharing occurs.
    /// @pre When @p interNodeComm is valid, @p root is in
    ///      <tt>[0, Env::MPIEnv::ClusterSize())</tt>; otherwise @p root is
    ///      in <tt>[0, intraNodeComm.size())</tt>.
    /// @warning In MPI mode this constructor is collective over the intra-node
    ///          communicator. When @p interNodeComm is valid, it is additionally
    ///          collective over the inter-node communicator.
    /// @note Only the root rank needs to provide a meaningful @p range;
    ///       the range is ignored on other ranks.
    explicit ShmArray(BcastTag, std::ranges::sized_range auto&& range, int root = 0,
                      muc::optional_ref<const mplr::communicator> intraNodeComm = {},
                      muc::optional_ref<const mplr::communicator> interNodeComm = {});
    /// @brief Construct a shared-memory array on a non-root rank without source
    ///        data, with inter-node broadcast.
    /// @param root Rank in the inter-node communicator of the node holding
    ///        the source data. Default 0. The calling process must <em>not</em> be @p root.
    /// @param intraNodeComm Optional intra-node communicator.
    ///        If absent, @ref Env::MPIEnv::IntraNodeComm() is used.
    ///        Provide an explicit communicator duplicated with
    ///        @c mplr::communicator(comm, info) when calling from a non-main
    ///        thread.
    /// @param interNodeComm Optional inter-node communicator.
    ///        If absent, @ref Env::MPIEnv::InterNodeComm() is used.
    ///        When provided and valid on the calling process, data is
    ///        broadcast from @p root to all node leaders before shared-memory
    ///        allocation. Otherwise only intra-node sharing occurs.
    /// @pre In MPI mode, the calling process must <em>not</em> be @p root.
    /// @pre When @p interNodeComm is valid, @p root is in
    ///      <tt>[0, Env::MPIEnv::ClusterSize())</tt>; otherwise @p root is
    ///      in <tt>[0, intraNodeComm.size())</tt>.
    /// @warning This constructor is collective over the intra-node
    ///          communicator. When @p interNodeComm is valid, it is additionally
    ///          collective over the inter-node communicator.
    /// @note The element count is broadcast from @p root and does not need to
    ///       be provided on this rank.
    explicit ShmArray(BcastTag, NonRootTag, int root = 0,
                      muc::optional_ref<const mplr::communicator> intraNodeComm = {},
                      muc::optional_ref<const mplr::communicator> interNodeComm = {});

    /// @brief Move-construct, transferring ownership of the shared-memory window.
    /// @param other The source object. After the move, @p other holds no window
    ///        and is safe to destroy.
    /// @note This is not a collective operation; only the calling process
    ///       transfers its local handle.
    ShmArray(ShmArray&& other) noexcept;

    /// @brief Destroy the shared-memory window or local storage.
    /// @warning This is a collective operation in MPI mode. All processes in the
    ///          intra-node communicator must call the destructor together.
    ~ShmArray() { Destroy(); }

    /// @brief Move-assign, releasing the current window and taking ownership
    ///        from @p other.
    /// @param other The source object. After the move, @p other holds no window
    ///        and is safe to destroy.
    auto operator=(ShmArray&& other) noexcept -> ShmArray&;

    /// @brief Iterator to the first element.
    auto begin() const -> auto { return fSpan.begin(); }
    /// @brief Iterator past the last element.
    auto end() const -> auto { return fSpan.end(); }
    /// @brief Reverse iterator to the last element.
    auto rbegin() const -> auto { return fSpan.rbegin(); }
    /// @brief Reverse iterator before the first element.
    auto rend() const -> auto { return fSpan.rend(); }

    /// @brief Reference to the first element.
    /// @pre @ref empty() is @c false.
    auto front() const -> const T& { return fSpan.front(); }
    /// @brief Reference to the last element.
    /// @pre @ref empty() is @c false.
    auto back() const -> const T& { return fSpan.back(); }
    /// @brief Reference to the @p i-th element.
    /// @pre @p i is in <tt>[0, size())</tt>.
    auto operator[](std::size_t i) const -> const T& { return fSpan[i]; }
    /// @brief Pointer to the underlying element storage.
    auto data() const -> const T* { return fSpan.data(); }

    /// @brief Number of elements.
    auto size() const -> std::size_t { return fSpan.size(); }
    /// @brief Size in bytes (<tt>size() * sizeof(T)</tt>).
    auto size_bytes() const -> std::size_t { return fSpan.size_bytes(); }
    /// @brief Whether the container is empty.
    auto empty() const -> bool { return fSpan.empty(); }

private:
    /// @brief Store data in a local vector (non-MPI path).
    auto Init(std::input_iterator auto first, std::size_t count) -> void;
    /// @brief Allocate an intra-node shared-memory window and copy data into it.
    auto Init(std::input_iterator auto first, std::size_t count, int root,
              muc::optional_ref<const mplr::communicator> intraNodeComm) -> void;
    /// @brief Broadcast data across nodes via the inter-node communicator, then
    ///        share within each node via intra-node shared memory.
    auto Init(std::input_iterator auto first, std::size_t count, int root,
              muc::optional_ref<const mplr::communicator> intraNodeComm,
              muc::optional_ref<const mplr::communicator> interNodeComm) -> void;
    /// @brief Transfer ownership from @p other and leave it in a safe-to-destroy state.
    auto MoveFrom(ShmArray&& other) noexcept -> void;
    /// @brief Release the MPI shared-memory window (if active) and destroy elements.
    /// @note After destruction of the MPI window, the current object must not be
    ///       used before receiving a new value via move-assignment.
    auto Destroy() noexcept -> void;

private:
    std::span<const T> fSpan;                            ///< Non-owning view of elements (into shared memory or local vector).
    std::variant<MPI_Win, gtl::vector<T>> fShmWinOrData; ///< Storage backend: MPI shared-memory window or local data.
};

} // namespace Mustard::Parallel

#include "Mustard/Parallel/ShmArray.inl"
