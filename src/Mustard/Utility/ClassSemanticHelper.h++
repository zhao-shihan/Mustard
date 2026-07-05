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
class NonconstructibleBase {
public:
    constexpr NonconstructibleBase() = delete;
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
class NoncopyableBase {
protected:
    constexpr NoncopyableBase() noexcept = default;
    constexpr ~NoncopyableBase() = default;

public:
    constexpr NoncopyableBase(const NoncopyableBase&) = delete;
    constexpr auto operator=(const NoncopyableBase&) -> NoncopyableBase& = delete;
};

/// @brief A convenience base for classes with virtual destructors that should
/// be fully copyable and movable.
/// @details Classes with a virtual `=default` destructor must explicitly
/// declare the remaining special member functions to satisfy the rule of five.
/// Inheriting from this base avoids manual boilerplate.
/// @note C.21: If you define or `=delete` any copy, move, or destructor
/// function, define or `=delete` them all.
class VirtualMovableBase {
protected:
    constexpr VirtualMovableBase() noexcept = default;
    constexpr virtual ~VirtualMovableBase() = default;

public:
    constexpr VirtualMovableBase(const VirtualMovableBase&) noexcept = default;
    constexpr VirtualMovableBase(VirtualMovableBase&&) noexcept = default;
    constexpr auto operator=(const VirtualMovableBase&) noexcept -> VirtualMovableBase& = default;
    constexpr auto operator=(VirtualMovableBase&&) noexcept -> VirtualMovableBase& = default;
};

/// @brief A convenience base for classes with virtual destructors that should
/// be move-only.
/// @details Classes with a virtual `=default` destructor must explicitly
/// declare the remaining special member functions to satisfy the rule of five.
/// Inheriting from this base avoids manual boilerplate.
/// @note C.21: If you define or `=delete` any copy, move, or destructor
/// function, define or `=delete` them all.
class VirtualMoveOnlyBase {
protected:
    constexpr VirtualMoveOnlyBase() noexcept = default;
    constexpr virtual ~VirtualMoveOnlyBase() = default;

public:
    constexpr VirtualMoveOnlyBase(VirtualMoveOnlyBase&&) noexcept = default;
    constexpr auto operator=(VirtualMoveOnlyBase&&) noexcept -> VirtualMoveOnlyBase& = default;
};

/// @brief Rule of five helper for simple non-virtual base classes want to
/// hide their destructor without disabling their move sliently.
/// @note Member visibility is set to private after the macro expansion.
#define MUSTARD_UTILITY_DECLARE_NONVIRTUAL_PROTECTED_DEFAULT_DESTRUCTOR_IN_DEFAULT_MOVEABLE(ClassName) \
protected:                                                                                             \
    constexpr ~ClassName() = default;                                                                  \
                                                                                                       \
public:                                                                                                \
    constexpr ClassName(const ClassName&) noexcept = default;                                          \
    constexpr ClassName(ClassName&&) noexcept = default;                                               \
    constexpr auto operator=(const ClassName&) noexcept -> ClassName& = default;                       \
    constexpr auto operator=(ClassName&&) noexcept -> ClassName& = default;                            \
                                                                                                       \
private:

/// @brief Rule of five helper for simple non-virtual base classes want to
/// hide their destructor without disabling their move sliently.
/// @note Member visibility is set to private after the macro expansion.
#define MUSTARD_UTILITY_DECLARE_NONVIRTUAL_PROTECTED_DEFAULT_DESTRUCTOR_IN_DEFAULT_MOVEONLY(ClassName) \
protected:                                                                                             \
    constexpr ~ClassName() = default;                                                                  \
                                                                                                       \
public:                                                                                                \
    constexpr ClassName(ClassName&&) noexcept = default;                                               \
    constexpr auto operator=(ClassName&&) noexcept -> ClassName& = default;                            \
                                                                                                       \
private:

} // namespace Mustard::inline Utility
