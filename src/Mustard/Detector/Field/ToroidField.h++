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
class ToroidField : public MagneticFieldBase<ToroidField> {
public:
    /// @brief The constructor.
    /// @param b magnetic flux density at r=r0
    /// @param r0 toroid big radius
    /// @param x0 toroid center position
    /// @param n toroid normal direction (the magnetic field is counterclockwisely around this direction)
    ToroidField(double b, double r0, Point3D x0, Vector3D n);

    auto B(Point3D x) const -> Vector3D;

private:
    double fB;
    double fR0;
    Point3D fCenter;
    Rotation fRotation;
    Rotation fInverseRotation;
};

} // namespace Mustard::Detector::Field
