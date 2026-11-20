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

#include "Mustard/CLI/BasicCLI.h++"
#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/IO/Print.h++"
#include "Mustard/Math/POCA.h++"
#include "Mustard/Testing/TestHelixAxisPOCA/TestHelixAxisPOCA.h++"
#include "Mustard/Utility/MathConstant.h++"

#include "muc/chrono"
#include "muc/numeric"

#include <cmath>
#include <cstdlib>

namespace Mustard::Testing {

TestHelixAxisPOCA::TestHelixAxisPOCA() :
    Subprogram{"TestHelixAxisPOCA", "Test and benchmark Helix-Axis POCA computation."} {}

auto TestHelixAxisPOCA::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace Mustard::Math;
    using namespace Mustard::MathConstant;

    int nPassed{};
    int nTotal{};

    const auto isClose3D{[](const auto& a, const auto& b) -> bool {
        return muc::isclose(a.x(), b.x()) and
               muc::isclose(a.y(), b.y()) and
               muc::isclose(a.z(), b.z());
    }};

    // helper: verify point transverse position is on the helix
    const auto onHelix{[](const Helix& helix, const Point3D& p) -> bool {
        const auto dx{p.x() - helix.center.x()};
        const auto dy{p.y() - helix.center.y()};
        return muc::isclose(muc::hypot(dx, dy), helix.radius);
    }};

    PrintLn("=== TestHelixAxisPOCA ===");

    // ---- Case 3.1: Axis at center ----
    ++nTotal;
    {
        const Helix helix({0, 0}, 5., 0., 0., pi / 4);
        const Point2D axis{0, 0};
        const auto result{POCA(helix, axis)};
        // doca = |0 - 5| = 5; both pocas at same point (phiCCw=phiCw=0)
        const bool ok{muc::isclose(result.doca, 5.) and
                      isClose3D(result.pocaCCw, helix.PointAt(0)) and
                      isClose3D(result.pocaCw, helix.PointAt(0))};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 3.1 (axis at center): {} (pocaCCw=({},{},{}) pocaCw=({},{},{}) doca={})",
                ok ? "PASS" : "FAIL",
                result.pocaCCw.x(), result.pocaCCw.y(), result.pocaCCw.z(),
                result.pocaCw.x(), result.pocaCw.y(), result.pocaCw.z(), result.doca);
    }

    // ---- Case 3.2: Axis offset from center ----
    ++nTotal;
    {
        const Helix helix({5, 0}, 2., 0., 0., pi / 4);
        const Point2D axis{0, 0};
        const auto result{POCA(helix, axis)};
        // xCT = (0,0)-(5,0) = (-5,0), |xCT| = 5, doca = |5-2| = 3
        // phi1 = atan2(0,-5) = pi, phiCCw = pi, phiCw = -pi
        const bool ok{muc::isclose(result.doca, 3.) and
                      isClose3D(result.pocaCCw, helix.PointAt(pi)) and
                      isClose3D(result.pocaCw, helix.PointAt(-pi))};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 3.2 (axis offset): {} (pocaCCw=({},{},{}) pocaCw=({},{},{}) doca={})",
                ok ? "PASS" : "FAIL",
                result.pocaCCw.x(), result.pocaCCw.y(), result.pocaCCw.z(),
                result.pocaCw.x(), result.pocaCw.y(), result.pocaCw.z(), result.doca);
    }

    // ---- Case 3.3: Non-zero phi0 ----
    ++nTotal;
    {
        const Helix helix({0, 0}, 3., pi / 2, 10., pi / 3);
        const Point2D axis{4, 0};
        const auto result{POCA(helix, axis)};
        // |axis - center| = 4, r = 3, doca = |4-3| = 1
        // Verify pocaCCw/Cw are on the helix (distance from center in transverse = radius)
        const Point2D pocaCcwTransverse{result.pocaCCw.x(), result.pocaCCw.y()};
        const bool ccwOnHelix{muc::isclose(pocaCcwTransverse.mag(), 3.)};
        const Point2D pocaCwTransverse{result.pocaCw.x(), result.pocaCw.y()};
        const bool cwOnHelix{muc::isclose(pocaCwTransverse.mag(), 3.)};
        const bool ok{muc::isclose(result.doca, 1.) and
                      ccwOnHelix and
                      cwOnHelix};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 3.3 (phi0=pi/2): {} (doca={}, pocaCCw on helix: {}, pocaCw on helix: {})",
                ok ? "PASS" : "FAIL", result.doca,
                ccwOnHelix ? "yes" : "no", cwOnHelix ? "yes" : "no");
    }

    // ---- Case 3.4: Axis on helix circle (doca = 0) ----
    ++nTotal;
    {
        const Helix helix({0, 0}, 10., 3., 0., 1.);
        const Point2D axis{10 / sqrt2, 10 / sqrt2}; // distance from center = 10 = radius
        const auto result{POCA(helix, axis)};
        const bool ok{muc::isclose(result.doca, 0.) and
                      muc::isclose(result.pocaCCw.x(), axis.x()) and
                      muc::isclose(result.pocaCCw.y(), axis.y()) and
                      muc::isclose(result.pocaCw.x(), axis.x()) and
                      muc::isclose(result.pocaCw.y(), axis.y()) and
                      onHelix(helix, result.pocaCCw) and
                      onHelix(helix, result.pocaCw)};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 3.4 (axis on helix circle): {} (pocaCCw=({},{},{}) pocaCw=({},{},{}) doca={})",
                ok ? "PASS" : "FAIL",
                result.pocaCCw.x(), result.pocaCCw.y(), result.pocaCCw.z(),
                result.pocaCw.x(), result.pocaCw.y(), result.pocaCw.z(), result.doca);
    }

    // ---- Timing ----
    constexpr auto testCount{1'000'000};
    PrintLn("  Timing ({} loops using Case 3.2 geometry with perturbation):", testCount);
    const Helix tHelix({5, 0}, 2., 0., 0., pi / 4);
    Point2D tAxis{0, 0};
    volatile double dummyPOCA{};
    volatile double dummyDOCA{};
    const auto computePOCA{[&]() {
        auto r{POCA(tHelix, tAxis)};
        dummyPOCA += r.pocaCCw.x();
        dummyPOCA += r.pocaCw.x();
        dummyDOCA += r.doca;
        tAxis[0] += 1e-9;
    }};
    // warmup
    for (int i{}; i < testCount / 100; ++i) {
        computePOCA();
    }
    muc::chrono::stopwatch stopwatch;
    for (int i{}; i < testCount; ++i) {
        computePOCA();
    }
    muc::chrono::milliseconds<double> time{stopwatch.read()};
    PrintLn("    {} calls: {} (avg: {}/call)", testCount, time, muc::chrono::nanoseconds<double>{time / testCount});

    // ---- Summary ----
    PrintLn("  Result: {}/{} passed", nPassed, nTotal);
    return nPassed == nTotal ? EXIT_SUCCESS : EXIT_FAILURE;
}

} // namespace Mustard::Testing
