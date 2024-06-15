#pragma once

#include "Mustard/Concept/MathVector.h++"
#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Detector/Field/ElectricFieldBase.h++"
#include "Mustard/Utility/VectorCast.h++"

#include "EFM/FieldMap3D.h++"

#include "CLHEP/Units/SystemOfUnits.h"

#include "Eigen/Core"

#include "muc/functional"

namespace Mustard::Detector::Field {

/// @brief A functional type converts E-field SI field value
/// to CLHEP unit system. Use in EFM template parameter.
struct EFieldSI2CLHEP {
    template<Concept::MathVector3D T>
    [[nodiscard]] constexpr auto operator()(T E) const -> T {
        return E * (CLHEP::volt / CLHEP::m);
    }
};

/// @brief An electric field interpolated from data.
/// Initialization and interpolation are performed by `AFieldMap`.
/// @tparam AFieldMap A field map type, e.g. `EFM::FieldMap3D<Eigen::Vector3d>` or
/// `EFM::FieldMap3DSymZ<Eigen::Vector3d>`.
template<typename AFieldMap = EFM::FieldMap3D<Eigen::Vector3d>>
    requires std::same_as<typename AFieldMap::CoordinateType, double>
class ElectricFieldMap : public ElectricFieldBase<ElectricFieldMap<AFieldMap>>,
                         public AFieldMap {
public:
    using AFieldMap::AFieldMap;

    template<Concept::NumericVector3D T>
    auto E(T x) const -> T { return VectorCast<T>((*this)(x[0], x[1], x[2])); }
};

/// @brief An YZ plane mirror symmetry electric field interpolated from data.
using ElectricFieldMapSymX = ElectricFieldMap<EFM::FieldMap3DSymX<Eigen::Vector3d, double, EFieldSI2CLHEP>>;

/// @brief An ZX plane mirror symmetry electric field interpolated from data.
using ElectricFieldMapSymY = ElectricFieldMap<EFM::FieldMap3DSymY<Eigen::Vector3d, double, EFieldSI2CLHEP>>;

/// @brief An XY plane mirror symmetry electric field interpolated from data.
using ElectricFieldMapSymZ = ElectricFieldMap<EFM::FieldMap3DSymZ<Eigen::Vector3d, double, EFieldSI2CLHEP>>;

/// @brief An YZ, ZX plane mirror symmetry electric field interpolated from data.
using ElectricFieldMapSymXY = ElectricFieldMap<EFM::FieldMap3DSymXY<Eigen::Vector3d, double, EFieldSI2CLHEP>>;

/// @brief An XY, YZ plane mirror symmetry electric field interpolated from data.
using ElectricFieldMapSymXZ = ElectricFieldMap<EFM::FieldMap3DSymXZ<Eigen::Vector3d, double, EFieldSI2CLHEP>>;

/// @brief An ZX, XY plane mirror symmetry electric field interpolated from data.
using ElectricFieldMapSymYZ = ElectricFieldMap<EFM::FieldMap3DSymYZ<Eigen::Vector3d, double, EFieldSI2CLHEP>>;

/// @brief An XY, YZ, ZX plane mirror symmetry electric field interpolated from data.
using ElectricFieldMapSymXYZ = ElectricFieldMap<EFM::FieldMap3DSymXYZ<Eigen::Vector3d, double, EFieldSI2CLHEP>>;

} // namespace Mustard::Detector::Field
