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

#include "Mustard/Memory/Alloc.h++"

#include <cstddef>
#include <limits>
#include <new>
#include <type_traits>

namespace Mustard::inline Memory {

/// @brief STL-compatible allocator backed by Mustard memory primitives.
/// @tparam T Value type allocated by this allocator.
template<typename T>
class Allocator {
public:
    using value_type = T;
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;
    using is_always_equal = std::true_type;

public:
    constexpr Allocator() noexcept = default;
    template<typename U>
    constexpr Allocator(const Allocator<U>&) noexcept {}

    /// @brief Allocates storage for @p n elements.
    /// @param n Number of elements to allocate.
    /// @return Pointer to uninitialized storage for @p n elements.
    /// @throws std::bad_array_new_length If byte size overflows.
    [[nodiscard]] auto allocate(std::size_t n) -> value_type*;
    /// @brief Deallocates storage previously returned by allocate.
    /// @param p Pointer returned by allocate.
    /// @param n Number of elements associated with @p p.
    auto deallocate(value_type* p, std::size_t n) noexcept -> void;

    template<typename U>
    constexpr auto operator==(const Allocator<U>&) const noexcept -> bool { return true; }
};

} // namespace Mustard::inline Memory

#include "Mustard/Memory/Allocator.inl"
