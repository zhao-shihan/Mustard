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
#include "Mustard/Testing/TestHelixPointPOCA/TestHelixPointPOCA.h++"
#include "Mustard/Utility/MathConstant.h++"

#include "muc/chrono"
#include "muc/numeric"

#include <cmath>
#include <cstdlib>

namespace Mustard::Testing {

TestHelixPointPOCA::TestHelixPointPOCA() :
    Subprogram{"TestHelixPointPOCA", "Test and benchmark Helix-Point POCA computation."} {}

auto TestHelixPointPOCA::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace Mustard::Math;
    using Mustard::MathConstant::pi;

    int nPassed{};
    int nTotal{};

    // helper: verify point transverse position is on the helix
    const auto onHelix{[](const Helix& helix, const Point3D& p) -> bool {
        const auto dx{p.x() - helix.center.x()};
        const auto dy{p.y() - helix.center.y()};
        return muc::isclose(muc::hypot(dx, dy), helix.radius);
    }};

    PrintLn("=== TestHelixPointPOCA ===");

    // ---- Case 4.1: Point on helix ----
    ++nTotal;
    {
        const Helix helix({0, 0}, 10., 0., 0., pi / 4);
        const Point3D pointOnHelix{helix.PointAt(1.5)};
        const auto result{POCA(helix, pointOnHelix, -5., 5.)};
        // use absolute tolerance for near-zero doca comparison
        const bool ok{result.has_value() and
                      result->doca < 1e-7 and
                      onHelix(helix, result->poca)};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 4.1 (point on helix): {} (doca={:.6e}, poca=({},{},{}))",
                ok ? "PASS" : "FAIL",
                result.has_value() ? result->doca : -1.,
                result.has_value() ? result->poca.x() : 0.,
                result.has_value() ? result->poca.y() : 0.,
                result.has_value() ? result->poca.z() : 0.);
    }

    // ---- Case 4.2: Point offset radially ----
    ++nTotal;
    {
        const Helix helix({0, 0}, 1., 0., 0., 1.5);
        const Point3D point{2., 0., 0.};
        const auto result{POCA(helix, point, -pi / 2, pi / 2)};
        // closest point near phi=0 where helix point is (1,0,~0), distance ~1
        const bool ok{result.has_value() and
                      muc::isclose(result->doca, 1.) and
                      onHelix(helix, result->poca)};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 4.2 (point offset radially): {} (doca={}, poca=({},{},{}))",
                ok ? "PASS" : "FAIL",
                result.has_value() ? result->doca : -1.,
                result.has_value() ? result->poca.x() : 0.,
                result.has_value() ? result->poca.y() : 0.,
                result.has_value() ? result->poca.z() : 0.);
    }

    // ---- Case 4.3: phiLow >= phiUp returns nullopt ----
    ++nTotal;
    {
        const Helix helix({0, 0}, 1., 0., 0., 1.);
        const Point3D point{5., 5., 5.};
        const auto result{POCA(helix, point, 2., 1.)}; // phiLow > phiUp
        const bool ok{not result.has_value()};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 4.3 (phiLow>=phiUp): {} (result is nullopt: {})",
                ok ? "PASS" : "FAIL", not result.has_value() ? "yes" : "no");
    }

    // ---- Case 4.4: nTrialPts=0 still converges ----
    ++nTotal;
    {
        const Helix helix({0, 0}, 1., 0., 0., 1.5);
        const Point3D point{2., 0., 0.};
        const auto result{POCA(helix, point, -pi / 2, pi / 2, 0)}; // nTrialPts=0
        const bool ok{result.has_value() and
                      muc::isclose(result->doca, 1.) and
                      onHelix(helix, result->poca)};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 4.4 (nTrialPts=0): {} (doca={}, poca=({},{},{}))",
                ok ? "PASS" : "FAIL",
                result.has_value() ? result->doca : -1.,
                result.has_value() ? result->poca.x() : 0.,
                result.has_value() ? result->poca.y() : 0.,
                result.has_value() ? result->poca.z() : 0.);
    }

    // ---- Case 4.5: phiLow == phiUp returns nullopt ----
    ++nTotal;
    {
        const Helix helix({0, 0}, 1., 0., 0., 1.);
        const Point3D point{5., 5., 5.};
        const auto result{POCA(helix, point, 1., 1.)}; // equal bounds
        const bool ok{not result.has_value()};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 4.5 (phiLow==phiUp): {} (result is nullopt: {})",
                ok ? "PASS" : "FAIL", not result.has_value() ? "yes" : "no");
    }

    // ---- Case 4.6: Brent non-convergence with insufficient maxIter ----
    ++nTotal;
    {
        const Helix helix({0, 0}, 1., 0., 0., 1.5);
        const Point3D point{2., 0., 0.};
        const auto result{POCA(helix, point, -pi / 2, pi / 2, 0, 3)}; // nTrialPts=0, maxIter=3
        const bool ok{not result.has_value()};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 4.6 (Brent non-convergence): {} (result is nullopt: {})",
                ok ? "PASS" : "FAIL", not result.has_value() ? "yes" : "no");
    }

    // ---- Case 4.7: multimodal function triggers grid search ----
    ++nTotal;
    {
        const Helix helix({0, 0}, 1., 0., 0., 49 * pi / 100);
        const Point3D point{20., 0., 0.};
        const auto result{POCA(helix, point, -10, 10)};
        // With cot small and wide interval, hMax (=rAB=20) >> 2*d, so grid search is triggered
        // Closest approach: point (20,0,0) to helix point near phi=0: (1,0,0), doca ≈ 19
        const bool ok{result.has_value() and
                      muc::isclose(result->doca, 19.) and
                      onHelix(helix, result->poca)};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 4.7 (multimodal w/ grid search): {} (doca={})",
                ok ? "PASS" : "FAIL",
                result.has_value() ? result->doca : -1.);
    }

    // ---- Timing ----
    constexpr auto testCount{10'000};
    PrintLn("  Timing ({} loops, using Case 4.2 geometry with perturbation):", testCount);
    const Helix tHelix({0, 0}, 1., 0., 0., 1.5);
    Point3D tPoint{2., 0., 0.};
    volatile double dummyDoca{};
    const auto computePOCA{[&]() {
        auto r{POCA(tHelix, tPoint, -pi / 2, pi / 2)};
        if (r) {
            dummyDoca += r->doca;
        }
        tPoint[0] += 1e-9;
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
