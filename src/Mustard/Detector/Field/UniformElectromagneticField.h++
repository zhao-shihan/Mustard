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

class UniformElectromagneticField : public ElectromagneticFieldBase<UniformElectromagneticField> {
public:
    UniformElectromagneticField(double bx, double by, double bz,
                                double ex, double ey, double ez);
    UniformElectromagneticField(Vector3D b, Vector3D e);

    auto B(Point3D) const -> Vector3D { return {fBx, fBy, fBz}; }
    auto E(Point3D) const -> Vector3D { return {fEx, fEy, fEz}; }
    auto BE(Point3D x) const -> BEField { return {B(x), E(x)}; }

private:
    double fBx;
    double fBy;
    double fBz;
    double fEx;
    double fEy;
    double fEz;
};

} // namespace Mustard::Detector::Field
