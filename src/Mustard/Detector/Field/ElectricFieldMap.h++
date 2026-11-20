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

#include "Mustard/Detector/Field/ElectricFieldBase.h++"
#include "Mustard/Detector/Field/FieldMap3D.h++"
#include "Mustard/Detector/Field/FieldMapSymmetry.h++"
#include "Mustard/Math/GeometryRepresentation.h++"
#include "Mustard/Math/Vector.h++"
#include "Mustard/Utility/FunctionAttribute.h++"

#include "CLHEP/Units/SystemOfUnits.h"

#include "Eigen/Core"

#include "muc/ceta_string"
#include "muc/functional"

#include <concepts>
#include <type_traits>

namespace Mustard::Detector::Field {

/// @brief A functional type converts E-field SI field value
/// to CLHEP unit system.
template<typename ATransformation = Identity>
struct EFieldSI2CLHEP : ATransformation {
    using ATransformation::ATransformation;

    /// @brief Apply the transformation chain with SI-to-CLHEP unit conversion.
    ///
    /// Multiplies the field value by @c (CLHEP::volt / CLHEP::m) to convert
    /// from SI (V/m) to CLHEP internal units, then applies the wrapped
    /// transformation.
    ///
    /// @tparam T A 3D math vector type.
    /// @param x The position (passed through to the wrapped transformation).
    /// @param E The E-field value in SI units.
    /// @return The transformed E-field value in CLHEP units.
    template<Concept::MathVector3D T>
    [[nodiscard]] MUSTARD_ALWAYS_INLINE auto operator()(Point3D x, T E) const -> T {
        return static_cast<const ATransformation&>(*this)(x, T{E * (CLHEP::volt / CLHEP::m)});
    }
};

/// @brief An electric field interpolated from data.
/// Initialization and interpolation are performed by `FieldMap3D`.
/// @tparam AProjection Callable `(Point3D) -> Point3D` that projects the query coordinates before interpolation.
/// @tparam ATransformation Callable `(Point3D, Vector3D) -> Vector3D` that transforms the interpolated field value.
template<std::regular_invocable<Point3D> AProjection = std::identity,
         std::regular_invocable<Point3D, Vector3D> ATransformation = Identity>
    requires std::convertible_to<std::invoke_result_t<AProjection, Point3D>, Point3D> and
                 std::convertible_to<std::invoke_result_t<ATransformation, Point3D, Vector3D>, Vector3D>
class ElectricFieldMap : public ElectricFieldBase<ElectricFieldMap<AProjection, ATransformation>>,
                         public FieldMap3D<Vector3D, AProjection, EFieldSI2CLHEP<ATransformation>> {
public:
    /// @copydoc FieldMap3D::FieldMap3D
    using FieldMap3D<Vector3D, AProjection, EFieldSI2CLHEP<ATransformation>>::FieldMap3D;

    /// @brief Evaluate the electric field at a point via interpolation.
    ///
    /// Delegates to @c FieldMap3D::At() which performs trilinear interpolation
    /// on the grid data.
    ///
    /// @param x The position at which to evaluate the field.
    /// @return The interpolated electric field vector in CLHEP units.
    auto E(Point3D x) const -> Vector3D { return this->At(x); }
};

/// @brief A mirror symmetry electric field interpolated from data.
/// @tparam AAxis Axis specifier: `"X"`, `"Y"`, `"Z"`, `"XY"`, `"XZ"`, `"YZ"`, or `"XYZ"`.
template<muc::ceta_string AAxis>
using ElectricFieldMapSymmetry = ElectricFieldMap<CoordinateSymmetry<AAxis>, FieldSymmetry<AAxis>>;

} // namespace Mustard::Detector::Field
