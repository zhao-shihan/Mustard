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
#include "Mustard/Testing/TestStatistic/TestStatisticCommon.h++"
#include "Mustard/Testing/TestStatisticalTest/TestStatisticalTest.h++"
#include "Mustard/Testing/TestStatisticalTest/TestStatisticalTestCommon.h++"

#include <cstdlib>

namespace Mustard::Testing {

TestStatisticalZTest::TestStatisticalZTest() :
    Subprogram{"TestStatisticalZTest", "Test Mustard::Math::StatisticalZTest."} {}

namespace TestStatisticalZTestSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;
using namespace Mustard::Testing::TestStatisticalTest;

// =========================================================================
// Helpers
// =========================================================================

auto FillStatWithTwoValues(auto& stat, double a, double b, int n) -> void {
    for (auto i{0}; i < n / 2; ++i) {
        stat.Fill(a);
        stat.Fill(b);
    }
    if (n % 2) {
        stat.Fill(a);
    }
}

// =========================================================================
// Smoke: All constructor combinations
// =========================================================================

constexpr auto secSmoke{[] {
    [[maybe_unused]] double scalar{};

    constexpr auto full{CovarianceOption::Full};
    constexpr auto diag{CovarianceOption::Diagonal};
    const auto x{MakeTestValue<1, full>(1)};
    const auto covF{MakeTestCov<1, full>(1)};
    const auto covD{MakeTestCov<1, diag>(1)};
    auto estF{MakeEstimate<1, full>(x, covF)};
    auto estD{MakeEstimate<1, diag>(x, covD)};

    auto statF{TestStatistic::MakeStatistic<1, full>(1)};
    FillStatWithTwoValues(statF, 4.0, 6.0, 100);
    auto statD{TestStatistic::MakeStatistic<1, diag>(1)};
    FillStatWithTwoValues(statD, 4.0, 6.0, 100);

    // ---- Two-sample from Statistic ----
    {
        StatisticalZTest zs{statF, statF};
        scalar = zs.Value();
    }
    {
        StatisticalZTest zs{statD, statD};
        scalar = zs.Value();
    }

    // ---- Two-sample from Estimate ----
    {
        StatisticalZTest zs{estF, estF};
        scalar = zs.Value();
    }
    {
        StatisticalZTest zs{estD, estD};
        scalar = zs.Value();
    }

    // ---- One-sample from Statistic ----
    {
        StatisticalZTest zs{statF, 5.0};
        scalar = zs.Value();
    }
    {
        StatisticalZTest zs{statD, 5.0};
        scalar = zs.Value();
    }

    // ---- One-sample from Estimate ----
    {
        StatisticalZTest zs{estF, 5.0};
        scalar = zs.Value();
    }
    {
        StatisticalZTest zs{estD, 5.0};
        scalar = zs.Value();
    }

    // ---- Access all methods ----
    {
        StatisticalZTest zs{estF, 5.0};
        scalar = zs.Value();
        scalar = zs.LeftTailP();
        scalar = zs.RightTailP();
        scalar = zs.TwoSidedP();
        scalar = zs.LeftTailSignificance();
        scalar = zs.RightTailSignificance();
        scalar = zs.TwoSidedSignificance();
    }
}};

// =========================================================================
// Section 1: Two-sample identical → z=0
// =========================================================================

constexpr auto sec1Identical{[] {
    constexpr auto full{CovarianceOption::Full};
    const auto est{MakeEstimate<1, full>(MakeTestValue<1, full>(1),
                                         MakeTestCov<1, full>(1))};
    StatisticalZTest z{est, est};
    CheckClose(z.Value(), 0.0, "1: z for identical estimates");
    CheckClose(z.LeftTailP(), 0.5, "1: LeftTailP for identical");
    CheckClose(z.RightTailP(), 0.5, "1: RightTailP for identical");
    CheckClose(z.TwoSidedP(), 1.0, "1: TwoSidedP for identical");
    CheckClose(z.LeftTailSignificance(), 0.0, "1: LeftTailSignificance == 0");
    CheckClose(z.RightTailSignificance(), 0.0, "1: RightTailSignificance == 0");
    CheckClose(z.TwoSidedSignificance(), 0.0, "1: TwoSidedSignificance == 0");
}};

// =========================================================================
// Section 2: Two-sample different → z≠0
// =========================================================================

constexpr auto sec2Different{[] {
    constexpr auto full{CovarianceOption::Full};
    auto x1{MakeTestValue<1, full>(1)};
    x1(0) = 10.0;
    auto est1{MakeEstimate<1, full>(x1, MakeTestCov<1, full>(1))};

    auto x2{MakeTestValue<1, full>(1)};
    x2[0] = 5.0;
    auto est2{MakeEstimate<1, full>(x2, MakeTestCov<1, full>(1))};

    StatisticalZTest z{est1, est2};
    CheckEq(z.Value() > 0, true, "2: z > 0 for est1 > est2");
    CheckBetween(z.RightTailP(), 0.0, 0.5, "2: RightTailP < 0.5");
}};

// =========================================================================
// Section 3: One-sample no effect
// =========================================================================

constexpr auto sec3OneSample{[] {
    constexpr auto full{CovarianceOption::Full};
    const auto est{MakeEstimate<1, full>(MakeTestValue<1, full>(1),
                                         MakeTestCov<1, full>(1))};
    StatisticalZTest z{est, est.Value()};
    CheckClose(z.Value(), 0.0, "3: z for no effect");
}};

// =========================================================================
// Section 4: P-value properties and exact significance
// =========================================================================

constexpr auto sec4Props{[] {
    constexpr auto full{CovarianceOption::Full};
    auto x1{MakeTestValue<1, full>(1)};
    x1(0) = 10.0;
    auto est1{MakeEstimate<1, full>(x1, MakeTestCov<1, full>(1))};
    auto x2{MakeTestValue<1, full>(1)};
    x2[0] = 5.0;
    auto est2{MakeEstimate<1, full>(x2, MakeTestCov<1, full>(1))};

    StatisticalZTest z{est1, est2};

    // P-value properties
    CheckClose(z.LeftTailP() + z.RightTailP(), 1.0, "4: LeftTailP + RightTailP = 1");
    CheckBetween(z.LeftTailP(), 0.0, 1.0, "4: LeftTailP in [0,1]");
    CheckBetween(z.RightTailP(), 0.0, 1.0, "4: RightTailP in [0,1]");
    CheckBetween(z.TwoSidedP(), 0.0, 1.0, "4: TwoSidedP in [0,1]");

    // Exact significance equalities
    CheckClose(z.LeftTailSignificance(), -z.Value(), "4: LeftTailSignificance == -z");
    CheckClose(z.RightTailSignificance(), z.Value(), "4: RightTailSignificance == z");
    CheckClose(z.TwoSidedSignificance(), std::abs(z.Value()), "4: TwoSidedSignificance == |z|");
}};

// =========================================================================
// Section 5: Statistic delegation
// =========================================================================

constexpr auto sec5Delegation{[] {
    constexpr auto full{CovarianceOption::Full};

    auto stat1{TestStatistic::MakeStatistic<1, full>(1)};
    FillStatWithTwoValues(stat1, 4.0, 6.0, 50);
    auto stat2{TestStatistic::MakeStatistic<1, full>(1)};
    FillStatWithTwoValues(stat2, 4.5, 5.5, 50);

    StatisticalZTest zs{stat1, stat2};
    StatisticalZTest ze{stat1.MeanEstimate(), stat2.MeanEstimate()};

    CheckClose(zs.Value(), ze.Value(), "5: z from Stat vs Est match");

    // One-sample
    StatisticalZTest ozs{stat1, 5.0};
    StatisticalZTest oze{stat1.MeanEstimate(), 5.0};
    CheckClose(ozs.Value(), oze.Value(), "5: one-sample z from Stat vs Est match");
}};

} // namespace TestStatisticalZTestSection

auto TestStatisticalZTest::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestStatisticalZTestSection;

    PrintLn("--- Smoke: All constructor combinations ---");
    secSmoke();
    PrintLn("  smoke passed: all constructor combinations compile and accessors work");

    PrintLn("--- Section 1: Two-sample identical → z=0 ---");
    sec1Identical();
    PrintLn("  1 passed: identical estimates give z=0, p=0.5");

    PrintLn("--- Section 2: Two-sample different → z≠0 ---");
    sec2Different();
    PrintLn("  2 passed: different estimates give z≠0, p<0.5");

    PrintLn("--- Section 3: One-sample no effect ---");
    sec3OneSample();
    PrintLn("  3 passed: estimate matches mu gives z=0");

    PrintLn("--- Section 4: P-value properties and exact significance ---");
    sec4Props();
    PrintLn("  4 passed: p-value range, LTP+RTP=1, exact significance equalities");

    PrintLn("--- Section 5: Statistic delegation ---");
    sec5Delegation();
    PrintLn("  5 passed: Statistic constructor matches Estimate constructor");

    PrintLn("All TestStatisticalZTest tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
