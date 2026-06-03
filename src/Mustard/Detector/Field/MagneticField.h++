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

#include "Mustard/Detector/Field/ElectromagneticField.h++"
#include "Mustard/Math/GeometryRepresentation.h++"
#include "Mustard/Math/Vector.h++"

#include <concepts>

namespace Mustard::Detector::Field {

/// @brief Concept constraining a type to a magnetic-field-only model.
///
/// Refines @c ElectromagneticField by additionally requiring that
/// @c F::E(position) is a valid static expression returning @c Vector3D.
/// This means the type provides only a magnetic field; the electric field
/// component is statically zero.
///
/// @tparam F Candidate type to check against the concept.
template<typename F>
concept MagneticField = requires(Point3D position) {
    requires ElectromagneticField<F>;
    { F::E(position) } -> std::same_as<Vector3D>;
};

} // namespace Mustard::Detector::Field
