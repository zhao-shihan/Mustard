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

#include "Mustard/Math/POCA.h++"
#include "Mustard/Utility/MathConstant.h++"

#include "Math/BrentMinimizer1D.h"
#include "Math/WrappedFunction.h"

#include "muc/math"

#include <algorithm>
#include <cmath>

namespace Mustard::inline Math {

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

    const auto& [c, r, phi0, _1, _2]{helix};

    const auto xCT{axis - c};
    const auto phi1{xCT.phi()};
    auto phiCCw{muc::fmod(phi1 - phi0, 2 * pi)}; // use fmod, not std::remainder!
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

    if (phiLow >= phiUp) {
        return std::nullopt;
    }

    const auto& [o, r, phi0, z0, lambda]{helix};
    const auto& t{point};

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
    if (nTrialPts > 0) {
        // compute oscillation bound for convexity check
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
        // convexity check: f''(x) = -h(x) + 2 d; 2 d >= hMax
        //                    =>  f''(x) >= 0 for all x in [x1, x2]
        if (2 * d >= hMax) {
            nTrialPts = 0; // function is convex, no grid search needed
        } else if (nTrialPts == 1) {
            // adaptive grid density based on oscillation strength
            // sigma = (hMax - 2 d) / (hMax + 2 d) in (0, 1] quantifies convexity:
            //   sigma -> 0: nearly convex, sparse grid (pi/2 spacing)
            //   sigma -> 1: oscillatory, dense grid (pi/6 spacing)
            const auto sigma{(hMax - 2 * d) / (hMax + 2 * d)};
            const auto spacing{muc::lerp(pi / 2, pi / 6, sigma)};
            const auto nDefault{static_cast<int>((x2 - x1) / spacing) + 1};
            nTrialPts = std::max(5, nDefault);
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

    if (phiLow >= phiUp) {
        return std::nullopt;
    }

    const auto lineDMag2{line.direction.mag2()};
    if (muc::isclose(lineDMag2, 0.)) {
        // line direction vector degenerate
        const auto r{POCA(helix, line.point, phiLow, phiUp, nTrialPts, maxIter, absTol, relTol)};
        if (not r.has_value()) {
            return std::nullopt;
        }
        return HelixLinePOCAResult{r->poca, line.point, r->doca};
    }

    const auto& [o, r, phi0, z0, lambda]{helix};
    const auto& [t, originalD]{line};

    const auto d{originalD / std::sqrt(lineDMag2)};
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
    if (nTrialPts > 0) {
        // compute oscillation bounds for convexity check
        // first term = 4 (a0 cos(2x) + b0 sin(2x)) := 4 g(x)
        const auto g{[&](double x) {
            const auto [sin2x, cos2x]{muc::sincos(2 * x)};
            return a0 * cos2x + b0 * sin2x;
        }};
        const auto rAB0{muc::hypot(a0, b0)};
        const auto phiAB0{std::atan2(b0, a0)};
        const auto k1{(x1 - phiAB0 / 2) / pi};
        const auto k2{(x2 - phiAB0 / 2) / pi};
        const auto gMaxAtBound{std::floor(k1) == std::floor(k2)};
        const auto gMax{gMaxAtBound ? std::max(g(x1), g(x2)) : rAB0};
        // second term = (a1 - 2 b2 + a2 x) sin(x) + (b1 + 2 a2 + b2 x) cos(x)
        //             <= sqrt((a1 - 2 b2 + a2 x)^2 + (b1 + 2 a2 + b2 x)^2) := sqrt(h(x))
        const auto h{[&](double x) {
            return muc::hypot_sq(a1 - 2 * b2 + a2 * x, b1 + 2 * a2 + b2 * x);
        }};
        const auto hMax{std::max(h(x1), h(x2))};
        const auto convexBound{4 * gMax + std::sqrt(hMax)};
        // convexity check: f''(x) >= 2 c2 - convexBound
        if (2 * c2 >= convexBound) {
            nTrialPts = 0; // function is convex, no grid search needed
        } else if (nTrialPts == 1) {
            // adaptive grid density based on oscillation strength
            // sigma = (bound - 2 c2) / (bound + 2 c2) in (0, 1] quantifies convexity
            // the function has both sincos(x) and sincos(2x) terms;
            // sincos(2x) doubles the maximum frequency versus the helix-point case
            //   sigma -> 0: nearly convex, sparse grid (pi/4 spacing)
            //   sigma -> 1: oscillatory, dense grid (pi/12 spacing)
            const auto sigma{(convexBound - 2 * c2) / (convexBound + 2 * c2)};
            const auto spacing{muc::lerp(pi / 4, pi / 12, sigma)};
            const auto nDefault{static_cast<int>((x2 - x1) / spacing) + 1};
            nTrialPts = std::max(5, nDefault);
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

} // namespace Mustard::inline Math
