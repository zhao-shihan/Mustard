// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
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
#include "Mustard/Detector/Field/ElectricFieldBase.h++"
#include "Mustard/Detector/Field/FieldMapSymmetry.h++"
#include "Mustard/Utility/InlineMacro.h++"
#include "Mustard/Utility/VectorCast.h++"

#include "EFM/FieldMap3D.h++"

#include "CLHEP/Units/SystemOfUnits.h"

#include "Eigen/Core"

#include "muc/functional"

namespace Mustard::Detector::Field {

/// @brief A functional type converts E-field SI field value
/// to CLHEP unit system. Use in EFM template parameter.
template<typename ATransformation = EFM::Identity>
struct EFieldSI2CLHEP : ATransformation {
    using ATransformation::ATransformation;

    template<Concept::MathVector3D T>
    [[nodiscard]] MUSTARD_ALWAYS_INLINE constexpr auto operator()(double x, double y, double z, T E) const noexcept -> T {
        return static_cast<const ATransformation&>(*this)(x, y, z, T{E * (CLHEP::volt / CLHEP::m)});
    }
};

/// @brief An electric field interpolated from data.
/// Initialization and interpolation are performed by `AFieldMap`.
/// @tparam AFieldMap A field map type, e.g. `EFM::FieldMap3D<Eigen::Vector3d>`
template<typename AFieldMap = EFM::FieldMap3D<Eigen::Vector3d, double, muc::multidentity, EFieldSI2CLHEP<>>>
    requires std::same_as<typename AFieldMap::CoordinateType, double>
class ElectricFieldMap : public ElectricFieldBase<ElectricFieldMap<AFieldMap>>,
                         public AFieldMap {
public:
    using AFieldMap::AFieldMap;

    template<Concept::NumericVector3D T>
    auto E(T x) const -> T { return VectorCast<T>((*this)(x[0], x[1], x[2])); }
};

/// @brief An YZ plane mirror symmetry electric field interpolated from data.
template<Concept::MathVector3D T = Eigen::Vector3d>
using ElectricFieldMapSymmetryX = ElectricFieldMap<
    EFM::FieldMap3D<T, double, CoordinateSymmetryX, EFieldSI2CLHEP<FieldSymmetryX>>>;

/// @brief An ZX plane mirror symmetry electric field interpolated from data.
template<Concept::MathVector3D T = Eigen::Vector3d>
using ElectricFieldMapSymmetryY = ElectricFieldMap<
    EFM::FieldMap3D<T, double, CoordinateSymmetryY, EFieldSI2CLHEP<FieldSymmetryY>>>;

/// @brief An XY plane mirror symmetry electric field interpolated from data.
template<Concept::MathVector3D T = Eigen::Vector3d>
using ElectricFieldMapSymmetryZ = ElectricFieldMap<
    EFM::FieldMap3D<T, double, CoordinateSymmetryZ, EFieldSI2CLHEP<FieldSymmetryZ>>>;

/// @brief An YZ, ZX plane mirror symmetry electric field interpolated from data.
template<Concept::MathVector3D T = Eigen::Vector3d>
using ElectricFieldMapSymmetryXY = ElectricFieldMap<
    EFM::FieldMap3D<T, double, CoordinateSymmetryXY, EFieldSI2CLHEP<FieldSymmetryXY>>>;

/// @brief An XY, YZ plane mirror symmetry electric field interpolated from data.
template<Concept::MathVector3D T = Eigen::Vector3d>
using ElectricFieldMapSymmetryXZ = ElectricFieldMap<
    EFM::FieldMap3D<T, double, CoordinateSymmetryXZ, EFieldSI2CLHEP<FieldSymmetryXZ>>>;

/// @brief An ZX, XY plane mirror symmetry electric field interpolated from data.
template<Concept::MathVector3D T = Eigen::Vector3d>
using ElectricFieldMapSymmetryYZ = ElectricFieldMap<
    EFM::FieldMap3D<T, double, CoordinateSymmetryYZ, EFieldSI2CLHEP<FieldSymmetryYZ>>>;

/// @brief An XY, YZ, ZX plane mirror symmetry electric field interpolated from data.
template<Concept::MathVector3D T = Eigen::Vector3d>
using ElectricFieldMapSymmetryXYZ = ElectricFieldMap<
    EFM::FieldMap3D<T, double, CoordinateSymmetryXYZ, EFieldSI2CLHEP<FieldSymmetryXYZ>>>;

} // namespace Mustard::Detector::Field
