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

namespace Mustard::inline Utility {

/// @brief A base class that prohibits construction.
/// @details Declaring the default constructor as deleted prevents any instance
/// of the class (or its derived classes) from being constructed. Use this as
/// a purely organizational base to signal that the type is not meant to be
/// instantiated.
class NonConstructibleBase {
public:
    constexpr NonConstructibleBase() = delete;
};

/// @brief A base class that makes derived types move-only.
/// @details Declaring the move constructor and move-assignment operator
/// suppresses the implicit generation of copy operations.
class MoveOnlyBase {
protected:
    constexpr MoveOnlyBase() noexcept = default;
    constexpr ~MoveOnlyBase() = default;

public:
    constexpr MoveOnlyBase(MoveOnlyBase&&) noexcept = default;
    constexpr auto operator=(MoveOnlyBase&&) noexcept -> MoveOnlyBase& = default;
};

/// @brief A base class that makes derived types non-copyable.
/// @details Explicitly deletes the copy constructor and copy-assignment
/// operator. As required by the rule of five, the move operations are omitted
/// so they are implicitly suppressed (they fall back to copy, which is
/// deleted).
class NonCopyableBase {
protected:
    constexpr NonCopyableBase() noexcept = default;
    constexpr ~NonCopyableBase() = default;

public:
    constexpr NonCopyableBase(const NonCopyableBase&) = delete;
    constexpr auto operator=(const NonCopyableBase&) -> NonCopyableBase& = delete;
};

/// @brief A convenience base for classes with virtual destructors that should
/// be fully copyable and movable.
/// @details Classes with a virtual `=default` destructor must explicitly
/// declare the remaining special member functions to satisfy the rule of five.
/// Inheriting from this base avoids manual boilerplate.
/// @note C.21: If you define or `=delete` any copy, move, or destructor
/// function, define or `=delete` them all.
class MovableVirtualBase {
protected:
    constexpr MovableVirtualBase() noexcept = default;
    constexpr virtual ~MovableVirtualBase() = default;

public:
    constexpr MovableVirtualBase(const MovableVirtualBase&) noexcept = default;
    constexpr MovableVirtualBase(MovableVirtualBase&&) noexcept = default;
    constexpr auto operator=(const MovableVirtualBase&) noexcept -> MovableVirtualBase& = default;
    constexpr auto operator=(MovableVirtualBase&&) noexcept -> MovableVirtualBase& = default;
};

/// @brief A convenience base for classes with virtual destructors that should
/// be move-only.
/// @details Classes with a virtual `=default` destructor must explicitly
/// declare the remaining special member functions to satisfy the rule of five.
/// Inheriting from this base avoids manual boilerplate.
/// @note C.21: If you define or `=delete` any copy, move, or destructor
/// function, define or `=delete` them all.
class MoveOnlyVirtualBase {
protected:
    constexpr MoveOnlyVirtualBase() noexcept = default;
    constexpr virtual ~MoveOnlyVirtualBase() = default;

public:
    constexpr MoveOnlyVirtualBase(MoveOnlyVirtualBase&&) noexcept = default;
    constexpr auto operator=(MoveOnlyVirtualBase&&) noexcept -> MoveOnlyVirtualBase& = default;
};

/// @brief A convenience base for classes with virtual destructors that should
/// be non-copyable.
/// @details Classes with a virtual `=default` destructor must explicitly
/// declare the remaining special member functions to satisfy the rule of five.
/// Inheriting from this base avoids manual boilerplate.
/// @note C.21: If you define or `=delete` any copy, move, or destructor
/// function, define or `=delete` them all.
class NonCopyableVirtualBase {
protected:
    constexpr NonCopyableVirtualBase() noexcept = default;
    constexpr virtual ~NonCopyableVirtualBase() = default;

public:
    constexpr NonCopyableVirtualBase(const NonCopyableVirtualBase&) = delete;
    constexpr auto operator=(const NonCopyableVirtualBase&) -> NonCopyableVirtualBase& = delete;
};

} // namespace Mustard::inline Utility
