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

#include "Mustard/Detector/Field/ElectromagneticFieldBase.h++"
#include "Mustard/Detector/Field/FieldMap3D.h++"
#include "Mustard/Detector/Field/FieldMapSymmetry.h++"
#include "Mustard/Math/GeometryRepresentation.h++"
#include "Mustard/Math/Vector.h++"
#include "Mustard/Utility/FunctionAttribute.h++"
#include "Mustard/Utility/VectorCast.h++"

#include "CLHEP/Units/SystemOfUnits.h"

#include "Eigen/Core"

#include "muc/ceta_string"
#include "muc/functional"

#include <concepts>
#include <type_traits>

namespace Mustard::Detector::Field {

/// @brief A functional type converts 6D EM-field SI field value
/// to CLHEP unit system.
template<typename ATransformation = Identity>
struct BEFieldSI2CLHEP : ATransformation {
    using ATransformation::ATransformation;

    /// @brief Apply the transformation chain with SI-to-CLHEP unit conversion.
    ///
    /// Multiplies B components (0–2) by @c CLHEP::tesla and E components
    /// (3–5) by @c (CLHEP::volt / CLHEP::m), then applies the wrapped
    /// transformation.
    ///
    /// @tparam T A 6D numeric vector type.
    /// @param x The position (passed through to the wrapped transformation).
    /// @param f The 6D field value @c [Bx, By, Bz, Ex, Ey, Ez] in SI units.
    /// @return The transformed 6D field value in CLHEP units.
    template<Concept::NumericVector<double, 6> T>
    [[nodiscard]] MUSTARD_ALWAYS_INLINE auto operator()(Point3D x, T f) const -> T {
        using namespace CLHEP;
        return static_cast<const ATransformation&>(*this)(
            x, T{f[0] * tesla, f[1] * tesla, f[2] * tesla,
                 f[3] * (volt / m), f[4] * (volt / m), f[5] * (volt / m)});
    }
};

/// @brief An electromagnetic field interpolated from data.
/// Initialization and interpolation are performed by `FieldMap3D`,
/// which stores 6-component field vectors (Bx, By, Bz, Ex, Ey, Ez).
/// Call @c BE(x) to get both field components from a single interpolation.
/// @tparam AProjection Callable `(Point3D) -> Point3D` that projects the query coordinates before interpolation.
/// @tparam ATransformation Callable `(Point3D, Eigen::Vector<double, 6>) -> Eigen::Vector<double, 6>` that transforms the interpolated field value.
template<std::regular_invocable<Point3D> AProjection = std::identity,
         std::regular_invocable<Point3D, Eigen::Vector<double, 6>> ATransformation = Identity>
    requires std::convertible_to<std::invoke_result_t<AProjection, Point3D>, Point3D> and
                 std::convertible_to<std::invoke_result_t<ATransformation, Point3D, Eigen::Vector<double, 6>>, Eigen::Vector<double, 6>>
class ElectromagneticFieldMap : public ElectromagneticFieldBase<ElectromagneticFieldMap<AProjection, ATransformation>>,
                                public FieldMap3D<Eigen::Vector<double, 6>, AProjection, BEFieldSI2CLHEP<ATransformation>> {
public:
    using typename ElectromagneticFieldBase<ElectromagneticFieldMap<AProjection, ATransformation>>::BEField;

public:
    /// @copydoc FieldMap3D::FieldMap3D
    using FieldMap3D<Eigen::Vector<double, 6>, AProjection, BEFieldSI2CLHEP<ATransformation>>::FieldMap3D;

    /// @brief Evaluate the magnetic field at a point.
    ///
    /// Extracts components 0–2 from the interpolated 6D field vector.
    ///
    /// @param x The position at which to evaluate the field.
    /// @return The interpolated magnetic field vector in CLHEP units.
    auto B(Point3D x) const -> Vector3D;
    /// @brief Evaluate the electric field at a point.
    ///
    /// Extracts components 3–5 from the interpolated 6D field vector.
    ///
    /// @param x The position at which to evaluate the field.
    /// @return The interpolated electric field vector in CLHEP units.
    auto E(Point3D x) const -> Vector3D;
    /// @brief Evaluate both field components at a point.
    ///
    /// Performs a single trilinear interpolation and splits the 6D result
    /// into B and E vectors. This is more efficient than calling @c B()
    /// and @c E() separately.
    ///
    /// @param x The position at which to evaluate the field.
    /// @return A @c BEField containing both the interpolated B and E vectors
    /// in CLHEP units.
    ///
    /// @warning Calling @c B() and @c E() separately performs two independent
    /// trilinear interpolations. Prefer @c BE() when both components
    /// are needed at the same position.
    auto BE(Point3D x) const -> BEField;
};

/// @brief A mirror symmetry electromagnetic field interpolated from data.
/// @tparam AAxis Axis specifier: `"X"`, `"Y"`, `"Z"`, `"XY"`, `"XZ"`, `"YZ"`, or `"XYZ"`.
template<muc::ceta_string AAxis>
using ElectromagneticFieldMapSymmetry = ElectromagneticFieldMap<CoordinateSymmetry<AAxis>, FieldSymmetry<AAxis>>;

} // namespace Mustard::Detector::Field

#include "Mustard/Detector/Field/ElectromagneticFieldMap.inl"
