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

#include "Mustard/Concept/MathVector.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Math/GeometryRepresentation.h++"
#include "Mustard/Parallel/ShmArray.h++"
#include "Mustard/ROOTX/ScopedIMT.h++"
#include "Mustard/Utility/FunctionAttribute.h++"
#include "Mustard/Utility/VectorCast.h++"
#include "Mustard/Utility/VectorDimension.h++"
#include "Mustard/Utility/ZeroVector.h++"

#include "ROOT/RDataFrame.hxx"

#include "gtl/vector.hpp"

#include "muc/algorithm"
#include "muc/array"
#include "muc/concepts"
#include "muc/functional"
#include "muc/numeric"

#include "fmt/format.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <functional>
#include <limits>
#include <string_view>
#include <type_traits>
#include <utility>

namespace Mustard::Detector::Field {

/// @brief A callable that returns its input field value unchanged.
/// Used as the default transformation (`ATransformation`) for `FieldMap3D`.
/// @see FieldMap3D
struct Identity {
    /// @brief Forward the field value without modification.
    /// @tparam T Field value type.
    /// @param t The field value to forward.
    /// @return The forwarded field value.
    template<typename T>
    [[nodiscard]] MUSTARD_ALWAYS_INLINE auto operator()(Point3D, T&& t) const -> T&& {
        return std::forward<T>(t);
    }
};

/// @brief Interpolate a scalar or vector field defined on a 3-dimensional
/// regular grid.
///
/// Trilinear interpolation is performed on a regular grid whose data is loaded
/// from a ROOT RDataFrame node. The RNode must contain two columns: `"grid"`
/// (type `muc::array3f`) for the spatial coordinates and `"field"` (type
/// `GridValueType`) for the field values. The grid must be regular: points must
/// be uniformly spaced along each axis, and the number of points along each
/// axis must be consistent.
///
/// The input coordinates are first transformed by the projection functor
/// @p AProjection, then the interpolated field value is transformed by the
/// transformation functor @p ATransformation. This two-stage design enables symmetry
/// operations (e.g., mirror symmetry) and unit conversions without modifying
/// the grid data.
///
/// The grid data is stored in a `ShmArray`, making it suitable for shared-memory
/// parallel processing.
///
/// @tparam T Field value type. For scalar fields, use a floating-point
/// type (e.g. `double`); for vector fields, use a math vector
/// type (e.g. `Eigen::Vector3d`).
/// @tparam AProjection Callable `(Point3D) -> Point3D` that projects
/// the query coordinates before interpolation. Default: `std::identity`
/// (no projection).
/// @tparam ATransformation Callable `(Point3D, T) -> T` that
/// transforms the interpolated field value. Default:
/// `Identity` (no transformation).
template<muc::general_arithmetic T,
         std::regular_invocable<Point3D> AProjection = std::identity,
         std::regular_invocable<Point3D, T> ATransformation = Identity>
    requires std::convertible_to<std::invoke_result_t<AProjection, Point3D>, Point3D> and
             std::convertible_to<std::invoke_result_t<ATransformation, Point3D, T>, T>
class FieldMap3D {
public:
    /// @brief Value type of the field.
    using ValueType = T;
    /// @brief Coordinate type (always `double`).
    using CoordinateType = double;

public:
    /// @brief Construct from a ROOT file and RNTuple/TTree name.
    ///
    /// When MPI is available and @c worldComm has more than one process, only
    /// world rank 0 reads the ROOT file and validates the grid; grid data is
    /// distributed to other processes via shared memory (@ref ShmArray with
    /// @ref BcastTag) and grid metadata is broadcast via @c worldComm.
    ///
    /// The RNTuple/TTree must contain @c "grid" (@c muc::array3f) and
    /// @c "field" (@c GridValueType) columns.
    ///
    /// @param dataName Name of the RNTuple/TTree in the file.
    /// @param fileName Path to the ROOT file.
    /// @param projection Projection functor applied to query coordinates before
    /// interpolation. Default: identity (no projection).
    /// @param transformation Transformation functor applied to the interpolated
    /// field value. Default: identity (no transformation).
    /// @param tol Tolerance configuration passed to @c muc::isclose for the grid
    /// regularity check (delta consistency along each axis). The field
    /// @c tol.rel is also used to guard against excessively fine grids
    /// (@c grid.n must be less than @c 1/tol.rel).
    /// Default: @c tolerance<float>{} .
    /// @param enableIMT If @c true (default), enable ROOT Implicit Multi-Threading
    /// during RDataFrame reading via @c ScopedIMT. If @c false,
    /// skip the @c ScopedIMT guard and use whatever IMT state is
    /// already active.
    ///
    /// @throws std::runtime_error If the grid is irregular or the data
    /// format is invalid.
    FieldMap3D(std::string_view dataName, std::string_view fileName,
               AProjection projection = {}, ATransformation transformation = {},
               muc::tolerance<float> tol = {}, bool enableIMT = true);

    /// @brief Interpolate the field value at the given coordinates.
    ///
    /// Performs trilinear interpolation on the regular grid. The query
    /// coordinates are first projected via @p AProjection. If the projected point
    /// lies outside the grid, a default-constructed value of @p T is returned.
    ///
    /// @param position The query position as a 3D vector.
    /// @return The interpolated field value, or a default-constructed `T` if
    /// the point is outside the grid.
    [[nodiscard]] auto At(Point3D position) const -> T;

private:
    /// @brief Access the field value at grid index `(i, j, k)`.
    ///
    /// The grid is stored as a flat 1D array in row-major order:
    /// `index = (i * Ny + j) * Nz + k`.
    ///
    /// @param i Grid index along the x-axis.
    /// @param j Grid index along the y-axis.
    /// @param k Grid index along the z-axis.
    /// @return The field value at the given grid point.
    [[nodiscard]] auto FieldGrid(int i, int j, int k) const -> T;

private:
    /// @brief Metadata for one axis of the grid.
    struct GridInfo {
        long long n;  ///< Number of grid points along this axis.
        double min;   ///< Minimum coordinate value along this axis.
        double max;   ///< Maximum coordinate value along this axis.
        double delta; ///< Spacing between adjacent grid points.
    };

    /// @brief The storage type for grid data.
    ///
    /// Scalar fields are stored as `float`; vector fields are stored as
    /// `muc::arrayf<N>` where `N` is the vector dimension.
    using GridValueType = std::conditional_t<
        std::floating_point<T>,
        float,
        std::conditional_t<
            Concept::MathVectorFloatingPoint<T>,
            muc::arrayf<VectorDimension<T>{}>,
            void>>;

private:
    [[no_unique_address]] AProjection fProjection;         ///< Coordinate projection functor.
    [[no_unique_address]] ATransformation fTransformation; ///< Field value transformation functor.

    std::array<GridInfo, 3> fGridInfo;            ///< Grid metadata for x, y, z axes.
    Parallel::ShmArray<GridValueType> fFieldGrid; ///< Grid data in shared memory, stored as a flat array.

    mutable Point3D fCachedProjPos;     ///< Cached projected position for interpolation cache.
    mutable T fCachedInterpolatedValue; ///< Cached trilinear interpolation result (before transformation).
};

} // namespace Mustard::Detector::Field

#include "Mustard/Detector/Field/FieldMap3D.inl"
