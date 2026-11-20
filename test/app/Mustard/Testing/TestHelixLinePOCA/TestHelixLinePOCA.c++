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
#include "Mustard/Testing/TestHelixLinePOCA/TestHelixLinePOCA.h++"
#include "Mustard/Utility/MathConstant.h++"

#include "muc/chrono"
#include "muc/numeric"

#include <cmath>
#include <cstdlib>

namespace Mustard::Testing {

TestHelixLinePOCA::TestHelixLinePOCA() :
    Subprogram{"TestHelixLinePOCA", "Test and benchmark Helix-Line POCA computation."} {}

auto TestHelixLinePOCA::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace Mustard::Math;
    using Mustard::MathConstant::pi;

    int nPassed{0};
    int nTotal{0};

    // helper: verify point transverse position is on the helix
    const auto onHelix{[](const Helix& helix, const Point3D& p) -> bool {
        const auto dx{p.x() - helix.center.x()};
        const auto dy{p.y() - helix.center.y()};
        return muc::isclose(muc::hypot(dx, dy), helix.radius);
    }};

    const auto pointOnLine{[](const Line3D& line, const Point3D& p) -> bool {
        const auto d{(p - line.point).cross(line.direction)};
        return muc::isclose(d.mag(), 0., {.abs = 1e-6});
    }};

    PrintLn("=== TestHelixLinePOCA ===");

    // ---- Case 5.1: Line tangent to helix at a point ----
    ++nTotal;
    {
        const Helix helix({0, 0}, 5., 0., 0., pi / 4);
        const double phiOnHelix{0.8};
        const Point3D onHelix_pt{helix.PointAt(phiOnHelix)};
        const Vector3D tangent{helix.DirectionAt(phiOnHelix)};
        const Line3D line{onHelix_pt, tangent};
        const auto result{POCA(helix, line, -5., 5.)};
        // use absolute tolerance for near-zero doca comparison
        const bool ok{result.has_value() and
                      result->doca < 1e-7 and
                      onHelix(helix, result->poca1) and
                      pointOnLine(line, result->poca2) and
                      muc::isclose((result->poca2 - result->poca1).mag(), result->doca)};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 5.1 (line tangent to helix): {} (doca={:.6e}, poca1=({},{},{}) poca2=({},{},{}))",
                ok ? "PASS" : "FAIL",
                result.has_value() ? result->doca : -1.,
                result.has_value() ? result->poca1.x() : 0.,
                result.has_value() ? result->poca1.y() : 0.,
                result.has_value() ? result->poca1.z() : 0.,
                result.has_value() ? result->poca2.x() : 0.,
                result.has_value() ? result->poca2.y() : 0.,
                result.has_value() ? result->poca2.z() : 0.);
    }

    // ---- Case 5.2: Line offset from helix ----
    ++nTotal;
    {
        const Helix helix({0, 0}, 1., 0., 0., pi / 4);
        const Line3D line({2, 0, 0}, {0, 0, 1}); // z-axis through (2,0)
        const auto result{POCA(helix, line, -pi, pi)};
        // closest approach is approx helix point (1,0,0) to line point (2,0,0): doca ~ 1
        const bool ok{result.has_value() and
                      muc::isclose(result->doca, 1.) and
                      onHelix(helix, result->poca1) and
                      pointOnLine(line, result->poca2) and
                      muc::isclose((result->poca2 - result->poca1).mag(), result->doca)};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 5.2 (line offset from helix): {} (doca={}, poca1=({},{},{}) poca2=({},{},{}))",
                ok ? "PASS" : "FAIL",
                result.has_value() ? result->doca : -1.,
                result.has_value() ? result->poca1.x() : 0.,
                result.has_value() ? result->poca1.y() : 0.,
                result.has_value() ? result->poca1.z() : 0.,
                result.has_value() ? result->poca2.x() : 0.,
                result.has_value() ? result->poca2.y() : 0.,
                result.has_value() ? result->poca2.z() : 0.);
    }

    // ---- Case 5.3: phiLow >= phiUp returns nullopt ----
    ++nTotal;
    {
        const Helix helix({0, 0}, 1., 0., 0., pi / 4);
        const Line3D line({2, 0, 0}, {0, 0, 1});
        const auto result{POCA(helix, line, 1., 0.5)}; // phiLow > phiUp
        const bool ok{not result.has_value()};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 5.3 (phiLow>=phiUp): {} (result is nullopt: {})",
                ok ? "PASS" : "FAIL", not result.has_value() ? "yes" : "no");
    }

    // ---- Case 5.4: nTrialPts=0 still converges ----
    ++nTotal;
    {
        const Helix helix({0, 0}, 1., 0., 0., pi / 4);
        const Line3D line({2, 0, 0}, {0, 0, 1});
        const auto result{POCA(helix, line, -pi, pi, 0)}; // nTrialPts=0
        const bool ok{result.has_value() and
                      muc::isclose(result->doca, 1.) and
                      onHelix(helix, result->poca1) and
                      pointOnLine(line, result->poca2) and
                      muc::isclose((result->poca2 - result->poca1).mag(), result->doca)};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 5.4 (nTrialPts=0): {} (doca={}, poca1=({},{},{}) poca2=({},{},{}))",
                ok ? "PASS" : "FAIL",
                result.has_value() ? result->doca : -1.,
                result.has_value() ? result->poca1.x() : 0.,
                result.has_value() ? result->poca1.y() : 0.,
                result.has_value() ? result->poca1.z() : 0.,
                result.has_value() ? result->poca2.x() : 0.,
                result.has_value() ? result->poca2.y() : 0.,
                result.has_value() ? result->poca2.z() : 0.);
    }

    // ---- Case 5.5: degenerate line direction falls back to helix-point POCA ----
    ++nTotal;
    {
        const Helix helix({0, 0}, 1., 0., 0., pi / 4);
        const Line3D line({2, 0, 0}, {0, 0, 0}); // degenerate direction
        const auto result{POCA(helix, line, -pi, pi)};
        // Falls back: POCA(helix, line.point) → closest on helix to (2,0,0)
        // Helix point near phi=0: (1,0,0), line point is (2,0,0) → doca ≈ 1
        const bool ok{result.has_value() and
                      muc::isclose(result->doca, 1.) and
                      pointOnLine(line, result->poca2) and
                      onHelix(helix, result->poca1)};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 5.5 (degenerate line): {} (doca={})",
                ok ? "PASS" : "FAIL",
                result.has_value() ? result->doca : -1.);
    }

    // ---- Case 5.6: phiLow == phiUp returns nullopt ----
    ++nTotal;
    {
        const Helix helix({0, 0}, 1., 0., 0., pi / 4);
        const Line3D line({2, 0, 0}, {0, 0, 1});
        const auto result{POCA(helix, line, 0.5, 0.5)}; // equal bounds
        const bool ok{not result.has_value()};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 5.6 (phiLow==phiUp): {} (result is nullopt: {})",
                ok ? "PASS" : "FAIL", not result.has_value() ? "yes" : "no");
    }

    // ---- Case 5.7: Brent non-convergence with insufficient maxIter ----
    ++nTotal;
    {
        const Helix helix({0, 0}, 1., 0., 0., pi / 4);
        const Line3D line({2, 0, 0}, {0, 0, 1});
        const auto result{POCA(helix, line, -pi, pi, 0, 3)}; // nTrialPts=0, maxIter=3
        const bool ok{not result.has_value()};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 5.7 (Brent non-convergence): {} (result is nullopt: {})",
                ok ? "PASS" : "FAIL", not result.has_value() ? "yes" : "no");
    }

    // ---- Case 5.8: multimodal function triggers grid search ----
    ++nTotal;
    {
        const Helix helix({0, 0}, 1., 0., 0., pi / 2 - 0.001);
        const Line3D line({5, 0, 0}, {1, 0.3, 0.});
        const auto result{POCA(helix, line, -10., 10.)};
        // Very small cot^2 + wide interval → unimodality fails → grid search used
        const bool ok{result.has_value() and
                      onHelix(helix, result->poca1) and
                      pointOnLine(line, result->poca2)};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 5.8 (multimodal w/ grid search): {} (doca={})",
                ok ? "PASS" : "FAIL",
                result.has_value() ? result->doca : -1.);
    }

    // ---- Case 5.9: non-zero phi0 exercises phi0 terms in coefficient computation ----
    ++nTotal;
    {
        const Helix helix({0, 0}, 2., 1.2, 5., pi / 3);
        const Line3D line({0, -3, 10}, {0.8, 0.3, 0.6}); // direction has both transverse and z components → dxdz, dydz ≠ 0
        const auto result{POCA(helix, line, -4 * pi, 4 * pi)};
        // Verifies POCA computes correctly with non-zero phi0 terms in a1, b1, c1
        const bool ok{result.has_value() and
                      onHelix(helix, result->poca1) and
                      pointOnLine(line, result->poca2)};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 5.9 (non-zero phi0): {} (doca={})",
                ok ? "PASS" : "FAIL",
                result.has_value() ? result->doca : -1.);
    }

    // ---- Timing ----
    constexpr auto testCount{10'000};
    PrintLn("  Timing ({} loops, using Case 5.2 geometry with perturbation):", testCount);
    const Helix tHelix({0, 0}, 1., 0., 0., pi / 4);
    Line3D tLine({2, 0, 0}, {0, 0, 1});
    volatile double dummyPOCA{};
    volatile double dummyDOCA{};
    const auto computePOCA{[&]() {
        auto r{POCA(tHelix, tLine, -pi, pi)};
        if (r) {
            dummyPOCA += r->poca1.x();
            dummyPOCA += r->poca2.x();
            dummyDOCA += r->doca;
        }
        tLine.point[0] += 1e-9;
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
