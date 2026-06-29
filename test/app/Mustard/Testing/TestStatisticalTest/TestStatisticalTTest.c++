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
#include "Mustard/Testing/TestStatistic/TestStatisticCommon.h++"
#include "Mustard/Testing/TestStatisticalTest/TestStatisticalTest.h++"
#include "Mustard/Testing/TestStatisticalTest/TestStatisticalTestCommon.h++"

#include <cstdlib>

namespace Mustard::Testing {

TestStatisticalTTest::TestStatisticalTTest() :
    Subprogram{"TestStatisticalTTest", "Test Mustard::Math::StatisticalTTest."} {}

namespace TestStatisticalTTestSection {

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

    // Build Estimates
    constexpr auto full{CovarianceOption::Full};
    constexpr auto diag{CovarianceOption::Diagonal};
    const auto x{MakeTestValue<1, full>(1)};
    const auto covF{MakeTestCov<1, full>(1)};
    const auto covD{MakeTestCov<1, diag>(1)};
    auto estF{MakeEstimate<1, full>(x, covF)};
    auto estD{MakeEstimate<1, diag>(x, covD)};

    // Build Statistics
    auto statF{TestStatistic::MakeStatistic<1, full>(1)};
    FillStatWithTwoValues(statF, 4.0, 6.0, 100);
    auto statD{TestStatistic::MakeStatistic<1, diag>(1)};
    FillStatWithTwoValues(statD, 4.0, 6.0, 100);

    // ---- Two-sample from Statistic ----
    {
        StatisticalTTest ts{statF, statF};
        scalar = ts.Value();
        scalar = ts.NDF();
    }
    {
        StatisticalTTest ts{statD, statD};
        scalar = ts.Value();
    }

    // ---- Two-sample from Estimate ----
    {
        StatisticalTTest ts{estF, 100.0, estF, 100.0};
        scalar = ts.Value();
    }
    {
        StatisticalTTest ts{estD, 100.0, estD, 100.0};
        scalar = ts.Value();
    }

    // ---- One-sample from Statistic ----
    {
        StatisticalTTest ts{statF, 5.0};
        scalar = ts.Value();
    }
    {
        StatisticalTTest ts{statD, 5.0};
        scalar = ts.Value();
    }

    // ---- One-sample from Estimate ----
    {
        StatisticalTTest ts{estF, 10.0, 5.0};
        scalar = ts.Value();
    }
    {
        StatisticalTTest ts{estD, 10.0, 5.0};
        scalar = ts.Value();
    }

    // ---- Access all methods ----
    {
        StatisticalTTest ts{estF, 10.0, 5.0};
        scalar = ts.Value();
        scalar = ts.NDF();
        scalar = ts.LeftTailP();
        scalar = ts.RightTailP();
        scalar = ts.TwoSidedP();
        scalar = ts.LeftTailSignificance();
        scalar = ts.RightTailSignificance();
        scalar = ts.TwoSidedSignificance();
    }
}};

// =========================================================================
// Section 1: Constructor validation
// =========================================================================

constexpr auto sec1Validation{[] {
    constexpr auto full{CovarianceOption::Full};
    const auto est{MakeEstimate<1, full>(MakeTestValue<1, full>(1),
                                         MakeTestCov<1, full>(1))};

    // Two-sample: n1 <= 1 throws
    CheckThrows("1: n1=0", [&] { StatisticalTTest{est, 0.0, est, 2.0}; });
    CheckThrows("1: n1=1", [&] { StatisticalTTest{est, 1.0, est, 2.0}; });

    // Two-sample: n2 <= 1 throws
    CheckThrows("1: n2=0", [&] { StatisticalTTest{est, 2.0, est, 0.0}; });
    CheckThrows("1: n2=1", [&] { StatisticalTTest{est, 2.0, est, 1.0}; });

    // Two-sample: n1=2, n2=2 is valid
    {
        auto threw{false};
        try {
            StatisticalTTest{est, 2.0, est, 2.0};
        } catch (...) { threw = true; }
        if (threw) {
            Throw<std::runtime_error>("1: n1=n2=2 should not throw");
        }
    }

    // One-sample: n <= 1 throws
    CheckThrows("1: one-sample n=0", [&] { StatisticalTTest{est, 0.0, 5.0}; });
    CheckThrows("1: one-sample n=1", [&] { StatisticalTTest{est, 1.0, 5.0}; });

    // One-sample: n=2 is valid
    {
        auto threw{false};
        try {
            StatisticalTTest{est, 2.0, 5.0};
        } catch (...) { threw = true; }
        if (threw) {
            Throw<std::runtime_error>("1: one-sample n=2 should not throw");
        }
    }
}};

// =========================================================================
// Section 2: Two-sample identical → no effect
// =========================================================================

constexpr auto sec2Identical{[] {
    constexpr auto full{CovarianceOption::Full};
    const auto est{MakeEstimate<1, full>(MakeTestValue<1, full>(1),
                                         MakeTestCov<1, full>(1))};
    StatisticalTTest t{est, 100.0, est, 100.0};
    CheckClose(t.Value(), 0.0, "2: t for identical estimates");
    CheckClose(t.LeftTailP(), 0.5, "2: LeftTailP for identical");
    CheckClose(t.RightTailP(), 0.5, "2: RightTailP for identical");
    CheckBetween(t.TwoSidedP(), 0.99, 1.01, "2: TwoSidedP for identical");
    CheckClose(t.LeftTailSignificance(), 0.0, "2: LeftTailSignificance ≈ 0 for identical");
    CheckClose(t.RightTailSignificance(), 0.0, "2: RightTailSignificance ≈ 0 for identical");
}};

// =========================================================================
// Section 3: Two-sample different → effect
// =========================================================================

constexpr auto sec3Different{[] {
    constexpr auto full{CovarianceOption::Full};
    auto x1{MakeTestValue<1, full>(1)};
    x1(0) = 10.0;
    const auto cov1{MakeTestCov<1, full>(1)};
    auto est1{MakeEstimate<1, full>(x1, cov1)};

    auto x2{MakeTestValue<1, full>(1)};
    x2(0) = 5.0;
    const auto cov2{MakeTestCov<1, full>(1)};
    auto est2{MakeEstimate<1, full>(x2, cov2)};

    StatisticalTTest t{est1, 30.0, est2, 30.0};
    CheckEq(t.Value() > 0, true, "3: t > 0 for est1 > est2");
    CheckBetween(t.RightTailP(), 0.0, 0.5, "3: RightTailP < 0.5");
    CheckNonNegative(t.NDF(), "3: NDF non-negative");
}};

// =========================================================================
// Section 4: One-sample no effect
// =========================================================================

constexpr auto sec4OneSample{[] {
    constexpr auto full{CovarianceOption::Full};
    const auto est{MakeEstimate<1, full>(MakeTestValue<1, full>(1),
                                         MakeTestCov<1, full>(1))};
    StatisticalTTest t{est, 10.0, est.Value()};
    CheckClose(t.Value(), 0.0, "4: t for no effect");
    CheckEq(t.NDF(), 9.0, "4: NDF == n-1");
}};

// =========================================================================
// Section 5: P-value properties
// =========================================================================

constexpr auto sec5PValueProps{[] {
    constexpr auto full{CovarianceOption::Full};
    auto x1{MakeTestValue<1, full>(1)};
    x1(0) = 10.0;
    auto est1{MakeEstimate<1, full>(x1, MakeTestCov<1, full>(1))};
    auto x2{MakeTestValue<1, full>(1)};
    x2(0) = 5.0;
    auto est2{MakeEstimate<1, full>(x2, MakeTestCov<1, full>(1))};

    StatisticalTTest t{est1, 30.0, est2, 30.0};

    CheckClose(t.LeftTailP() + t.RightTailP(), 1.0, "5: LeftTailP + RightTailP = 1");
    CheckClose(t.TwoSidedP(), 2.0 * std::min(t.LeftTailP(), t.RightTailP()),
               "5: TwoSidedP == 2 * min(LTP, RTP)");
    CheckBetween(t.LeftTailP(), 0.0, 1.0, "5: LeftTailP in [0,1]");
    CheckBetween(t.RightTailP(), 0.0, 1.0, "5: RightTailP in [0,1]");
    CheckBetween(t.TwoSidedP(), 0.0, 1.0, "5: TwoSidedP in [0,1]");
    CheckClose(t.LeftTailSignificance(), -t.RightTailSignificance(), "5: LeftTailSig = -RightTailSig");
}};

// =========================================================================
// Section 6: Significance sign
// =========================================================================

constexpr auto sec6Significance{[] {
    constexpr auto full{CovarianceOption::Full};
    auto x1{MakeTestValue<1, full>(1)};
    x1(0) = 10.0;
    auto est1{MakeEstimate<1, full>(x1, MakeTestCov<1, full>(1))};
    auto x2{MakeTestValue<1, full>(1)};
    x2(0) = 5.0;
    auto est2{MakeEstimate<1, full>(x2, MakeTestCov<1, full>(1))};

    // t > 0 case
    {
        StatisticalTTest t{est1, 30.0, est2, 30.0};
        CheckEq(t.RightTailSignificance() > 0, true, "6: RightTailSignificance > 0 for t>0");
        CheckEq(t.LeftTailSignificance() < 0, true, "6: LeftTailSignificance < 0 for t>0");
        CheckEq(t.TwoSidedSignificance() > 0, true, "6: TwoSidedSignificance > 0 for t>0");
    }

    // t < 0 case (swap est1 and est2)
    {
        StatisticalTTest t{est2, 30.0, est1, 30.0};
        CheckEq(t.RightTailSignificance() < 0, true, "6: RightTailSignificance < 0 for t<0");
        CheckEq(t.LeftTailSignificance() > 0, true, "6: LeftTailSignificance > 0 for t<0");
        CheckEq(t.TwoSidedSignificance() > 0, true, "6: TwoSidedSignificance > 0 for t<0");
    }
}};

// =========================================================================
// Section 7: Statistic delegation
// =========================================================================

constexpr auto sec7Delegation{[] {
    constexpr auto full{CovarianceOption::Full};

    // Build two Statistics with known data
    auto stat1{TestStatistic::MakeStatistic<1, full>(1)};
    FillStatWithTwoValues(stat1, 4.0, 6.0, 50);
    auto stat2{TestStatistic::MakeStatistic<1, full>(1)};
    FillStatWithTwoValues(stat2, 4.5, 5.5, 50);

    // Via Statistic constructor
    StatisticalTTest ts{stat1, stat2};

    // Via Estimate constructor with equivalent inputs
    auto n1{stat1.EffectiveSampleSize()};
    auto n2{stat2.EffectiveSampleSize()};
    StatisticalTTest te{stat1.MeanEstimate(), n1,
                        stat2.MeanEstimate(), n2};

    CheckClose(ts.Value(), te.Value(), "7: t from Stat vs Est match");
    CheckClose(ts.NDF(), te.NDF(), "7: NDF from Stat vs Est match");

    // One-sample delegation
    StatisticalTTest os{stat1, 5.0};
    StatisticalTTest oe{stat1.MeanEstimate(), n1, 5.0};
    CheckClose(os.Value(), oe.Value(), "7: one-sample t from Stat vs Est match");
    CheckClose(os.NDF(), oe.NDF(), "7: one-sample NDF from Stat vs Est match");
}};

} // namespace TestStatisticalTTestSection

auto TestStatisticalTTest::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestStatisticalTTestSection;

    PrintLn("--- Smoke: All constructor combinations ---");
    secSmoke();
    PrintLn("  smoke passed: all constructor combinations compile and accessors work");

    PrintLn("--- Section 1: Constructor validation ---");
    sec1Validation();
    PrintLn("  1 passed: invalid sample sizes throw, valid ones succeed");

    PrintLn("--- Section 2: Two-sample identical → no effect ---");
    sec2Identical();
    PrintLn("  2 passed: identical estimates give t≈0, p≈0.5");

    PrintLn("--- Section 3: Two-sample different → effect ---");
    sec3Different();
    PrintLn("  3 passed: different estimates give t≠0, p<0.5");

    PrintLn("--- Section 4: One-sample no effect ---");
    sec4OneSample();
    PrintLn("  4 passed: estimate matches mu gives t≈0, NDF=n-1");

    PrintLn("--- Section 5: P-value properties ---");
    sec5PValueProps();
    PrintLn("  5 passed: p-value symmetry and range checks");

    PrintLn("--- Section 6: Significance sign ---");
    sec6Significance();
    PrintLn("  6 passed: significance signs match direction");

    PrintLn("--- Section 7: Statistic delegation ---");
    sec7Delegation();
    PrintLn("  7 passed: Statistic constructor matches Estimate constructor");

    PrintLn("All TestStatisticalTTest tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
