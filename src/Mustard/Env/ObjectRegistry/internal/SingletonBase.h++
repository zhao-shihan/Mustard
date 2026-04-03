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
#include "Mustard/Utility/NonCopyableBase.h++"

namespace Mustard::Env::inline ObjectRegistry::internal {

/// @brief Implementation detail of Mustard::Env::Singleton.
/// @details Not API. Combines non-copyable semantics with special
/// allocation/deallocation operators.
/// @warning Singleton uses a special allocator to allocate instances,
/// and this base class inherits matching ClassSpecificAllocation to
/// allow usual delete expression on singleton instances. Do not implement
/// any allocation/deallocation function in derived classes, otherwise
/// memory could corrupt.
class SingletonBase : public ClassSpecificAllocation,
                      public NonCopyableBase {
protected:
    /// @brief Protected default constructor for mixin inheritance.
    SingletonBase() = default;
    /// @brief Protected default destructor for mixin inheritance.
    ~SingletonBase() = default;
};

} // namespace Mustard::Env::inline ObjectRegistry::internal
