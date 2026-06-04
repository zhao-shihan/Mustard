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
ShmArray<T>::ShmArray() noexcept :
    MoveOnlyBase{},
    fSpan{},
    fShmWinOrData{MPI_WIN_NULL} {}

template<typename T>
    requires std::is_trivially_copyable_v<T>
ShmArray<T>::ShmArray(std::input_iterator auto first, std::size_t count, int root,
                      muc::optional_ref<const mplr::communicator> intraNodeComm) :
    ShmArray{} {
    Init(first, count, root,
         std::move(intraNodeComm));
}

template<typename T>
    requires std::is_trivially_copyable_v<T>
ShmArray<T>::ShmArray(std::ranges::sized_range auto&& range, int root,
                      muc::optional_ref<const mplr::communicator> intraNodeComm) :
    ShmArray{} {
    Init(std::ranges::begin(range), std::ranges::size(range), root,
         std::move(intraNodeComm));
}

template<typename T>
    requires std::is_trivially_copyable_v<T>
ShmArray<T>::ShmArray(NonRootTag, int root,
                      muc::optional_ref<const mplr::communicator> intraNodeComm) :
    ShmArray{} {
    Init(static_cast<const T*>(nullptr), 0, root,
         std::move(intraNodeComm));
}

template<typename T>
    requires std::is_trivially_copyable_v<T>
ShmArray<T>::ShmArray(BcastTag, std::input_iterator auto first, std::size_t count, int root,
                      muc::optional_ref<const mplr::communicator> intraNodeComm,
                      muc::optional_ref<const mplr::communicator> interNodeComm) :
    ShmArray{} {
    Init(first, count, root,
         std::move(intraNodeComm), std::move(interNodeComm));
}

template<typename T>
    requires std::is_trivially_copyable_v<T>
ShmArray<T>::ShmArray(BcastTag, std::ranges::sized_range auto&& range, int root,
                      muc::optional_ref<const mplr::communicator> intraNodeComm,
                      muc::optional_ref<const mplr::communicator> interNodeComm) :
    ShmArray{} {
    Init(std::ranges::begin(range), std::ranges::size(range), root,
         std::move(intraNodeComm), std::move(interNodeComm));
}

template<typename T>
    requires std::is_trivially_copyable_v<T>
ShmArray<T>::ShmArray(BcastTag, NonRootTag, int root,
                      muc::optional_ref<const mplr::communicator> intraNodeComm,
                      muc::optional_ref<const mplr::communicator> interNodeComm) :
    ShmArray{} {
    Init(static_cast<const T*>(nullptr), 0, root,
         std::move(intraNodeComm), std::move(interNodeComm));
}

template<typename T>
    requires std::is_trivially_copyable_v<T>
ShmArray<T>::ShmArray(ShmArray&& other) noexcept :
    ShmArray{} {
    MoveFrom(std::move(other));
}

template<typename T>
    requires std::is_trivially_copyable_v<T>
auto ShmArray<T>::operator=(ShmArray&& other) noexcept -> ShmArray& {
    if (this == &other) {
        return *this;
    }
    Destroy();
    MoveFrom(std::move(other));
    return *this;
}

template<typename T>
    requires std::is_trivially_copyable_v<T>
auto ShmArray<T>::Init(std::input_iterator auto first, std::size_t count) -> void {
    if (count == 0) {
        return;
    }
    if (std::to_address(first) == nullptr) {
        Throw<std::invalid_argument>("first cannot be null when count is non-zero.");
    }
    auto& storage{fShmWinOrData.template emplace<gtl::vector<T>>()};
    storage.resize(count);
#ifdef __GNUG__
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wnonnull"
#endif
    std::ranges::copy_n(first, count, storage.begin());
#ifdef __GNUG__
#    pragma GCC diagnostic pop
#endif
    fSpan = std::span{storage};
}

template<typename T>
    requires std::is_trivially_copyable_v<T>
auto ShmArray<T>::Init(std::input_iterator auto first, std::size_t count, int root,
                       muc::optional_ref<const mplr::communicator> intraNodeComm) -> void {
    // Non-MPI path: store data in a local vector.
    if (not mplr::available() or mplr::comm_world().size() == 1) {
        Init(first, count);
        return;
    }

    // MPI path: create shared-memory window on the root process and copy data.
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    const auto& intraComm{intraNodeComm ?
                              intraNodeComm->get() :
                              mpiEnv.IntraNodeComm()};
    if (root < 0 or root >= intraComm.size()) {
        Throw<std::runtime_error>("root must be in [0, comm.size()).");
    }

    intraComm.ibcast(root, count)
        .wait(mplr::duty_ratio::preset::moderate);
    if (count == 0) {
        return;
    }
    if (std::to_address(first) == nullptr and intraComm.rank() == root) {
        Throw<std::invalid_argument>("first cannot be null when count is non-zero on the root process.");
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
    if (intraComm.rank() == root) {
        MPI_Win_allocate_shared(requiredWinSizeByte, 1, winInfo.native_handle(), intraComm.native_handle(), &shm, &shmWin);
    } else {
        MPI_Win_allocate_shared(0, 1, winInfo.native_handle(), intraComm.native_handle(), &shm, &shmWin);
        MPI_Aint winSizeByte;
        int winDispUnit;
        MPI_Win_shared_query(shmWin, root, &winSizeByte, &winDispUnit, &shm);
        Ensures(winSizeByte >= requiredWinSizeByte);
        Ensures(winDispUnit == 1);
    }
    Ensures(shm != nullptr);

    // root copies data and synchronizes; others wait at barrier.
    if (intraComm.rank() == root) {
#ifdef __GNUG__
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wnonnull"
#endif
        std::ranges::uninitialized_copy_n(first, count, shm, shm + count);
#ifdef __GNUG__
#    pragma GCC diagnostic pop
#endif
    }
    MPI_Win_fence(MPI_MODE_NOSUCCEED, shmWin);
    fSpan = std::span{shm, count};
}

template<typename T>
    requires std::is_trivially_copyable_v<T>
auto ShmArray<T>::Init(std::input_iterator auto first, std::size_t count, int root,
                       muc::optional_ref<const mplr::communicator> intraNodeComm,
                       muc::optional_ref<const mplr::communicator> interNodeComm) -> void {
    // Non-MPI path: store data in a local vector.
    if (not mplr::available() or mplr::comm_world().size() == 1) {
        Init(first, count);
        return;
    }

    // Single-node: intra-node sharing directly from source data.
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    const auto& intraComm{intraNodeComm ?
                              intraNodeComm->get() :
                              mpiEnv.IntraNodeComm()};
    if (mpiEnv.OnSingleNode()) {
        Init(first, count, 0, intraComm);
        return;
    }

    // Multi-node path: inter-node broadcast before intra-node sharing.
    const auto& interComm{interNodeComm ?
                              interNodeComm->get() :
                              mpiEnv.InterNodeComm()};
    if (intraComm == interComm) {
        Throw<std::invalid_argument>("intraNodeComm and interNodeComm cannot be the same communicator.");
    }
    if (interComm.is_valid()) {
        if (root < 0 or mpiEnv.ClusterSize() <= root) {
            Throw<std::out_of_range>(fmt::format("Invalid root node index {}: must be in [0, {}).",
                                                 root, mpiEnv.ClusterSize()));
        }
        if (std::to_address(first) == nullptr and count != 0 and mpiEnv.LocalNodeIdx() == root) {
            Throw<std::invalid_argument>("first cannot be null when count is non-zero on the root process.");
        }
        auto bcastCount{interComm.ibcast(root, count)};
        gtl::vector<T> bcastData;
        if (mpiEnv.LocalNodeIdx() == root) {
            bcastData.resize(count);
#ifdef __GNUG__
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wnonnull"
#endif
            std::ranges::copy_n(first, count, bcastData.begin());
#ifdef __GNUG__
#    pragma GCC diagnostic pop
#endif
        }
        bcastCount.wait(mplr::duty_ratio::preset::moderate);
        if (mpiEnv.LocalNodeIdx() != root) {
            bcastData.resize(count);
        }
        if (count != 0) {
            interComm.ibcast(root, bcastData.data(), mplr::vector_layout<T>{count})
                .wait(mplr::duty_ratio::preset::active);
        }
        Init(bcastData.begin(), bcastData.size(), 0, intraComm);
    } else { // Non-leader rank on multi-node: attach without source data.
        Init(static_cast<const T*>(nullptr), 0, 0, intraComm);
    }
}

template<typename T>
    requires std::is_trivially_copyable_v<T>
auto ShmArray<T>::MoveFrom(ShmArray&& other) noexcept -> void {
    fSpan = other.fSpan;
    fShmWinOrData = std::move(other.fShmWinOrData);
    // Reset the source's MPI_Win to prevent double-free in its destructor.
    if (auto win{std::get_if<MPI_Win>(&other.fShmWinOrData)}) {
        *win = MPI_WIN_NULL;
    }
}

template<typename T>
    requires std::is_trivially_copyable_v<T>
auto ShmArray<T>::Destroy() noexcept -> void {
    if (auto win{std::get_if<MPI_Win>(&fShmWinOrData)}) {
        if (*win != MPI_WIN_NULL) {
            // no need to destroy the data in the window since it's trivially copyable
            MPI_Win_free(win);
        }
    }
}

} // namespace Mustard::Parallel
