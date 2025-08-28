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

#include "Mustard/Concept/MathVector.h++"
#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Detector/Field/FieldMapSymmetry.h++"
#include "Mustard/Detector/Field/MagneticFieldBase.h++"
#include "Mustard/Utility/FunctionAttribute.h++"
#include "Mustard/Utility/VectorCast.h++"

#include "CLHEP/Units/SystemOfUnits.h"

#include "Eigen/Core"

#include "EFM/FieldMap3D.h++"

#include "muc/functional"

namespace Mustard::Detector::Field {

/// @brief A functional type converts B-field SI field value
/// to CLHEP unit system. Use in EFM template parameter.
template<typename ATransformation = EFM::Identity>
struct BFieldSI2CLHEP : ATransformation {
    using ATransformation::ATransformation;

    template<Concept::MathVector3D T>
    [[nodiscard]] MUSTARD_ALWAYS_INLINE constexpr auto operator()(double x, double y, double z, T B) const noexcept -> T {
        return static_cast<const ATransformation&>(*this)(x, y, z, T{B * CLHEP::tesla});
    }
};

/// @brief An magnetic field interpolated from data.
/// Initialization and interpolation are performed by `AFieldMap`.
/// @tparam AFieldMap A field map type, e.g. `EFM::FieldMap3D<Eigen::Vector3d>`
template<typename AFieldMap = EFM::FieldMap3D<Eigen::Vector3d, double, muc::multidentity, BFieldSI2CLHEP<>>>
    requires std::same_as<typename AFieldMap::CoordinateType, double>
class MagneticFieldMap : public MagneticFieldBase<MagneticFieldMap<AFieldMap>>,
                         public AFieldMap {
public:
    using AFieldMap::AFieldMap;

    template<Concept::NumericVector3D T>
    auto B(T x) const -> T { return VectorCast<T>((*this)(x[0], x[1], x[2])); }
};

/// @brief An YZ plane mirror symmetry magnetic field interpolated from data.
template<Concept::MathVector3D T = Eigen::Vector3d>
using MagneticFieldMapSymmetryX = MagneticFieldMap<
    EFM::FieldMap3D<T, double, CoordinateSymmetryX, BFieldSI2CLHEP<FieldSymmetryX>>>;

/// @brief An ZX plane mirror symmetry magnetic field interpolated from data.
template<Concept::MathVector3D T = Eigen::Vector3d>
using MagneticFieldMapSymmetryY = MagneticFieldMap<
    EFM::FieldMap3D<T, double, CoordinateSymmetryY, BFieldSI2CLHEP<FieldSymmetryY>>>;

/// @brief An XY plane mirror symmetry magnetic field interpolated from data.
template<Concept::MathVector3D T = Eigen::Vector3d>
using MagneticFieldMapSymmetryZ = MagneticFieldMap<
    EFM::FieldMap3D<T, double, CoordinateSymmetryZ, BFieldSI2CLHEP<FieldSymmetryZ>>>;

/// @brief An YZ, ZX plane mirror symmetry magnetic field interpolated from data.
template<Concept::MathVector3D T = Eigen::Vector3d>
using MagneticFieldMapSymmetryXY = MagneticFieldMap<
    EFM::FieldMap3D<T, double, CoordinateSymmetryXY, BFieldSI2CLHEP<FieldSymmetryXY>>>;

/// @brief An XY, YZ plane mirror symmetry magnetic field interpolated from data.
template<Concept::MathVector3D T = Eigen::Vector3d>
using MagneticFieldMapSymmetryXZ = MagneticFieldMap<
    EFM::FieldMap3D<T, double, CoordinateSymmetryXZ, BFieldSI2CLHEP<FieldSymmetryXZ>>>;

/// @brief An ZX, XY plane mirror symmetry magnetic field interpolated from data.
template<Concept::MathVector3D T = Eigen::Vector3d>
using MagneticFieldMapSymmetryYZ = MagneticFieldMap<
    EFM::FieldMap3D<T, double, CoordinateSymmetryYZ, BFieldSI2CLHEP<FieldSymmetryYZ>>>;

/// @brief An XY, YZ, ZX plane mirror symmetry magnetic field interpolated from data.
template<Concept::MathVector3D T = Eigen::Vector3d>
using MagneticFieldMapSymmetryXYZ = MagneticFieldMap<
    EFM::FieldMap3D<T, double, CoordinateSymmetryXYZ, BFieldSI2CLHEP<FieldSymmetryXYZ>>>;

} // namespace Mustard::Detector::Field
