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

#include "Mustard/Memory/ClassSpecificAllocation.h++"

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
/// ArcObj inherit from ClassSpecificAllocation to redirect dynamic allocation to
/// Mustard memory primitives, inherit from gtl::intrusive_ref_counter to provide
/// thread-safe reference counting, and inherit from T to store user data.
template<typename T>
class ArcObj : public ClassSpecificAllocation,
               public gtl::intrusive_ref_counter<ArcObj<T>, gtl::thread_safe_counter>,
               public T {
public:
    /// @brief Inherit all T constructors
    using T::T;
};

} // namespace impl

/// @brief Automatically reference-counted (Arc) smart pointer for shared ownership of objects.
/// @tparam T User object type.
/// @note Reference counting is thread-safe.
/// @warning Arc's element type inherits from T but is not T itself... so if anything funny happens,
/// you can always fall back to std::shared_ptr. Managed class should not define their custom
/// allocation/deallocation functions, otherwise compilation will fail.
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
