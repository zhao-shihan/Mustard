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

/// @brief A spatially uniform (constant) electric field.
///
/// The electric field vector is the same at every point in space.
/// Inherits from @c ElectricFieldBase<UniformElectricField>, so the
/// magnetic field component is automatically zero.
class UniformElectricField : public ElectricFieldBase<UniformElectricField> {
public:
    /// @brief Construct from three scalar components.
    /// @param ex Electric field x-component.
    /// @param ey Electric field y-component.
    /// @param ez Electric field z-component.
    UniformElectricField(double ex, double ey, double ez);
    /// @brief Construct from a vector.
    /// @param e Electric field vector.
    UniformElectricField(Vector3D e);

    /// @brief Returns the constant electric field.
    /// @param Unused. Position parameter for interface uniformity.
    /// @return The constant electric field vector.
    auto E(Point3D) const -> Vector3D { return {fEx, fEy, fEz}; }

private:
    double fEx; ///< Electric field x-component.
    double fEy; ///< Electric field y-component.
    double fEz; ///< Electric field z-component.
};

} // namespace Mustard::Detector::Field
