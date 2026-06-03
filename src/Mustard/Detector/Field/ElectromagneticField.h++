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

#include "Mustard/Math/GeometryRepresentation.h++"
#include "Mustard/Math/Vector.h++"

#include <concepts>

namespace Mustard::Detector::Field {

/// @brief Concept constraining a type to provide both electric and magnetic
/// field queries.
///
/// Requires four expressions to be valid:
/// - @c f.B(position) returns @c Vector3D (magnetic field at a point)
/// - @c f.E(position) returns @c Vector3D (electric field at a point)
/// - @c f.BE(position).B returns @c Vector3D&& (combined B component)
/// - @c f.BE(position).E returns @c Vector3D&& (combined E component)
///
/// @tparam F Candidate type to check against the concept.
template<typename F>
concept ElectromagneticField = requires(const F f, Point3D position) {
    { f.B(position) } -> std::same_as<Vector3D>;
    { f.E(position) } -> std::same_as<Vector3D>;
    { f.BE(position).B } -> std::same_as<Vector3D&&>;
    { f.BE(position).E } -> std::same_as<Vector3D&&>;
};

} // namespace Mustard::Detector::Field
