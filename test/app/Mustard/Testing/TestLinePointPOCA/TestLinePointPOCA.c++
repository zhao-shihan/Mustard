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
#include "Mustard/Testing/TestLinePointPOCA/TestLinePointPOCA.h++"

#include "muc/chrono"
#include "muc/numeric"

#include <cmath>
#include <cstdlib>

namespace Mustard::Testing {

TestLinePointPOCA::TestLinePointPOCA() :
    Subprogram{"TestLinePointPOCA", "Test and benchmark Line-Point POCA computation."} {}

auto TestLinePointPOCA::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace Mustard::Math;

    int nPassed{};
    int nTotal{};

    // Helper: component-wise 3D closeness check
    const auto isClose3D{[](const auto& a, const auto& b) -> bool {
        return muc::isclose(a.x(), b.x()) and
               muc::isclose(a.y(), b.y()) and
               muc::isclose(a.z(), b.z());
    }};

    // helper to verify poca lies on the line
    const auto pointOnLine{[](const Line3D& line, const Point3D& p) -> bool {
        const auto d{(p - line.point).cross(line.direction)};
        return muc::isclose(d.mag(), 0.);
    }};

    PrintLn("=== TestLinePointPOCA ===");

    // ---- Case 1.1: General case ----
    ++nTotal;
    {
        const Line3D line({1, 0, 0}, {0, 1, 0});
        const Point3D point{1, 3, 4};
        const auto result{POCA(line, point)};
        const bool ok{isClose3D(result.poca, Point3D{1, 3, 0}) and
                      muc::isclose(result.doca, 4.) and
                      muc::isclose((result.poca - point).mag(), result.doca) and
                      pointOnLine(line, result.poca)};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 1.1 (general case): {} (poca=({},{},{}) doca={})",
                ok ? "PASS" : "FAIL", result.poca.x(), result.poca.y(), result.poca.z(), result.doca);
    }

    // ---- Case 1.2: Point on the line ----
    ++nTotal;
    {
        const Line3D line({0, 0, 0}, {1, 0, 0});
        const Point3D point{5, 0, 0};
        const auto result{POCA(line, point)};
        const bool ok{isClose3D(result.poca, point) and
                      muc::isclose(result.doca, 0.) and
                      pointOnLine(line, result.poca)};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 1.2 (point on line): {} (poca=({},{},{}) doca={})",
                ok ? "PASS" : "FAIL", result.poca.x(), result.poca.y(), result.poca.z(), result.doca);
    }

    // ---- Case 1.3: Degenerate direction ----
    ++nTotal;
    {
        const Line3D line({2, 3, 4}, {0, 0, 0});
        const Point3D point{5, 7, 9};
        const auto result{POCA(line, point)};
        const double expectedDoca{std::sqrt(9. + 16. + 25.)}; // sqrt(50)
        const bool ok{isClose3D(result.poca, Point3D{2, 3, 4}) and
                      muc::isclose(result.doca, expectedDoca)};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 1.3 (degenerate direction): {} (poca=({},{},{}) doca={})",
                ok ? "PASS" : "FAIL", result.poca.x(), result.poca.y(), result.poca.z(), result.doca);
    }

    // ---- Case 1.4: Non-unit direction vector ----
    ++nTotal;
    {
        const Line3D line({0, 0, 0}, {2, 4, 6});
        const Point3D point{1, 2, 3}; // on line at t=0.5
        const auto result{POCA(line, point)};
        const bool ok{isClose3D(result.poca, point) and
                      muc::isclose(result.doca, 0.)};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 1.4 (non-unit direction): {} (poca=({},{},{}) doca={})",
                ok ? "PASS" : "FAIL", result.poca.x(), result.poca.y(), result.poca.z(), result.doca);
    }

    // ---- Case 1.5: General 3D skew ----
    ++nTotal;
    {
        const Line3D line({1, 2, 3}, {1, 1, 1});
        const Point3D point{5, 0, 1};
        const auto result{POCA(line, point)};
        // x12=(4,-2,-2), d1d1=3, t=(4-2-2)/3=0, poca=(1,2,3)
        const double expectedDoca{std::sqrt(24.)}; // sqrt(16+4+4)
        const bool ok{isClose3D(result.poca, Point3D{1, 2, 3}) and
                      muc::isclose(result.doca, expectedDoca) and
                      pointOnLine(line, result.poca)};
        if (ok) {
            ++nPassed;
        }
        PrintLn("  Case 1.5 (general 3D skew): {} (poca=({},{},{}) doca={})",
                ok ? "PASS" : "FAIL", result.poca.x(), result.poca.y(), result.poca.z(), result.doca);
    }

    // ---- Timing ----
    constexpr auto testCount{1'000'000};
    PrintLn("  Timing ({} loops using Case 1.1 geometry):", testCount);
    const Line3D tLine({1, 0, 0}, {0, 1, 0});
    Point3D tPoint{1, 3, 4};
    volatile double dummyDoca{};
    const auto computePOCA{[&]() {
        auto r{POCA(tLine, tPoint)};
        dummyDoca += r.doca;
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
