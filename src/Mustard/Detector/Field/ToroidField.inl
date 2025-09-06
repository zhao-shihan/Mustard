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

namespace Mustard::Detector::Field {

template<Concept::InputVector3D T1, Concept::InputVector3D T2>
ToroidField::ToroidField(double b, double r0, T1 x0, T2 n) :
    MagneticFieldBase<ToroidField>{},
    fB{b},
    fR0{r0},
    fTransform{} {
    const Eigen::Vector3d normal{VectorCast<Eigen::Vector3d>(n).normalized()}; // clang-format off
    const Eigen::Vector3d axis{Eigen::Vector3d{0, 0, 1}.cross(normal)};
    const double angle{std::acos(Eigen::Vector3d{0, 0, 1}.dot(normal))}; // clang-format on
    fTransform = (Eigen::Translation3d{VectorCast<Eigen::Vector3d>(x0)} * Eigen::AngleAxisd{angle, axis}).inverse();
}

template<Concept::NumericVector3D T>
auto ToroidField::B(T x) const -> T {
    const auto x0{fTransform * VectorCast<Eigen::Vector3d>(x)};
    const auto alpha{fB * fR0 / x0.squaredNorm()};
    return {alpha * x0.z(), 0, -alpha * x0.x()};
}

} // namespace Mustard::Detector::Field
