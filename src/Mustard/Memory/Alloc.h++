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

#pragma once

#include <cstddef>
#include <new>

namespace Mustard::inline Memory {

/// @brief Allocate raw storage for @p n bytes.
/// @param n Number of bytes to allocate.
/// @return Pointer to the allocated storage.
/// @throws std::bad_alloc when allocation fails.
[[nodiscard]] auto Allocate(std::size_t n) -> void*;

/// @brief Allocate aligned raw storage for @p n bytes.
/// @param n Number of bytes to allocate.
/// @param al Requested alignment.
/// @return Pointer to the allocated storage.
/// @throws std::bad_alloc when allocation fails.
[[nodiscard]] auto Allocate(std::size_t n, std::align_val_t al) -> void*;

/// @brief Allocate raw storage for @p n bytes with nothrow semantics.
/// @param n Number of bytes to allocate.
/// @param nt Nothrow tag selecting the non-throwing allocation form.
/// @return Pointer to allocated storage, or @c nullptr on failure.
[[nodiscard]] auto Allocate(std::size_t n, const std::nothrow_t&) noexcept -> void*;

/// @brief Allocate aligned raw storage for @p n bytes with nothrow semantics.
/// @param n Number of bytes to allocate.
/// @param al Requested alignment.
/// @param nt Nothrow tag selecting the non-throwing allocation form.
/// @return Pointer to allocated storage, or @c nullptr on failure.
[[nodiscard]] auto Allocate(std::size_t n, std::align_val_t al, const std::nothrow_t&) noexcept -> void*;

/// @brief Deallocate storage allocated by the corresponding unaligned @c Allocate overload.
/// @param p Pointer to storage to deallocate, accepts @c nullptr.
auto Deallocate(void* p) noexcept -> void;

/// @brief Deallocate storage allocated by the corresponding aligned @c Allocate overload.
/// @param p Pointer to storage to deallocate, accepts @c nullptr.
/// @param al Alignment used for allocation.
auto Deallocate(void* p, std::align_val_t al) noexcept -> void;

/// @brief Sized deallocation for storage allocated by the corresponding unaligned @c Allocate overload.
/// @param p Pointer to storage to deallocate, accepts @c nullptr.
/// @param n Allocation size in bytes.
auto Deallocate(void* p, std::size_t n) noexcept -> void;

/// @brief Sized aligned deallocation for storage allocated by the corresponding aligned @c Allocate overload.
/// @param p Pointer to storage to deallocate, accepts @c nullptr.
/// @param n Allocation size in bytes.
/// @param al Alignment used for allocation.
auto Deallocate(void* p, std::size_t n, std::align_val_t al) noexcept -> void;

} // namespace Mustard::inline Memory
