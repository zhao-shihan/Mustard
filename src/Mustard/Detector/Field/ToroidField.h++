#pragma once

#include "Mustard/Concept/InputVector.h++"
#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Detector/Field/MagneticFieldBase.h++"
#include "Mustard/Utility/VectorCast.h++"

#include "Eigen/Core"
#include "Eigen/Geometry"

#include <cmath>

namespace Mustard::Detector::Field {

/// @brief Inner magnetic field of a perfect toroid.
class ToroidField : public MagneticFieldBase<ToroidField> {
public:
    /// @brief The constructor.
    /// @tparam T1 type of `x0` satisfies `Concept::InputVector3D`
    /// @tparam T2 type of `n` satisfies `Concept::InputVector3D`
    /// @param b magnetic flux density at r=r0
    /// @param r0 toroid big radius
    /// @param x0 toroid center position
    /// @param n toroid normal direction (the magnetic field is counterclockwisely around this direction)
    template<Concept::InputVector3D T1 = Eigen::Vector3d, Concept::InputVector3D T2 = Eigen::Vector3d>
    ToroidField(double b, double r0, T1 x0, T2 n);

    template<Concept::NumericVector3D T>
    auto B(T x) const -> T;

private:
    double fB;
    double fR0;
    Eigen::Transform<double, 3, Eigen::Isometry> fTransform;
};

} // namespace Mustard::Detector::Field

#include "Mustard/Detector/Field/ToroidField.inl"
