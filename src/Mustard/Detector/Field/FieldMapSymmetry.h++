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

#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Math/GeometryRepresentation.h++"
#include "Mustard/Utility/FunctionAttribute.h++"

#include "muc/array"
#include "muc/ceta_string"
#include "muc/math"

#include <concepts>

namespace Mustard::Detector::Field {

/// @brief Mirror symmetry coordinate projection.
/// @tparam AAxis Axis specifier: `"X"`, `"Y"`, `"Z"`, `"XY"`, `"XZ"`, `"YZ"`, or `"XYZ"`.
template<muc::ceta_string AAxis>
    requires(AAxis == "X" or AAxis == "Y" or AAxis == "Z" or AAxis == "XY" or AAxis == "XZ" or AAxis == "YZ" or AAxis == "XYZ")
struct CoordinateSymmetry {
    /// @brief Fold the coordinate into the positive half-space.
    ///
    /// Maps the specified axis (or axes) through absolute value, effectively
    /// projecting coordinates into the all-positive octant. Multi-axis
    /// variants (e.g. @c "XY", @c "XYZ") recursively compose the single-axis
    /// operations.
    ///
    /// @param x The coordinate to project.
    /// @return The projected coordinate, with the designated axes non-negative.
    [[nodiscard]] MUSTARD_ALWAYS_INLINE auto operator()(Point3D x) const -> Point3D {
        if constexpr (AAxis == "X") {
            return {muc::abs(x[0]), x[1], x[2]};
        } else if constexpr (AAxis == "Y") {
            return {x[0], muc::abs(x[1]), x[2]};
        } else if constexpr (AAxis == "Z") {
            return {x[0], x[1], muc::abs(x[2])};
        } else if constexpr (AAxis == "XY") {
            return CoordinateSymmetry<"Y">{}(CoordinateSymmetry<"X">{}(x));
        } else if constexpr (AAxis == "XZ") {
            return CoordinateSymmetry<"Z">{}(CoordinateSymmetry<"X">{}(x));
        } else if constexpr (AAxis == "YZ") {
            return CoordinateSymmetry<"Z">{}(CoordinateSymmetry<"Y">{}(x));
        } else {
            return CoordinateSymmetry<"Z">{}(CoordinateSymmetry<"Y">{}(CoordinateSymmetry<"X">{}(x)));
        }
    }
};

/// @brief Mirror symmetry field component transformation.
/// @tparam AAxis Axis specifier: `"X"`, `"Y"`, `"Z"`, `"XY"`, `"XZ"`, `"YZ"`, or `"XYZ"`.
template<muc::ceta_string AAxis>
    requires(AAxis == "X" or AAxis == "Y" or AAxis == "Z" or AAxis == "XY" or AAxis == "XZ" or AAxis == "YZ" or AAxis == "XYZ")
struct FieldSymmetry {
    /// @brief Apply mirror symmetry sign flip to field components.
    ///
    /// In the negative half-space (determined by @c AAxis), the field
    /// component perpendicular to the mirror plane is flipped in sign.
    /// For 6D vectors (EM fields), the corresponding B and E components
    /// are flipped together.
    ///
    /// @tparam T Field vector type (3D for B or E alone, 6D for combined EM).
    /// @param x The coordinate (determines which half-space the point is in).
    /// @param f The field value to transform.
    /// @return The transformed field value with sign flips applied.
    ///
    /// @note For 6D vectors, components 0–2 are interpreted as B and 3–5 as
    /// E. Under mirror symmetry, both the B pseudovector component
    /// normal to the mirror plane and the corresponding E component
    /// (for the same coordinate axis) flip sign together.
    template<typename T>
        requires Concept::NumericVector<T, double, 3> or Concept::NumericVector<T, double, 6>
    [[nodiscard]] MUSTARD_ALWAYS_INLINE auto operator()(Point3D x, T f) const -> T {
        if constexpr (AAxis == "X") {
            if (x[0] >= 0) {
                return f;
            }
            f[0] = -f[0];
            if constexpr (Concept::NumericVector<T, double, 6>) {
                f[3] = -f[3];
            }
            return f;
        } else if constexpr (AAxis == "Y") {
            if (x[1] >= 0) {
                return f;
            }
            f[1] = -f[1];
            if constexpr (Concept::NumericVector<T, double, 6>) {
                f[4] = -f[4];
            }
            return f;
        } else if constexpr (AAxis == "Z") {
            if (x[2] >= 0) {
                return f;
            }
            f[2] = -f[2];
            if constexpr (Concept::NumericVector<T, double, 6>) {
                f[5] = -f[5];
            }
            return f;
        } else if constexpr (AAxis == "XY") {
            return FieldSymmetry<"Y">{}(x, FieldSymmetry<"X">{}(x, f));
        } else if constexpr (AAxis == "XZ") {
            return FieldSymmetry<"Z">{}(x, FieldSymmetry<"X">{}(x, f));
        } else if constexpr (AAxis == "YZ") {
            return FieldSymmetry<"Z">{}(x, FieldSymmetry<"Y">{}(x, f));
        } else {
            return FieldSymmetry<"Z">{}(x, FieldSymmetry<"Y">{}(x, FieldSymmetry<"X">{}(x, f)));
        }
    }
};

} // namespace Mustard::Detector::Field
