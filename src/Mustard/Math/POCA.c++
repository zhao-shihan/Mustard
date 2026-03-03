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

#include "Mustard/Math/POCA.h++"
#include "Mustard/Utility/MathConstant.h++"

#include "Math/BrentMinimizer1D.h"
#include "Math/WrappedFunction.h"

#include "muc/math"

#include <algorithm>
#include <cmath>

namespace Mustard::Math {

auto POCA(const Line3D& line, const Point3D& point) -> LinePoint3DPOCAResult {
    const auto& [x1, d1]{line};
    const auto& x2{point};

    const auto d1d1{d1.mag2()};
    const auto x12{x2 - x1};
    if (muc::isclose(d1d1, 0.)) {
        // direction vector degenerates
        return {x1, x12.mag()};
    }

    // general case
    const auto t{x12.dot(d1) / d1d1};
    const auto poca{x1 + t * d1};
    return {poca, (point - poca).mag()};
}

auto POCA(const Line3D& line1, const Line3D& line2) -> LineLine3DPOCAResult {
    const auto& [x1, d1]{line1};
    const auto& [x2, d2]{line2};

    const auto d2d2{d2.mag2()};
    if (muc::isclose(d2d2, 0.)) {
        // line 2 direction vector degenerate
        const auto [poca1, doca]{POCA(line1, x2)};
        return {poca1, x2, doca};
    }
    const auto d1d1{d1.mag2()};
    const auto d1d2{d1.dot(d2)};
    const auto denom{d1d1 * d2d2 - d1d2 * d1d2};
    if (muc::isclose(d1d1, 0.) or muc::isclose(denom, 0.)) {
        // line 1 direction vector degenerate or lines parallel
        const auto [poca2, doca]{POCA(line2, x1)};
        return {x1, poca2, doca};
    }

    // general case
    const auto x12{x2 - x1};
    const auto x12d1{x12.dot(d1)};
    const auto x12d2{x12.dot(d2)};
    const auto t1{(x12d1 * d2d2 - d1d2 * x12d2) / denom};
    const auto t2{(x12d1 * d1d2 - d1d1 * x12d2) / denom};
    const auto poca1{x1 + t1 * d1};
    const auto poca2{x2 + t2 * d2};
    return {poca1, poca2, (poca2 - poca1).mag()};
}

auto POCA(const Helix& helix, const Point2D& axis) -> HelixAxisPOCAResult {
    using Mustard::MathConstant::pi;

    const auto& [c, r, phi0, z0, lambda]{helix};

    const auto xCT{axis - c};
    const auto phi1{xCT.phi()};
    auto phiCCw{std::fmod(phi1 - phi0, 2 * pi)}; // use std::fmod, not std::remainder!
    if (phiCCw < 0) {
        phiCCw += 2 * pi;
    } // ensure in [0, 2pi)
    auto phiCw{phiCCw - 2 * pi};
    if (muc::isclose(phiCCw, 0.)) {
        phiCw = 0;
    } // ensure in (-2pi, 0]

    const auto pocaCCw{helix.PointAt(phiCCw)};
    const auto pocaCw{helix.PointAt(phiCw)};
    const auto doca{std::abs(xCT.mag() - r)};
    return {pocaCCw, pocaCw, doca};
}

auto POCA(const Helix& helix, const Point3D& point, double phiLow, double phiUp,
          int nTrialPts, int maxIter, double absTol, double relTol) -> std::optional<HelixPointPOCAResult> {
    using Mustard::MathConstant::pi;

    const auto& [o, r, phi0, z0, lambda]{helix};
    const auto& t{point};
    if (phiLow >= phiUp) {
        return std::nullopt;
    }

    const auto k{1 / std::tan(lambda)}; // can be 0
    const auto a{(o.x() - t.x()) / r};
    const auto b{(o.y() - t.y()) / r};
    const auto c{k * ((z0 - t.z()) / r - k * phi0)};
    const auto d{k * k / 2};
    // 1/2*(d(x)/r)^2 without constant terms; to be minimized (x=phi+phi0)
    const ROOT::Math::WrappedFunction reducedSquaredDistance{[&](double x) {
        const auto [sinx, cosx]{muc::sincos(x)};
        return a * cosx + b * sinx + (c + d * x) * x;
    }};
    // define interval
    const auto x1{phiLow + phi0};
    const auto x2{phiUp + phi0};

    // determine initial trial points
    if (nTrialPts == 1) {
        // default to 1 point for every pi/5 interval; minimum 5 points
        const auto nDefault{static_cast<int>((x2 - x1) / (pi / 5)) + 1};
        nTrialPts = std::max(5, nDefault);
    }
    if (nTrialPts > 0) {
        // unimodality check
        const auto h{[&](double x) {
            const auto [sinx, cosx]{muc::sincos(x)};
            return a * cosx + b * sinx;
        }};
        const auto rAB{muc::hypot(a, b)};
        const auto phiAB{std::atan2(b, a)};
        const auto k1{(x1 - phiAB) / (2 * pi)};
        const auto k2{(x2 - phiAB) / (2 * pi)};
        const auto hMaxAtBound{std::floor(k1) == std::floor(k2)};
        const auto hMax{hMaxAtBound ? std::max(h(x1), h(x2)) : rAB};
        // check the unimodality condition
        if (2 * d >= hMax) { // function is unimodal in the interval
            nTrialPts = 0;   // no need of grid search for unimodal function
        }
    }

    // minimize
    ROOT::Math::BrentMinimizer1D minimizer;
    minimizer.SetFunction(reducedSquaredDistance, x1, x2);
    minimizer.SetNpx(nTrialPts); // nTrialPts<2 skips grid search in ROOT::Math::BrentMinimizer1D
    const auto converged{minimizer.Minimize(maxIter, absTol, relTol)};
    if (not converged) {
        return std::nullopt;
    }

    // compute POCA
    const auto phiOCA{minimizer.XMinimum() - phi0};
    const auto poca{helix.PointAt(phiOCA)};
    return HelixPointPOCAResult{poca, (point - poca).mag()};
}

auto POCA(const Helix& helix, const Line3D& line, double phiLow, double phiUp,
          int nTrialPts, int maxIter, double absTol, double relTol) -> std::optional<HelixLinePOCAResult> {
    using Mustard::MathConstant::pi;

    const auto& [o, r, phi0, z0, lambda]{helix};
    const auto& [t, originalD]{line};
    if (phiLow >= phiUp) {
        return std::nullopt;
    }

    const auto d{originalD.unit()};
    const auto dxdy{d.x() * d.y()};
    const auto dxdz{d.x() * d.z()};
    const auto dydz{d.y() * d.z()};
    const auto dx2{muc::pow(d.x(), 2)};
    const auto dy2{muc::pow(d.y(), 2)};
    const Point3D xC{o.x(), o.y(), z0};
    const auto tPrime{(t - xC) / r};
    const auto delta{(tPrime.dot(d) * d - tPrime)};
    const auto k{1 / std::tan(lambda)}; // can be 0

    const auto a0{(dy2 - dx2) / 4};
    const auto b0{-dxdy / 2};
    const auto a1{delta.x() + k * dxdz * phi0};
    const auto b1{delta.y() + k * dydz * phi0};
    const auto c1{k * (delta.z() - k * (dx2 + dy2) * phi0)};
    const auto a2{-k * dxdz};
    const auto b2{-k * dydz};
    const auto c2{k * k * (dx2 + dy2) / 2};
    // 1/2*(d(x)/r)^2 without constant terms; to be minimized (x=phi+phi0)
    const ROOT::Math::WrappedFunction reducedSquaredDistance{[&](double x) {
        const auto [sinx, cosx]{muc::sincos(x)};
        //>  a0 cos(2x) + b0 sin(2x) +
        //>  a1 cos(x) + b1 sin(x) + c1 x +
        //> (a2 cos(x) + b2 sin(x) + c2 x) x
        return a0 * (cosx - sinx) * (cosx + sinx) +
               a1 * cosx + (b1 + 2 * b0 * cosx) * sinx +
               (a2 * cosx + b2 * sinx + c2 * x + c1) * x;
    }};
    // define interval
    const auto x1{phiLow + phi0};
    const auto x2{phiUp + phi0};

    // determine initial trial points
    if (nTrialPts == 1) {
        // default to 1 point for every pi/5 interval; minimum 5 points
        const auto nDefault{static_cast<int>((x2 - x1) / (pi / 5)) + 1};
        nTrialPts = std::max(5, nDefault);
    }
    if (nTrialPts > 0) {
        // deal with the first term of inequality
        const auto g{[&](double x) {
            const auto [sin2x, cos2x]{muc::sincos(2 * x)};
            return a0 * cos2x + b0 * sin2x;
        }};
        // check whether there is a maximum between x1 and x2,
        // if so, use the maximum of g; otherwise use the endpoints
        const auto rAB{muc::hypot(a0, b0)};
        const auto phiAB{std::atan2(b0, a0)};
        const auto k1{(x1 - phiAB / 2) / pi};
        const auto k2{(x2 - phiAB / 2) / pi};
        const auto gMaxAtBound{std::floor(k1) == std::floor(k2)};
        const auto gMax{gMaxAtBound ? std::max(g(x1), g(x2)) : rAB};
        // deal with the second term of inequality
        const auto h{[&](double x) {
            return muc::hypot_sq(a1 - 2 * b2 + a2 * x, b1 + 2 * a2 + b2 * x);
        }};
        // apply the extreme value theorem directly to h
        const auto hMax{std::max(h(x1), h(x2))};
        // check the unimodality condition
        if (2 * c2 >= 4 * gMax + std::sqrt(hMax)) { // function is unimodal in the interval
            nTrialPts = 0;                          // no need of grid search for unimodal function
        }
    }

    // minimize
    ROOT::Math::BrentMinimizer1D minimizer;
    minimizer.SetFunction(reducedSquaredDistance, x1, x2);
    minimizer.SetNpx(nTrialPts); // nTrialPts<2 skips grid search in ROOT::Math::BrentMinimizer1D
    const auto converged{minimizer.Minimize(maxIter, absTol, relTol)};
    if (not converged) {
        return std::nullopt;
    }

    // compute POCAs
    const auto phiOCA{minimizer.XMinimum() - phi0};
    const auto poca1{helix.PointAt(phiOCA)};
    const auto [poca2, doca]{POCA(line, poca1)};
    return HelixLinePOCAResult{poca1, poca2, doca};
}

} // namespace Mustard::Math
