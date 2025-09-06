// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

#include "Mustard/Concept/InputVector.h++"
#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Detector/Field/MagneticFieldBase.h++"
#include "Mustard/Utility/VectorCast.h++"

#include "Eigen/Core"
#include "Eigen/Geometry"

#include <cmath>

namespace Mustard::Detector::Field {

/// @brief Inner magnetic field of a perfect toroid.
class ToroidField : public MagneticFieldBase<ToroidField> {
public:
    /// @brief The constructor.
    /// @tparam T1 type of `x0` satisfies `Concept::InputVector3D`
    /// @tparam T2 type of `n` satisfies `Concept::InputVector3D`
    /// @param b magnetic flux density at r=r0
    /// @param r0 toroid big radius
    /// @param x0 toroid center position
    /// @param n toroid normal direction (the magnetic field is counterclockwisely around this direction)
    template<Concept::InputVector3D T1 = Eigen::Vector3d, Concept::InputVector3D T2 = Eigen::Vector3d>
    ToroidField(double b, double r0, T1 x0, T2 n);

    template<Concept::NumericVector3D T>
    auto B(T x) const -> T;

private:
    double fB;
    double fR0;
    Eigen::Transform<double, 3, Eigen::Isometry> fTransform;
};

} // namespace Mustard::Detector::Field

#include "Mustard/Detector/Field/ToroidField.inl"
