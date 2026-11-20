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

#include "Mustard/Detector/Field/MagneticFieldBase.h++"
#include "Mustard/Math/GeometryRepresentation.h++"
#include "Mustard/Math/LorentzRotation.h++"
#include "Mustard/Math/Vector.h++"

#include <cmath>

namespace Mustard::Detector::Field {

/// @brief Inner magnetic field of a perfect toroid.
///
/// Models the internal B field of an ideal toroidal magnet:
/// @f$ \mathbf{B} = \frac{B_0 R_0}{r^2} (z\,\hat{x} - x\,\hat{z}) @f$
/// in the local frame (where the normal is along @f$ \hat{z} @f$ and
/// @f$ r = \sqrt{x^2 + z^2} @f$). The field is rotated from the local
/// frame to the world frame defined by the normal vector @p n.
class ToroidField : public MagneticFieldBase<ToroidField> {
public:
    /// @brief The constructor.
    /// @param b Magnetic flux density at @f$ r = r_0 @f$.
    /// @param r0 Toroid big radius.
    /// @param x0 Toroid center position in world coordinates.
    /// @param n Toroid normal direction (the magnetic field circulates
    /// counterclockwise around this direction).
    ToroidField(double b, double r0, Point3D x0, Vector3D n);

    /// @brief Evaluate the toroid magnetic field at a point.
    ///
    /// Transforms the world-coordinate position to the toroid's local frame,
    /// computes the ideal toroid field, and rotates the result back to world
    /// coordinates.
    ///
    /// @param x Position in world coordinates.
    /// @return The magnetic field vector at @p x.
    auto B(Point3D x) const -> Vector3D;

private:
    double fB;                 ///< Nominal B-field magnitude at @c fR0.
    double fR0;                ///< Reference radius for the nominal field.
    Point3D fCenter;           ///< Toroid center position in world coordinates.
    Rotation fRotation;        ///< Rotation from local to world frame.
    Rotation fInverseRotation; ///< Inverse rotation from world to local frame.
};

} // namespace Mustard::Detector::Field
