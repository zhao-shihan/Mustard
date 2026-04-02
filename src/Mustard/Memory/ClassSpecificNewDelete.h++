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

namespace Mustard::inline Memory {

/// @brief Mixin base class that redirects class-specific dynamic allocation to Mustard memory primitives.
/// @details
/// Inheriting from this class replaces global @c new/@c new[] and @c delete/@c delete[]
/// for the derived type with overloads backed by Mustard::Allocate and
/// Mustard::Deallocate. The full standard family is provided, including
/// aligned, @c nothrow, and sized-deallocation forms.
class ClassSpecificNewDelete {
protected:
    /// @brief Protected default constructor for mixin-style inheritance.
    constexpr ClassSpecificNewDelete() = default;
    /// @brief Protected default destructor for mixin-style inheritance.
    constexpr ~ClassSpecificNewDelete() = default;

public:
    /// @name Scalar Allocation
    /// @brief Overloads used by @c new for single-object allocation.
    /// @{
    [[nodiscard]] auto operator new(std::size_t n) -> void* { return Allocate(n); }
    [[nodiscard]] auto operator new(std::size_t n, std::align_val_t al) -> void* { return Allocate(n, al); }
    [[nodiscard]] auto operator new(std::size_t n, const std::nothrow_t& nt) noexcept -> void* { return Allocate(n, nt); }
    [[nodiscard]] auto operator new(std::size_t n, std::align_val_t al, const std::nothrow_t& nt) noexcept -> void* { return Allocate(n, al, nt); }
    /// @}

    /// @name Array Allocation
    /// @brief Overloads used by @c new[] for array allocation.
    /// @{
    [[nodiscard]] auto operator new[](std::size_t n) -> void* { return Allocate(n); }
    [[nodiscard]] auto operator new[](std::size_t n, std::align_val_t al) -> void* { return Allocate(n, al); }
    [[nodiscard]] auto operator new[](std::size_t n, const std::nothrow_t& nt) noexcept -> void* { return Allocate(n, nt); }
    [[nodiscard]] auto operator new[](std::size_t n, std::align_val_t al, const std::nothrow_t& nt) noexcept -> void* { return Allocate(n, al, nt); }
    /// @}

    /// @name Scalar Deallocation
    /// @brief Overloads used by @c delete for single-object deallocation.
    /// @{
    auto operator delete(void* p) noexcept -> void { Deallocate(p); }
    auto operator delete(void* p, std::align_val_t al) noexcept -> void { Deallocate(p, al); }
    auto operator delete(void* p, std::size_t n) noexcept -> void { Deallocate(p, n); }
    auto operator delete(void* p, std::size_t n, std::align_val_t al) noexcept -> void { Deallocate(p, n, al); }
    auto operator delete(void* p, const std::nothrow_t&) noexcept -> void { Deallocate(p); }
    auto operator delete(void* p, std::align_val_t al, const std::nothrow_t&) noexcept -> void { Deallocate(p, al); }
    /// @}

    /// @name Array Deallocation
    /// @brief Overloads used by @c delete[] for array deallocation.
    /// @{
    auto operator delete[](void* p) noexcept -> void { Deallocate(p); }
    auto operator delete[](void* p, std::align_val_t al) noexcept -> void { Deallocate(p, al); }
    auto operator delete[](void* p, std::size_t n) noexcept -> void { Deallocate(p, n); }
    auto operator delete[](void* p, std::size_t n, std::align_val_t al) noexcept -> void { Deallocate(p, n, al); }
    auto operator delete[](void* p, const std::nothrow_t&) noexcept -> void { Deallocate(p); }
    auto operator delete[](void* p, std::align_val_t al, const std::nothrow_t&) noexcept -> void { Deallocate(p, al); }
    /// @}
};

} // namespace Mustard::inline Memory
