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
#include "Mustard/Detector/Field/ElectromagneticFieldBase.h++"
#include "Mustard/Detector/Field/FieldMapSymmetry.h++"
#include "Mustard/Utility/InlineMacro.h++"
#include "Mustard/Utility/VectorCast.h++"

#include "EFM/FieldMap3D.h++"

#include "CLHEP/Units/SystemOfUnits.h"

#include "Eigen/Core"

#include "muc/ceta_string"
#include "muc/functional"

#include <concepts>
#include <optional>

namespace Mustard::Detector::Field {

/// @brief A functional type converts 6D EM-field SI field value
/// to CLHEP unit system. Use in EFM template parameter.
template<typename ATransformation = EFM::Identity>
struct BEFieldSI2CLHEP : ATransformation {
    using ATransformation::ATransformation;

    template<Concept::NumericVector<double, 6> T>
    [[nodiscard]] MUSTARD_ALWAYS_INLINE constexpr auto operator()(T f) const noexcept -> T {
        using namespace CLHEP;
        return static_cast<const ATransformation&>(*this)(
            T{f[0] * tesla, f[1] * tesla, f[2] * tesla,
              f[3] * (volt / m), f[4] * (volt / m), f[5] * (volt / m)});
    }
};

/// @brief An electromagnetic field interpolated from data.
/// Initialization and interpolation are performed by `AFieldMap`.
/// @tparam ACache A string literal, can be "WithCache" or "NoCache".
/// @tparam AFieldMap A field map type, e.g. `EFM::FieldMap3D<T>` or
/// `EFM::FieldMap3DSymZ<Eigen::Vector<double, .
/// @note "WithCache" and "NoCache" decides whether field cache will be used.
/// "WithCache" field will reuse the field value calculated in last calculation if
/// this calculation happens exactly at the same position. In principle, "WithCache"
/// accelerates cases when there are many subsequent calls to E or B or EB with same x:
///     Something(field.E(x), field.B(x));
/// However, if these cases do not matter or you need maximum performace in EB then
/// "NoCache" would be better.
template<muc::ceta_string ACache = "WithCache", Concept::MathVector<double, 6> T = Eigen::Vector<double, 6>,
         typename AFieldMap = EFM::FieldMap3D<T, double, muc::multidentity, BEFieldSI2CLHEP>>
    requires((ACache == "WithCache" or == "NoCache") and
             std::same_as<typename AFieldMap::CoordinateType, double>)
class ElectromagneticFieldMap;

template<typename AFieldMap>
class ElectromagneticFieldMap<"WithCache", AFieldMap> : public ElectromagneticFieldBase<ElectromagneticFieldMap<"WithCache", AFieldMap>>,
                                                        public AFieldMap {
private:
    template<Concept::NumericVector3D T>
    using F = typename ElectromagneticFieldBase<ElectromagneticFieldMap<"WithCache", AFieldMap>>::template F<T>;

public:
    using AFieldMap::AFieldMap;

    template<Concept::NumericVector3D T>
    auto B(T x) const -> T;
    template<Concept::NumericVector3D T>
    auto E(T x) const -> T;
    template<Concept::NumericVector3D T>
    auto BE(T x) const -> F<T>;

private:
    mutable Eigen::Vector3d fCachedX;
    mutable std::optional<typename AFieldMap::ValueType> fCache;
};

template<typename AFieldMap>
class ElectromagneticFieldMap<"NoCache", AFieldMap> : public ElectromagneticFieldBase<ElectromagneticFieldMap<"NoCache", AFieldMap>>,
                                                      public AFieldMap {
private:
    template<Concept::NumericVector3D T>
    using F = typename ElectromagneticFieldBase<ElectromagneticFieldMap<"NoCache", AFieldMap>>::template F<T>;

public:
    using AFieldMap::AFieldMap;

    template<Concept::NumericVector3D T>
    auto B(T x) const -> T;
    template<Concept::NumericVector3D T>
    auto E(T x) const -> T;
    template<Concept::NumericVector3D T>
    auto BE(T x) const -> F<T>;
};

/// @brief An YZ plane mirror symmetry electromagnetic field interpolated from data.
/// @tparam ACache Use cache or not. See `ElectromagneticFieldMap`.
template<muc::ceta_string ACache = "WithCache", Concept::MathVector<double, 6> T = Eigen::Vector<double, 6>>
using ElectromagneticFieldMapSymmetryX = ElectromagneticFieldMap<
    ACache, EFM::FieldMap3D<T, double, CoordinateSymmetryX, BEFieldSI2CLHEP<FieldSymmetryX>>>;

/// @brief An ZX plane mirror symmetry electromagnetic field interpolated from data.
/// @tparam ACache Use cache or not. See `ElectromagneticFieldMap`.
template<muc::ceta_string ACache = "WithCache", Concept::MathVector<double, 6> T = Eigen::Vector<double, 6>>
using ElectromagneticFieldMapSymmetryY = ElectromagneticFieldMap<
    ACache, EFM::FieldMap3D<T, double, CoordinateSymmetryY, BEFieldSI2CLHEP<FieldSymmetryY>>>;

/// @brief An XY plane mirror symmetry electromagnetic field interpolated from data.
/// @tparam ACache Use cache or not. See `ElectromagneticFieldMap`.
template<muc::ceta_string ACache = "WithCache", Concept::MathVector<double, 6> T = Eigen::Vector<double, 6>>
using ElectromagneticFieldMapSymmetryZ = ElectromagneticFieldMap<
    ACache, EFM::FieldMap3D<T, double, CoordinateSymmetryZ, BEFieldSI2CLHEP<FieldSymmetryZ>>>;

/// @brief An YZ, ZX plane mirror symmetry electromagnetic field interpolated from data.
/// @tparam ACache Use cache or not. See `ElectromagneticFieldMap`.
template<muc::ceta_string ACache = "WithCache", Concept::MathVector<double, 6> T = Eigen::Vector<double, 6>>
using ElectromagneticFieldMapSymmetryXY = ElectromagneticFieldMap<
    ACache, EFM::FieldMap3D<T, double, CoordinateSymmetryXY, BEFieldSI2CLHEP<FieldSymmetryXY>>>;

/// @brief An XY, YZ plane mirror symmetry electromagnetic field interpolated from data.
/// @tparam ACache Use cache or not. See `ElectromagneticFieldMap`.
template<muc::ceta_string ACache = "WithCache", Concept::MathVector<double, 6> T = Eigen::Vector<double, 6>>
using ElectromagneticFieldMapSymmetryXZ = ElectromagneticFieldMap<
    ACache, EFM::FieldMap3D<T, double, CoordinateSymmetryXZ, BEFieldSI2CLHEP<FieldSymmetryXZ>>>;

/// @brief An ZX, XY plane mirror symmetry electromagnetic field interpolated from data.
/// @tparam ACache Use cache or not. See `ElectromagneticFieldMap`.
template<muc::ceta_string ACache = "WithCache", Concept::MathVector<double, 6> T = Eigen::Vector<double, 6>>
using ElectromagneticFieldMapSymmetryYZ = ElectromagneticFieldMap<
    ACache, EFM::FieldMap3D<T, double, CoordinateSymmetryYZ, BEFieldSI2CLHEP<FieldSymmetryYZ>>>;

/// @brief An XY, YZ, ZX plane mirror symmetry electromagnetic field interpolated from data.
/// @tparam ACache Use cache or not. See `ElectromagneticFieldMap`.
template<muc::ceta_string ACache = "WithCache", Concept::MathVector<double, 6> T = Eigen::Vector<double, 6>>
using ElectromagneticFieldMapSymmetryXYZ = ElectromagneticFieldMap<
    ACache, EFM::FieldMap3D<T, double, CoordinateSymmetryXYZ, BEFieldSI2CLHEP<FieldSymmetryXYZ>>>;

} // namespace Mustard::Detector::Field

#include "Mustard/Detector/Field/ElectromagneticFieldMap.inl"
