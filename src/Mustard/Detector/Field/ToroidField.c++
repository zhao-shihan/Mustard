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

#include "Mustard/Detector/Field/ToroidField.h++"

namespace Mustard::Detector::Field {

ToroidField::ToroidField(double b, double r0, Point3D x0, Vector3D n) :
    MagneticFieldBase<ToroidField>{},
    fB{b},
    fR0{r0},
    fCenter{x0},
    fRotation{},
    fInverseRotation{} {
    const Vector3D zHat{0, 0, 1};
    if (n.isParallel(zHat)) {
        return;
    }
    const auto axis{zHat.cross(n)};
    const auto angle{zHat.angle(n)};
    fRotation = AxisAngle{axis, angle};
    fInverseRotation = fRotation.inverse();
}

auto ToroidField::B(Point3D x) const -> Vector3D {
    const auto localX{fInverseRotation * (x - fCenter)};
    const auto alpha{fB * fR0 / localX.mag2()};
    const Vector3D localB{alpha * localX.z(), 0, -alpha * localX.x()};
    return fRotation * localB;
}

} // namespace Mustard::Detector::Field
