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

namespace Mustard::Parallel {

template<typename T>
    requires std::is_trivially_copyable_v<T>
SharedMemory<T>::SharedMemory() noexcept :
    fSpan{},
    fShmWinOrData{MPI_WIN_NULL} {}

template<typename T>
    requires std::is_trivially_copyable_v<T>
SharedMemory<T>::SharedMemory(std::input_iterator auto first, std::size_t count, int root,
                              muc::optional_ref<const mplr::communicator> intraNodeComm) :
    SharedMemory{} {
    Initialize(first, count, root, std::move(intraNodeComm));
}

template<typename T>
    requires std::is_trivially_copyable_v<T>
SharedMemory<T>::SharedMemory(std::nullptr_t, int root,
                              muc::optional_ref<const mplr::communicator> intraNodeComm) :
    SharedMemory{} {
    const T* dummy{};
    Initialize(dummy, 0, root, std::move(intraNodeComm));
}

template<typename T>
    requires std::is_trivially_copyable_v<T>
SharedMemory<T>::SharedMemory(std::ranges::sized_range auto&& range, int root,
                              muc::optional_ref<const mplr::communicator> intraNodeComm) :
    SharedMemory{} {
    Initialize(std::ranges::begin(range), std::ranges::size(range), root, std::move(intraNodeComm));
}

template<typename T>
    requires std::is_trivially_copyable_v<T>
SharedMemory<T>::SharedMemory(SharedMemory&& other) noexcept :
    SharedMemory{} {
    MoveFrom(std::move(other));
}

template<typename T>
    requires std::is_trivially_copyable_v<T>
auto SharedMemory<T>::operator=(SharedMemory&& other) noexcept -> SharedMemory& {
    if (this == &other) {
        return *this;
    }
    Destroy();
    MoveFrom(std::move(other));
    return *this;
}

template<typename T>
    requires std::is_trivially_copyable_v<T>
auto SharedMemory<T>::Initialize(std::input_iterator auto first, std::size_t count, int root,
                                 muc::optional_ref<const mplr::communicator> intraNodeComm) -> void {
    // Non-MPI path: store data in a local vector.
    if (not mplr::available() or mplr::comm_world().size() == 1) {
        auto& storage{fShmWinOrData.template emplace<gtl::vector<T>>()};
        storage.resize(count);
        std::ranges::copy_n(first, count, storage.begin());
        fSpan = std::span{storage};
        return;
    }

    // MPI path: allocate a shared-memory window.
    // root allocates the full window; others allocate zero-size and attach.
    const auto& comm{intraNodeComm ?
                         intraNodeComm->get() :
                         Env::MPIEnv::Instance().IntraNodeComm()};
    if (root < 0 or root >= comm.size()) {
        Throw<std::runtime_error>("root must be in [0, comm.size())");
    }

    comm.bcast(root, count);
    if (count == 0) {
        return;
    }
    const auto requiredWinSizeByte{gsl::narrow<MPI_Aint>(count * sizeof(T))};

    // Passive-target window: write-once, read-many without locks.
    mplr::info winInfo;
    winInfo.set("no_locks", "true");
    winInfo.set("same_disp_unit", "true");
    winInfo.set("alloc_shared_noncontig", "true");
    winInfo.set("mpi_minimum_memory_alignment", std::to_string(alignof(T)));

    T* shm{};
    auto& shmWin{std::get<MPI_Win>(fShmWinOrData)};
    if (comm.rank() == root) {
        MPI_Win_allocate_shared(requiredWinSizeByte, 1, winInfo.native_handle(), comm.native_handle(), &shm, &shmWin);
    } else {
        MPI_Win_allocate_shared(0, 1, winInfo.native_handle(), comm.native_handle(), &shm, &shmWin);
        MPI_Aint winSizeByte;
        int winDispUnit;
        MPI_Win_shared_query(shmWin, root, &winSizeByte, &winDispUnit, &shm);
        Ensures(winSizeByte >= requiredWinSizeByte);
        Ensures(winDispUnit == 1);
    }
    Ensures(shm != nullptr);

    // root copies data and synchronizes; others wait at barrier.
    if (comm.rank() == root) {
        std::ranges::uninitialized_copy_n(first, count, shm, shm + count);
        MPI_Win_sync(shmWin);
        comm.barrier();
    } else {
        comm.barrier();
        MPI_Win_sync(shmWin);
    }
    fSpan = std::span{shm, count};
}

template<typename T>
    requires std::is_trivially_copyable_v<T>
auto SharedMemory<T>::MoveFrom(SharedMemory&& other) noexcept -> void {
    fSpan = other.fSpan;
    fShmWinOrData = std::move(other.fShmWinOrData);
    // Reset the source's MPI_Win to prevent double-free in its destructor.
    if (auto win{std::get_if<MPI_Win>(&other.fShmWinOrData)}) {
        *win = MPI_WIN_NULL;
    }
}

template<typename T>
    requires std::is_trivially_copyable_v<T>
auto SharedMemory<T>::Destroy() noexcept -> void {
    if (auto win{std::get_if<MPI_Win>(&fShmWinOrData)}) {
        if (*win != MPI_WIN_NULL) {
            // no need to destroy the data in the window since it's trivially copyable
            MPI_Win_free(win);
        }
    }
}

} // namespace Mustard::Parallel
