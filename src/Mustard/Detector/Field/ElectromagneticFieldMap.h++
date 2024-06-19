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

#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Detector/Field/ElectromagneticFieldBase.h++"
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
struct BEFieldSI2CLHEP {
    template<Concept::NumericVector<double, 6> T>
    [[nodiscard]] constexpr auto operator()(T f) const -> T {
        using namespace CLHEP;
        return {f[0] * tesla, f[1] * tesla, f[2] * tesla,
                f[3] * (volt / m), f[4] * (volt / m), f[5] * (volt / m)};
    }
};

/// @brief An electromagnetic field interpolated from data.
/// Initialization and interpolation are performed by `AFieldMap`.
/// @tparam ACache A string literal, can be "WithCache" or "NoCache".
/// @tparam AFieldMap A field map type, e.g. `EFM::FieldMap3D<Eigen::Vector<double, 6>>` or
/// `EFM::FieldMap3DSymZ<Eigen::Vector<double, 6>>`.
/// @note "WithCache" and "NoCache" decides whether field cache will be used.
/// "WithCache" field will reuse the field value calculated in last calculation if
/// this calculation happens exactly at the same position. In principle, "WithCache"
/// accelerates cases when there are many subsequent calls to E or B or EB with same x:
///     Something(field.E(x), field.B(x));
/// However, if these cases do not matter or you need maximum performace in EB then
/// "NoCache" would be better.
template<muc::ceta_string ACache = "WithCache",
         typename AFieldMap = EFM::FieldMap3D<Eigen::Vector<double, 6>, double, muc::multidentity, BEFieldSI2CLHEP>>
    requires((ACache == "WithCache" or ACache == "NoCache") and
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
template<muc::ceta_string ACache = "WithCache">
using ElectromagneticFieldMapSymX = ElectromagneticFieldMap<ACache, EFM::FieldMap3DSymX<Eigen::Vector<double, 6>, double, BEFieldSI2CLHEP>>;

/// @brief An ZX plane mirror symmetry electromagnetic field interpolated from data.
/// @tparam ACache Use cache or not. See `ElectromagneticFieldMap`.
template<muc::ceta_string ACache = "WithCache">
using ElectromagneticFieldMapSymY = ElectromagneticFieldMap<ACache, EFM::FieldMap3DSymY<Eigen::Vector<double, 6>, double, BEFieldSI2CLHEP>>;

/// @brief An XY plane mirror symmetry electromagnetic field interpolated from data.
/// @tparam ACache Use cache or not. See `ElectromagneticFieldMap`.
template<muc::ceta_string ACache = "WithCache">
using ElectromagneticFieldMapSymZ = ElectromagneticFieldMap<ACache, EFM::FieldMap3DSymZ<Eigen::Vector<double, 6>, double, BEFieldSI2CLHEP>>;

/// @brief An YZ, ZX plane mirror symmetry electromagnetic field interpolated from data.
/// @tparam ACache Use cache or not. See `ElectromagneticFieldMap`.
template<muc::ceta_string ACache = "WithCache">
using ElectromagneticFieldMapSymXY = ElectromagneticFieldMap<ACache, EFM::FieldMap3DSymXY<Eigen::Vector<double, 6>, double, BEFieldSI2CLHEP>>;

/// @brief An XY, YZ plane mirror symmetry electromagnetic field interpolated from data.
/// @tparam ACache Use cache or not. See `ElectromagneticFieldMap`.
template<muc::ceta_string ACache = "WithCache">
using ElectromagneticFieldMapSymXZ = ElectromagneticFieldMap<ACache, EFM::FieldMap3DSymXZ<Eigen::Vector<double, 6>, double, BEFieldSI2CLHEP>>;

/// @brief An ZX, XY plane mirror symmetry electromagnetic field interpolated from data.
/// @tparam ACache Use cache or not. See `ElectromagneticFieldMap`.
template<muc::ceta_string ACache = "WithCache">
using ElectromagneticFieldMapSymYZ = ElectromagneticFieldMap<ACache, EFM::FieldMap3DSymYZ<Eigen::Vector<double, 6>, double, BEFieldSI2CLHEP>>;

/// @brief An XY, YZ, ZX plane mirror symmetry electromagnetic field interpolated from data.
/// @tparam ACache Use cache or not. See `ElectromagneticFieldMap`.
template<muc::ceta_string ACache = "WithCache">
using ElectromagneticFieldMapSymXYZ = ElectromagneticFieldMap<ACache, EFM::FieldMap3DSymXYZ<Eigen::Vector<double, 6>, double, BEFieldSI2CLHEP>>;

} // namespace Mustard::Detector::Field

#include "Mustard/Detector/Field/ElectromagneticFieldMap.inl"
