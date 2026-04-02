// -*- C++ -*-
//
// Copyright (C) 2020-2025  Mustard developers
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

#include "Mustard/Memory/Alloc.h++"

#if MUSTARD_USE_MIMALLOC
#    include "mimalloc.h"
#endif

namespace Mustard::inline Memory {

[[nodiscard]] auto Allocate(std::size_t n) -> void* {
#if MUSTARD_USE_MIMALLOC
    return mi_new(n);
#else
    return ::operator new(n);
#endif
}

[[nodiscard]] auto Allocate(std::size_t n, std::align_val_t al) -> void* {
#if MUSTARD_USE_MIMALLOC
    return mi_new_aligned(n, static_cast<std::size_t>(al));
#else
    return ::operator new(n, al);
#endif
}

[[nodiscard]] auto Allocate(std::size_t n, const std::nothrow_t&) noexcept -> void* {
#if MUSTARD_USE_MIMALLOC
    return mi_new_nothrow(n);
#else
    return ::operator new(n, std::nothrow);
#endif
}

[[nodiscard]] auto Allocate(std::size_t n, std::align_val_t al, const std::nothrow_t&) noexcept -> void* {
#if MUSTARD_USE_MIMALLOC
    return mi_new_aligned_nothrow(n, static_cast<std::size_t>(al));
#else
    return ::operator new(n, al, std::nothrow);
#endif
}

auto Deallocate(void* p) noexcept -> void {
#if MUSTARD_USE_MIMALLOC
    mi_free(p);
#else
    ::operator delete(p);
#endif
}

auto Deallocate(void* p, std::align_val_t al) noexcept -> void {
#if MUSTARD_USE_MIMALLOC
    mi_free_aligned(p, static_cast<std::size_t>(al));
#else
    ::operator delete(p, al);
#endif
}

auto Deallocate(void* p, std::size_t n) noexcept -> void {
#if MUSTARD_USE_MIMALLOC
    mi_free_size(p, n);
#else
    ::operator delete(p, n);
#endif
}

auto Deallocate(void* p, std::size_t n, std::align_val_t al) noexcept -> void {
#if MUSTARD_USE_MIMALLOC
    mi_free_size_aligned(p, n, static_cast<std::size_t>(al));
#else
    ::operator delete(p, n, al);
#endif
}

} // namespace Mustard::inline Memory
