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
#include "Mustard/Testing/TestStatistic/TestStatistic.h++"
#include "Mustard/Testing/TestStatistic/TestStatisticCommon.h++"

#include <cstdlib>

namespace Mustard::Testing {

TestStatistic9::TestStatistic9() :
    Subprogram{"TestStatistic9", "Test Mustard::Statistic (Section 9: Component Consistency)."} {}

namespace TestStatisticSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestStatistic;

// =========================================================================
// Section 9 Smoke: Component Consistency
// =========================================================================

constexpr auto sec9Smoke{[] {
    Statistic<1> s1;
    Statistic<2> s2;

    s1.Fill(2.0);
    s2.Fill(Eigen::Vector2d{2.0, 0.0});

    [[maybe_unused]] auto m1{s1.Mean()};
    [[maybe_unused]] auto m2{s2.Mean(0)};
    [[maybe_unused]] auto v1{s1.Variance()};
    [[maybe_unused]] auto v2{s2.Variance(0)};
}};

// =========================================================================
// Section 9: Component Consistency
// N=1 component of Statistic<2> matches Statistic<1>
// =========================================================================

constexpr auto sec9ComponentConsistency{[] {
    Statistic<1> s1;
    Statistic<2> s2;
    const std::array values{2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0};
    for (auto v : values) {
        s1.Fill(v);
        s2.Fill(Eigen::Vector2d{v, 0.0});
    }

    CheckClose(s2.Mean(0), s1.Mean(), "9: Mean(0) matches scalar Statistic");
    CheckClose(s2.Variance(0), s1.Variance(), "9: Variance(0) matches scalar Statistic");
    CheckClose(s2.Mean(1), 0.0, "9: Mean(1) is zero");
    CheckClose(s2.Variance(1), 0.0, "9: Variance(1) is zero");
}};

} // namespace TestStatisticSection

auto TestStatistic9::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestStatisticSection;

    PrintLn("--- Section 9: Smoke Test ---");
    sec9Smoke();
    PrintLn("  9 smoke passed: component consistency compilation smoke");

    PrintLn("--- Section 9: Component Consistency ---");
    sec9ComponentConsistency();
    PrintLn("  9 passed: component consistency (K=1 vs K=2)");

    PrintLn("All TestStatistic9 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
