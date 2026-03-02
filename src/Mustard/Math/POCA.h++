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

#include "Mustard/Math/GeometryRepresentation.h++"

#include "muc/numeric"

#include <optional>
#include <utility>

namespace Mustard::Math {

/// @brief Result of line-point POCA calculation
struct LinePoint3DPOCAResult {
    Point3D poca; ///< point of closest approach
    double doca;  ///< distance of closest approach
};

/// @brief Calculate the point of closest approach (POCA) between a line and a point
/// @param line The line
/// @param point The point
/// @return The POCA result containing the POCA and the distance
auto POCA(const Line3D& line, const Point3D& point) -> LinePoint3DPOCAResult;

/// @brief Result of line-line POCA calculation
struct LineLine3DPOCAResult {
    Point3D poca1; ///< point of closest approach on the first line
    Point3D poca2; ///< point of closest approach on the second line
    double doca;   ///< distance of closest approach
};

/// @brief Calculate the point of closest approach (POCA) between two lines
/// @param line1 The first line
/// @param line2 The second line
/// @return The POCA result containing the POCAs on both lines and the distance
auto POCA(const Line3D& line1, const Line3D& line2) -> LineLine3DPOCAResult;

/// @brief Result of helix-axis POCA calculation
struct HelixAxisPOCAResult {
    Point3D pocaCCw; ///< point of closest approach on the helix (in counter-clockwise direction)
    Point3D pocaCw;  ///< point of closest approach on the helix (in clockwise direction)
    double doca;     ///< distance of closest approach
};

/// @brief Calculate the point of closest approach (POCA) between a helix and an axis
/// @param helix The helix
/// @param axis The axis (2D point representing a line along the z direction)
/// @return The POCA result containing the POCAs in both directions and the distance
auto POCA(const Helix& helix, const Point2D& axis) -> HelixAxisPOCAResult;

/// @brief Result of helix-point POCA calculation
struct HelixPointPOCAResult {
    Point3D poca; ///< point of closest approach on the helix
    double doca;  ///< distance of closest approach
};

/// @brief Calculate the point of closest approach (POCA) between a helix and a point
/// within a specified phi range. This function uses Brent's method with initial
/// grid search to find the POCA
/// @param helix The helix
/// @param point The point
/// @param phiLow Lower bound of the helix azimuthal angle (relative to phi0)
/// @param phiUp Upper bound of the helix azimuthal angle (relative to phi0)
/// @param nTrialPts Number of trial points for initial grid search. If zero or negative,
/// skip both the unimodality check and initial grid search; if 1, determine the actual
/// number automatically (default: 1, i.e. determined automatically)
/// @param maxIter Maximum number of iterations for minimization (default: 300)
/// @param absTol Absolute tolerance for minimization (default: machine epsilon)
/// @param relTol Relative tolerance for minimization (default: sqrt(machine epsilon))
/// @return If successful, an instance of HelixPointPOCAResult containing the POCA and
/// the distance; null result if minimization failed
auto POCA(const Helix& helix, const Point3D& point, double phiLow, double phiUp,
          int nTrialPts = 1, int maxIter = 300,
          double absTol = muc::default_abs_tol<double>,
          double relTol = muc::default_rel_tol<double>) -> std::optional<HelixPointPOCAResult>;

/// @brief Result of helix-line POCA calculation
struct HelixLinePOCAResult {
    Point3D poca1; ///< point of closest approach on the helix
    Point3D poca2; ///< point of closest approach on the line
    double doca;   ///< distance of closest approach
};

/// @brief Calculate the point of closest approach (POCA) between a helix and a line
/// within a specified phi range. This function uses Brent's method with initial
/// grid search to find the POCA
/// @param helix The helix
/// @param line The line
/// @param phiLow Lower bound of the helix azimuthal angle (relative to phi0)
/// @param phiUp Upper bound of the helix azimuthal angle (relative to phi0)
/// @param nTrialPts Number of trial points for initial grid search. If zero or negative,
/// skip both the unimodality check and initial grid search; if 1, determine the actual
/// number automatically (default: 1, i.e. determined automatically)
/// @param maxIter Maximum number of iterations for minimization (default: 300)
/// @param absTol Absolute tolerance for minimization (default: machine epsilon)
/// @param relTol Relative tolerance for minimization (default: sqrt(machine epsilon))
/// @return If successful, an instance of HelixLinePOCAResult containing the POCAs and
/// the distance; null result if minimization failed
auto POCA(const Helix& helix, const Line3D& line, double phiLow, double phiUp,
          int nTrialPts = 1, int maxIter = 300,
          double absTol = muc::default_abs_tol<double>,
          double relTol = muc::default_rel_tol<double>) -> std::optional<HelixLinePOCAResult>;

} // namespace Mustard::Math
