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

namespace Mustard::Detector::Field {

template<std::regular_invocable<Point3D> AProjection,
         std::regular_invocable<Point3D, Eigen::Vector<double, 6>> ATransformation>
    requires std::convertible_to<std::invoke_result_t<AProjection, Point3D>, Point3D> and
             std::convertible_to<std::invoke_result_t<ATransformation, Point3D, Eigen::Vector<double, 6>>, Eigen::Vector<double, 6>>
auto ElectromagneticFieldMap<AProjection, ATransformation>::B(Point3D x) const -> Vector3D {
    const auto f{this->At(x)};
    return {f[0], f[1], f[2]};
}

template<std::regular_invocable<Point3D> AProjection,
         std::regular_invocable<Point3D, Eigen::Vector<double, 6>> ATransformation>
    requires std::convertible_to<std::invoke_result_t<AProjection, Point3D>, Point3D> and
             std::convertible_to<std::invoke_result_t<ATransformation, Point3D, Eigen::Vector<double, 6>>, Eigen::Vector<double, 6>>
auto ElectromagneticFieldMap<AProjection, ATransformation>::E(Point3D x) const -> Vector3D {
    const auto f{this->At(x)};
    return {f[3], f[4], f[5]};
}

template<std::regular_invocable<Point3D> AProjection,
         std::regular_invocable<Point3D, Eigen::Vector<double, 6>> ATransformation>
    requires std::convertible_to<std::invoke_result_t<AProjection, Point3D>, Point3D> and
             std::convertible_to<std::invoke_result_t<ATransformation, Point3D, Eigen::Vector<double, 6>>, Eigen::Vector<double, 6>>
auto ElectromagneticFieldMap<AProjection, ATransformation>::BE(Point3D x) const -> BEField {
    const auto v{this->At(x)}; // clang-format off
    return {{v[0], v[1], v[2]}, {v[3], v[4], v[5]}}; // clang-format on
}

} // namespace Mustard::Detector::Field
