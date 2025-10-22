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

#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Utility/FunctionAttribute.h++"

#include "muc/math"

#include <concepts>
#include <tuple>

namespace Mustard::Detector::Field {

/// @brief Mirror symmetry operation along x-axis. Flip the x-coordinate from
/// negative to positive.
struct CoordinateSymmetryX {
    template<std::floating_point C>
    [[nodiscard]] MUSTARD_ALWAYS_INLINE constexpr auto operator()(C x, C y, C z) const noexcept -> std::tuple<C, C, C> {
        return {muc::abs(x), y, z};
    }
};

/// @brief Mirror symmetry operation along x-axis. Flip the x-coordinate from
/// negative to positive.
struct FieldSymmetryX {
    template<std::floating_point C, typename T>
        requires Concept::NumericVector<T, C, 3> or Concept::NumericVector<T, C, 6>
    [[nodiscard]] MUSTARD_ALWAYS_INLINE constexpr auto operator()(C x, C, C, T f) const noexcept -> T {
        if (x >= 0) {
            return f;
        }
        f[0] = -f[0];
        if constexpr (Concept::NumericVector<T, C, 6>) {
            f[3] = -f[3];
        }
        return f;
    }
};

/// @brief Mirror symmetry operation along y-axis. Flip the y-coordinate from
/// negative to positive.
struct CoordinateSymmetryY {
    template<std::floating_point C>
    [[nodiscard]] MUSTARD_ALWAYS_INLINE constexpr auto operator()(C x, C y, C z) const noexcept -> std::tuple<C, C, C> {
        return {x, muc::abs(y), z};
    }
};

/// @brief Mirror symmetry operation along y-axis. Flip the y-coordinate from
/// negative to positive.
struct FieldSymmetryY {
    template<std::floating_point C, typename T>
        requires Concept::NumericVector<T, C, 3> or Concept::NumericVector<T, C, 6>
    [[nodiscard]] MUSTARD_ALWAYS_INLINE constexpr auto operator()(C, C y, C, T f) const noexcept -> T {
        if (y >= 0) {
            return f;
        }
        f[1] = -f[1];
        if constexpr (Concept::NumericVector<T, C, 6>) {
            f[4] = -f[4];
        }
        return f;
    }
};

/// @brief Mirror symmetry operation along z-axis. Flip the z-coordinate from
/// negative to positive.
struct CoordinateSymmetryZ {
    template<std::floating_point C>
    [[nodiscard]] MUSTARD_ALWAYS_INLINE constexpr auto operator()(C x, C y, C z) const noexcept -> std::tuple<C, C, C> {
        return {x, y, muc::abs(z)};
    }
};

/// @brief Mirror symmetry operation along z-axis. Flip the z-coordinate from
/// negative to positive.
struct FieldSymmetryZ {
    template<std::floating_point C, typename T>
        requires Concept::NumericVector<T, C, 3> or Concept::NumericVector<T, C, 6>
    [[nodiscard]] MUSTARD_ALWAYS_INLINE constexpr auto operator()(C, C, C z, T f) const noexcept -> T {
        if (z >= 0) {
            return f;
        }
        f[2] = -f[2];
        if constexpr (Concept::NumericVector<T, C, 6>) {
            f[5] = -f[5];
        }
        return f;
    }
};

/// @brief Mirror symmetry operation along x-axis and y-axis. Flip the
/// x-coordinate and y-coordinate from negative to positive.
struct CoordinateSymmetryXY {
    template<std::floating_point C>
    [[nodiscard]] MUSTARD_ALWAYS_INLINE constexpr auto operator()(C x, C y, C z) const noexcept -> std::tuple<C, C, C> {
        return std::apply(CoordinateSymmetryY{}, CoordinateSymmetryX{}(x, y, z));
    }
};

/// @brief Mirror symmetry operation along x-axis and y-axis. Flip the
/// x-coordinate and y-coordinate from negative to positive.
struct FieldSymmetryXY {
    template<std::floating_point C, typename T>
        requires Concept::NumericVector<T, C, 3> or Concept::NumericVector<T, C, 6>
    [[nodiscard]] MUSTARD_ALWAYS_INLINE constexpr auto operator()(C x, C y, C z, T f) const noexcept -> T {
        return FieldSymmetryY{}(x, y, z, FieldSymmetryX{}(x, y, z, f));
    }
};

/// @brief Mirror symmetry operation along x-axis and z-axis. Flip the
/// x-coordinate and z-coordinate from negative to positive.
struct CoordinateSymmetryXZ {
    template<std::floating_point C>
    [[nodiscard]] MUSTARD_ALWAYS_INLINE constexpr auto operator()(C x, C y, C z) const noexcept -> std::tuple<C, C, C> {
        return std::apply(CoordinateSymmetryZ{}, CoordinateSymmetryX{}(x, y, z));
    }
};

/// @brief Mirror symmetry operation along x-axis and z-axis. Flip the
/// x-coordinate and z-coordinate from negative to positive.
struct FieldSymmetryXZ {
    template<std::floating_point C, typename T>
        requires Concept::NumericVector<T, C, 3> or Concept::NumericVector<T, C, 6>
    [[nodiscard]] MUSTARD_ALWAYS_INLINE constexpr auto operator()(C x, C y, C z, T f) const noexcept -> T {
        return FieldSymmetryZ{}(x, y, z, FieldSymmetryX{}(x, y, z, f));
    }
};

/// @brief Mirror symmetry operation along y-axis and z-axis. Flip the
/// y-coordinate and z-coordinate from negative to positive.
struct CoordinateSymmetryYZ {
    template<std::floating_point C>
    [[nodiscard]] MUSTARD_ALWAYS_INLINE constexpr auto operator()(C x, C y, C z) const noexcept -> std::tuple<C, C, C> {
        return std::apply(CoordinateSymmetryZ{}, CoordinateSymmetryY{}(x, y, z));
    }
};

/// @brief Mirror symmetry operation along y-axis and z-axis. Flip the
/// y-coordinate and z-coordinate from negative to positive.
struct FieldSymmetryYZ {
    template<std::floating_point C, typename T>
        requires Concept::NumericVector<T, C, 3> or Concept::NumericVector<T, C, 6>
    [[nodiscard]] MUSTARD_ALWAYS_INLINE constexpr auto operator()(C x, C y, C z, T f) const noexcept -> T {
        return FieldSymmetryZ{}(x, y, z, FieldSymmetryY{}(x, y, z, f));
    }
};

/// @brief Mirror symmetry operation along x-axis, y-axis and z-axis. Flip the
/// x-coordinate, y-coordinate and z-coordinate from negative to positive.
struct CoordinateSymmetryXYZ {
    template<std::floating_point C>
    [[nodiscard]] MUSTARD_ALWAYS_INLINE constexpr auto operator()(C x, C y, C z) const noexcept -> std::tuple<C, C, C> {
        return std::apply(CoordinateSymmetryZ{}, std::apply(CoordinateSymmetryY{}, CoordinateSymmetryX{}(x, y, z)));
    }
};

/// @brief Mirror symmetry operation along x-axis, y-axis and z-axis. Flip the
/// x-coordinate, y-coordinate and z-coordinate from negative to positive.
struct FieldSymmetryXYZ {
    template<std::floating_point C, typename T>
        requires Concept::NumericVector<T, C, 3> or Concept::NumericVector<T, C, 6>
    [[nodiscard]] MUSTARD_ALWAYS_INLINE constexpr auto operator()(C x, C y, C z, T f) const noexcept -> T {
        return FieldSymmetryZ{}(x, y, z, FieldSymmetryY{}(x, y, z, FieldSymmetryX{}(x, y, z, f)));
    }
};

} // namespace Mustard::Detector::Field
