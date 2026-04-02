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

#include "gtl/intrusive.hpp"

#include "muc/concepts"

#include <concepts>
#include <utility>

namespace Mustard::inline Memory {

namespace impl {

/// @brief Internal storage type for Arc.
/// @tparam T User object type.
/// @warning ArcObj is an implementation detail and should not be used directly by users.
/// @details
/// ArcObj inherit from gtl::intrusive_ref_counter to provide thread-safe
/// reference counting, and inherit from T to store user data.
template<typename T>
class ArcObj : public gtl::intrusive_ref_counter<ArcObj<T>, gtl::thread_safe_counter>,
               public T {
public:
    /// @brief Inherit all T constructors
    using T::T;

    /// @brief Allocate ArcObj with Mustard memory allocation primitive.
    [[nodiscard]] auto operator new(std::size_t n) -> void* { return Allocate(n); }
    [[nodiscard]] auto operator new(std::size_t n, std::align_val_t al) -> void* { return Allocate(n, al); }
    [[nodiscard]] auto operator new(std::size_t n, const std::nothrow_t& nt) noexcept -> void* { return Allocate(n, nt); }
    [[nodiscard]] auto operator new(std::size_t n, std::align_val_t al, const std::nothrow_t& nt) noexcept -> void* { return Allocate(n, al, nt); }

    /// @brief Deallocate ArcObj with Mustard memory deallocation primitive.
    auto operator delete(void* p) noexcept -> void { Deallocate(p); }
    auto operator delete(void* p, std::align_val_t al) noexcept -> void { Deallocate(p, al); }
    auto operator delete(void* p, std::size_t n) noexcept -> void { Deallocate(p, n); }
    auto operator delete(void* p, std::size_t n, std::align_val_t al) noexcept -> void { Deallocate(p, n, al); }
    auto operator delete(void* p, const std::nothrow_t&) noexcept -> void { Deallocate(p); }
    auto operator delete(void* p, std::align_val_t al, const std::nothrow_t&) noexcept -> void { Deallocate(p, al); }

    /// @brief Array allocation/deallocation is intentionally disabled for ArcObj.
    auto operator new[](std::size_t) -> void* = delete;
    auto operator new[](std::size_t, std::align_val_t) -> void* = delete;
    auto operator new[](std::size_t, const std::nothrow_t&) noexcept -> void* = delete;
    auto operator new[](std::size_t, std::align_val_t, const std::nothrow_t&) noexcept -> void* = delete;
    auto operator delete[](void*) noexcept -> void = delete;
    auto operator delete[](void*, std::align_val_t) noexcept -> void = delete;
    auto operator delete[](void*, std::size_t) noexcept -> void = delete;
    auto operator delete[](void*, std::size_t, std::align_val_t) noexcept -> void = delete;
    auto operator delete[](void*, const std::nothrow_t&) noexcept -> void = delete;
    auto operator delete[](void*, std::align_val_t, const std::nothrow_t&) noexcept -> void = delete;
};

} // namespace impl

/// @brief Automatically reference-counted (Arc) smart pointer for shared ownership of objects.
/// @tparam T User object type.
/// @note Reference counting is thread-safe.
/// @warning Arc's element type inherits from T but is not T itself... so if anything funny happens,
/// you can always fall back to std::shared_ptr.
/// @details
/// Arc stores an impl::ArcObj<T> and shares ownership using intrusive reference counting.
/// This is just an alias of gtl::intrusive_ptr.
template<typename T>
using Arc = gtl::intrusive_ptr<impl::ArcObj<T>>;

/// @brief Construct an Arc-managed object.
/// @tparam T User object type.
/// @tparam Us Constructor argument types.
/// @param args Arguments forwarded to T's constructor.
/// @return Arc<T> that shares ownership of the created object.
template<typename T, typename... Us>
    requires std::constructible_from<T, Us&&...>
auto MakeArc(Us&&... args) -> Arc<T> { return new impl::ArcObj<T>(std::forward<Us>(args)...); }

} // namespace Mustard::inline Memory
