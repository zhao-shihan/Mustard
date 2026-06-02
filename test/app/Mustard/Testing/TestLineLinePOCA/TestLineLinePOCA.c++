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
#include "Mustard/Testing/TestLineLinePOCA/TestLineLinePOCA.h++"

#include "muc/chrono"
#include "muc/numeric"

#include <cmath>
#include <cstdlib>

namespace Mustard::Testing {

TestLineLinePOCA::TestLineLinePOCA() :
    Subprogram{"TestLineLinePOCA", "Test and benchmark Line-Line POCA computation."} {}

auto TestLineLinePOCA::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace Mustard::Math;

    int nPassed{};
    int nTotal{};

    const auto isClose3D{[](const auto& a, const auto& b) -> bool {
        return muc::isclose(a.x(), b.x()) and
               muc::isclose(a.y(), b.y()) and
               muc::isclose(a.z(), b.z());
    }};

    const auto pointOnLine{[](const Line3D& line, const Point3D& p) -> bool {
        const auto d{(p - line.point).cross(line.direction)};
        return muc::isclose(d.mag(), 0.);
    }};

    PrintLn("=== TestLineLinePOCA ===");

    // ---- Case 2.1: Intersecting lines ----
    ++nTotal;
    {
        const Line3D line1({1, 0, 0}, {0, 1, 0}); // y-axis through x=1, z=0
        const Line3D line2({0, 1, 0}, {1, 0, 0}); // x-axis through y=1, z=0
        const auto result{POCA(line1, line2)};
        const bool ok{isClose3D(result.poca1, Point3D{1, 1, 0}) and
                      isClose3D(result.poca2, Point3D{1, 1, 0}) and
                      muc::isclose(result.doca, 0.) and
                      pointOnLine(line1, result.poca1) and
                      pointOnLine(line2, result.poca2)};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 2.1 (intersecting lines): {} (poca1=({},{},{}) poca2=({},{},{}) doca={})",
                ok ? "PASS" : "FAIL", result.poca1.x(), result.poca1.y(), result.poca1.z(),
                result.poca2.x(), result.poca2.y(), result.poca2.z(), result.doca);
    }

    // ---- Case 2.2: Skew lines (classic skew) ----
    ++nTotal;
    {
        const Line3D line1({0, 0, 0}, {1, 0, 0}); // x-axis in z=0 plane
        const Line3D line2({0, 0, 1}, {0, 1, 0}); // y-axis in z=1 plane
        const auto result{POCA(line1, line2)};
        const bool ok{isClose3D(result.poca1, Point3D{0, 0, 0}) and
                      isClose3D(result.poca2, Point3D{0, 0, 1}) and
                      muc::isclose(result.doca, 1.) and
                      pointOnLine(line1, result.poca1) and
                      pointOnLine(line2, result.poca2)};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 2.2 (skew lines): {} (poca1=({},{},{}) poca2=({},{},{}) doca={})",
                ok ? "PASS" : "FAIL", result.poca1.x(), result.poca1.y(), result.poca1.z(),
                result.poca2.x(), result.poca2.y(), result.poca2.z(), result.doca);
    }

    // ---- Case 2.3: Parallel lines ----
    ++nTotal;
    {
        const Line3D line1({0, 0, 0}, {1, 0, 0});
        const Line3D line2({0, 5, 0}, {3, 0, 0}); // parallel to x-axis, offset y=5
        const auto result{POCA(line1, line2)};
        // parallel branch: falls back to POCA(line2, line1.point)
        // projects (0,0,0) onto line2: t = ((0,0,0)-(0,5,0)).(3,0,0)/9 = 0, poca2=(0,5,0)
        const bool ok{isClose3D(result.poca1, Point3D{0, 0, 0}) and
                      isClose3D(result.poca2, Point3D{0, 5, 0}) and
                      muc::isclose(result.doca, 5.) and
                      pointOnLine(line1, result.poca1) and
                      pointOnLine(line2, result.poca2)};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 2.3 (parallel lines): {} (poca1=({},{},{}) poca2=({},{},{}) doca={})",
                ok ? "PASS" : "FAIL", result.poca1.x(), result.poca1.y(), result.poca1.z(),
                result.poca2.x(), result.poca2.y(), result.poca2.z(), result.doca);
    }

    // ---- Case 2.4: Degenerate line2 direction ----
    ++nTotal;
    {
        const Line3D line1({0, 0, 0}, {1, 0, 0});
        const Line3D line2({3, 4, 5}, {0, 0, 0}); // degenerate direction
        const auto result{POCA(line1, line2)};
        // falls back to POCA(line1, line2.point): project (3,4,5) onto x-axis -> (3,0,0)
        const double expectedDoca{std::sqrt(16. + 25.)}; // sqrt(41)
        const bool ok{isClose3D(result.poca1, Point3D{3, 0, 0}) and
                      isClose3D(result.poca2, Point3D{3, 4, 5}) and
                      muc::isclose(result.doca, expectedDoca) and
                      pointOnLine(line1, result.poca1)};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 2.4 (degenerate line2): {} (poca1=({},{},{}) poca2=({},{},{}) doca={})",
                ok ? "PASS" : "FAIL", result.poca1.x(), result.poca1.y(), result.poca1.z(),
                result.poca2.x(), result.poca2.y(), result.poca2.z(), result.doca);
    }

    // ---- Case 2.5: General skew (x-axis vs z-axis through (0,1,2)) ----
    ++nTotal;
    {
        const Line3D line1({0, 0, 0}, {1, 0, 0}); // x-axis
        const Line3D line2({0, 1, 2}, {0, 0, 1}); // z-axis through (0,1,2)
        const auto result{POCA(line1, line2)};
        // d1d1=1, d2d2=1, d1d2=0, denom=1
        // x12=(0,1,2), x12d1=0, x12d2=2, t1=0, t2=-2
        // poca1=(0,0,0), poca2=(0,1,2)+(-2)*(0,0,1)=(0,1,0)
        // doca=|(0,1,0)-(0,0,0)| = 1
        const bool ok{isClose3D(result.poca1, Point3D{0, 0, 0}) and
                      isClose3D(result.poca2, Point3D{0, 1, 0}) and
                      muc::isclose(result.doca, 1.) and
                      pointOnLine(line1, result.poca1) and
                      pointOnLine(line2, result.poca2)};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 2.5 (general skew): {} (poca1=({},{},{}) poca2=({},{},{}) doca={})",
                ok ? "PASS" : "FAIL", result.poca1.x(), result.poca1.y(), result.poca1.z(),
                result.poca2.x(), result.poca2.y(), result.poca2.z(), result.doca);
    }

    // ---- Timing ----
    constexpr auto testCount{1'000'000};
    PrintLn("  Timing ({} loops using Case 2.2 geometry):", testCount);
    Line3D tLine1({0, 0, 0}, {1, 0, 0});
    const Line3D tLine2({0, 0, 1}, {0, 1, 0});
    volatile double dummyDoca{};
    const auto computePOCA{[&]() {
        auto r{POCA(tLine1, tLine2)};
        dummyDoca += r.doca;
        tLine1.point[0] += 1e-9;
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
