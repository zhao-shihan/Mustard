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
#include "Mustard/Math/Vector.h++"

namespace Mustard::Detector::Field {

/// @brief A spatially uniform (constant) magnetic field.
///
/// The magnetic field vector is the same at every point in space.
/// Inherits from @c MagneticFieldBase<UniformMagneticField>, so the
/// electric field component is automatically zero.
class UniformMagneticField : public MagneticFieldBase<UniformMagneticField> {
public:
    /// @brief Construct from three scalar components.
    /// @param bx Magnetic field x-component.
    /// @param by Magnetic field y-component.
    /// @param bz Magnetic field z-component.
    UniformMagneticField(double bx, double by, double bz);
    /// @brief Construct from a vector.
    /// @param b Magnetic field vector.
    UniformMagneticField(Vector3D b);

    /// @brief Returns the constant magnetic field.
    /// @note The position parameter is unused; it exists for interface uniformity.
    /// @return The constant magnetic field vector.
    auto B(Point3D) const -> Vector3D { return {fBx, fBy, fBz}; }

private:
    double fBx; ///< Magnetic field x-component.
    double fBy; ///< Magnetic field y-component.
    double fBz; ///< Magnetic field z-component.
};

} // namespace Mustard::Detector::Field
