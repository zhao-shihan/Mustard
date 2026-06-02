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
#include "Mustard/Math/GeometryRepresentation.h++"
#include "Mustard/Math/Vector.h++"

namespace Mustard::Detector::Field {

class UniformElectricField : public ElectricFieldBase<UniformElectricField> {
public:
    UniformElectricField(double ex, double ey, double ez);
    UniformElectricField(Vector3D e);

    auto E(Point3D) const -> Vector3D { return {fEx, fEy, fEz}; }

private:
    double fEx;
    double fEy;
    double fEz;
};

} // namespace Mustard::Detector::Field
