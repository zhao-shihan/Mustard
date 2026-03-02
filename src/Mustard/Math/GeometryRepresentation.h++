// -*- C++ -*-
//
// Copyright (C) 2020-2025  Mustard developers
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

#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Vector/TwoVector.h"

#include "muc/array"
#include "muc/math"

#include <cmath>
#include <utility>

namespace Mustard::Math {

/// @brief 2D vector
using Vector2D = CLHEP::Hep2Vector;

/// @brief 2D point representation
using Point2D = Vector2D;

/// @brief 2D line representation
struct Line2D {
    Point2D point;      ///< A point on the line
    Vector2D direction; ///< Direction of the line (need not be normalized)

    /// @brief Get point at parameter t
    /// @param t Parameter along the line
    /// @return Point on the line at parameter t
    auto PointAt(double t) const -> Point2D {
        return point + t * direction;
    }
};

/// @brief 3D vector
using Vector3D = CLHEP::Hep3Vector;

/// @brief 3D point representation
using Point3D = Vector3D;

/// @brief 3D line representation
struct Line3D {
    Point3D point;      ///< A point on the line
    Vector3D direction; ///< Direction of the line (need not be normalized)

    /// @brief Get point at parameter t
    /// @param t Parameter along the line
    /// @return Point on the line at parameter t
    auto PointAt(double t) const -> Point3D {
        return point + t * direction;
    }
};

/// @brief 3D helix representation
///
/// The helix parametric equations are defined as
/// @f[
/// \begin{equation}
/// x(\phi) = c_x + r \cos\left( \phi + \phi_0 \right) \right) \quad
/// y(\phi) = c_y + r \sin\left( \phi + \phi_0 \right) \right) \quad
/// z(\phi) = z_0 + r \phi \cot\lambda
/// \end{equation}
/// @f]
/// where @f$r\in(0, +\infty)@f$ is the transverse radius,
/// @f$(c_x, c_y)\in\mathbb{R}^2@f$ is the transverse center,
/// @f$\phi_0\in(-\pi, \pi]@f$ is the helix azimuthal angle at the reference point,
/// @f$z_0\in\mathbb{R}@f$ is the reference z coordinate, and
/// @f$\lambda\in(-\pi, \pi)\backslash\{0\}@f$ is the dip angle.
///
/// Although @f$\tan\lambda@f$ has mathematical poles at @f$\pm\pi/2@f$,
/// no common floating-point representation is able to represent @f$\pm\pi/2@f$ exactly,
/// thus there is no value of the argument for which a pole error occurs.
///
/// The relationship between direction and @f$(\phi_0, \lambda)@f$ is given by
/// @f[
/// \begin{equation}
/// d_{0,x} = -\sin\phi_0 \sin\lambda \quad
/// d_{0,y} = \cos\phi_0 \sin\lambda \quad
/// d_{0,z} = \cos\lambda
/// \end{equation}
/// @f]
struct Helix {
    Point2D center; ///< Transverse center of the helix
    double radius;  ///< Transverse radius of the helix
    double phi0;    ///< Azimuthal angle at reference point
    double z0;      ///< z coordinate at reference point
    double lambda;  ///< Dip angle

    /// @brief Get point at helix azimuthal angle phi (relative to phi0)
    /// @param phi Helix azimuthal angle (relative to phi0)
    /// @return Point on the helix at azimuthal angle phi
    auto PointAt(double phi) const -> Point3D {
        const auto [sinX, cosX]{muc::sincos(phi + phi0)};
        return {center.x() + radius * cosX,
                center.y() + radius * sinX,
                z0 + radius * phi / std::tan(lambda)};
    }

    /// @brief Get tangent direction at helix azimuthal angle phi (relative to phi0)
    /// @param phi Helix azimuthal angle (relative to phi0)
    /// @return Tangent vector on the helix at azimuthal angle phi
    auto DirectionAt(double phi) const -> Vector3D {
        const auto [sinX, cosX]{muc::sincos(phi + phi0)};
        const auto [sinLambda, cosLambda]{muc::sincos(lambda)};
        return {-sinX * sinLambda,
                cosX * sinLambda,
                cosLambda};
    }

    /// @brief Get point and direction at helix azimuthal angle phi (relative to phi0)
    /// @param phi Helix azimuthal angle (relative to phi0)
    /// @return Pair of point and tangent vector on the helix at azimuthal angle phi
    /// @note This can be more efficient than calling PointAt() and DirectionAt() separately
    auto PointDirectionAt(double phi) const -> std::pair<Point3D, Vector3D> {
        const auto [sinX, cosX]{muc::sincos(phi + phi0)};
        const auto [sinLambda, cosLambda]{muc::sincos(lambda)};
        const auto cotLambda{cosLambda / sinLambda};
        const Point3D point{center.x() + radius * cosX,
                            center.y() + radius * sinX,
                            z0 + radius * phi * cotLambda};
        const Vector3D direction{-sinX * sinLambda,
                                 cosX * sinLambda,
                                 cosLambda};
        return {point, direction};
    }
};

} // namespace Mustard::Math
