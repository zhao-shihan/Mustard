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
#include "Mustard/Math/GeometryRepresentation.h++"
#include "Mustard/Math/Vector.h++"

namespace Mustard::Detector::Field {

/// @brief A spatially uniform (constant) electromagnetic field.
///
/// Both the electric and magnetic field vectors are constant throughout
/// space. Inherits directly from @c ElectromagneticFieldBase, implementing
/// all three query methods (@c B(), @c E(), @c BE()).
class UniformElectromagneticField : public ElectromagneticFieldBase<UniformElectromagneticField> {
public:
    /// @brief Construct from six scalar components.
    /// @param bx Magnetic field x-component.
    /// @param by Magnetic field y-component.
    /// @param bz Magnetic field z-component.
    /// @param ex Electric field x-component.
    /// @param ey Electric field y-component.
    /// @param ez Electric field z-component.
    UniformElectromagneticField(double bx, double by, double bz,
                                double ex, double ey, double ez);
    /// @brief Construct from two vectors.
    /// @param b Magnetic field vector.
    /// @param e Electric field vector.
    UniformElectromagneticField(Vector3D b, Vector3D e);

    /// @brief Returns the constant magnetic field.
    /// @param Unused. Position parameter for interface uniformity.
    /// @return The constant magnetic field vector.
    auto B(Point3D) const -> Vector3D { return {fBx, fBy, fBz}; }
    /// @brief Returns the constant electric field.
    /// @param Unused. Position parameter for interface uniformity.
    /// @return The constant electric field vector.
    auto E(Point3D) const -> Vector3D { return {fEx, fEy, fEz}; }
    /// @brief Returns both field components at a point.
    /// @param x The position (ignored for uniform fields).
    /// @return A @c BEField holding both the constant B and E vectors.
    auto BE(Point3D x) const -> BEField { return {B(x), E(x)}; }

private:
    double fBx; ///< Magnetic field x-component.
    double fBy; ///< Magnetic field y-component.
    double fBz; ///< Magnetic field z-component.
    double fEx; ///< Electric field x-component.
    double fEy; ///< Electric field y-component.
    double fEz; ///< Electric field z-component.
};

} // namespace Mustard::Detector::Field
