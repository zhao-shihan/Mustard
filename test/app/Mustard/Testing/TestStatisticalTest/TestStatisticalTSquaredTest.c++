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

TestStatisticalTSquaredTest::TestStatisticalTSquaredTest() :
    Subprogram{"TestStatisticalTSquaredTest", "Test Mustard::Math::StatisticalTSquaredTest."} {}

namespace TestStatisticalTSquaredTestSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;
using namespace Mustard::Testing::TestStatisticalTest;

// Dimensions for multivariate tests (K >= 1)
using MultiDims = std::integer_sequence<int, 2, 3, 5>;

// =========================================================================
// Smoke: All constructor combinations over multiple K/C
// =========================================================================

constexpr auto secSmoke{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    constexpr bool isFull{C == CovarianceOption::Full};
    constexpr CovarianceOption oppositeC{isFull ? CovarianceOption::Diagonal : CovarianceOption::Full};

    [[maybe_unused]] double scalar{};

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    const auto xOpp{MakeTestValue<K, oppositeC>(dim)};
    const auto covOpp{MakeTestCov<K, oppositeC>(dim)};

    auto est{MakeEstimate<K, C>(x, cov)};
    auto estOpp{MakeEstimate<K, oppositeC>(xOpp, covOpp)};

    auto muVec{MakeTestValue<K, C>(dim)};

    // ---- Two-sample: same K, same C ----
    {
        StatisticalTSquaredTest t{est, 20.0, est, 20.0};
        scalar = t.Value();
    }

    // ---- Two-sample: same K, different C ----
    {
        StatisticalTSquaredTest t{est, 20.0, estOpp, 20.0};
        scalar = t.Value();
    }

    // ---- One-sample from Estimate ----
    {
        StatisticalTSquaredTest t{est, 20, muVec};
        scalar = t.Value();
    }
    {
        StatisticalTSquaredTest t{est, 20, muVec, 0};
        scalar = t.Value();
    }
    if constexpr (dim >= 2) {
        StatisticalTSquaredTest t{est, 20, muVec, 1};
        scalar = t.Value();
    }

    // ---- One-sample cross-C ----
    {
        StatisticalTSquaredTest t{estOpp, 20, muVec};
        scalar = t.Value();
    }

    // ---- Access all methods ----
    {
        StatisticalTSquaredTest t{est, 20, muVec};
        scalar = t.Value();
        scalar = t.Dimension();
        scalar = t.NDF1();
        scalar = t.NDF2();
        scalar = t.PValue();
        scalar = t.Significance();
    }
}};

constexpr auto secSmokeCrossType{[] {
    [[maybe_unused]] double scalar{};
    constexpr auto full{CovarianceOption::Full};
    constexpr auto diag{CovarianceOption::Diagonal};
    constexpr int dyn{Eigen::Dynamic};

    // A: Static Full × Static Diag (K=2)
    {
        auto eF2{MakeEstimate<2, full>(MakeTestValue<2, full>(2), MakeTestCov<2, full>(2))};
        auto eD2{MakeEstimate<2, diag>(MakeTestValue<2, diag>(2), MakeTestCov<2, diag>(2))};
        StatisticalTSquaredTest t{eF2, 20.0, eD2, 20.0};
        scalar = t.Value();
    }

    // B: Static × Dynamic (K=2)
    {
        auto eS2{MakeEstimate<2, full>(MakeTestValue<2, full>(2), MakeTestCov<2, full>(2))};
        auto eDy2{MakeEstimate<dyn, full>(MakeTestValue<dyn, full>(2), MakeTestCov<dyn, full>(2))};
        StatisticalTSquaredTest t{eS2, 20.0, eDy2, 20.0};
        scalar = t.Value();
    }

    // C: Dynamic × Dynamic different C (dim=2)
    {
        auto eDyF2{MakeEstimate<dyn, full>(MakeTestValue<dyn, full>(2), MakeTestCov<dyn, full>(2))};
        auto eDyD2{MakeEstimate<dyn, diag>(MakeTestValue<dyn, diag>(2), MakeTestCov<dyn, diag>(2))};
        StatisticalTSquaredTest t{eDyF2, 20.0, eDyD2, 20.0};
        scalar = t.Value();
    }

    // D: One-sample Static Full with Dynamic mu
    {
        auto eS2{MakeEstimate<2, full>(MakeTestValue<2, full>(2), MakeTestCov<2, full>(2))};
        Eigen::Vector2d mu;
        mu << 1.0, 2.0;
        StatisticalTSquaredTest t{eS2, 20, mu};
        scalar = t.Value();
    }
}};

// =========================================================================
// Section 1: Constructor validation
// =========================================================================

constexpr auto sec1Validation{[] {
    constexpr auto full{CovarianceOption::Full};
    constexpr int dyn{Eigen::Dynamic};

    // Dimension mismatch (both dynamic for compile-time compatibility)
    auto ed2{MakeEstimate<dyn, full>(MakeTestValue<dyn, full>(2), MakeTestCov<dyn, full>(2))};
    auto ed3{MakeEstimate<dyn, full>(MakeTestValue<dyn, full>(3), MakeTestCov<dyn, full>(3))};
    CheckThrows("1: dim mismatch 2 vs 3", [&] { StatisticalTSquaredTest{ed2, 20.0, ed3, 20.0}; });

    auto e2{MakeEstimate<2, full>(MakeTestValue<2, full>(2), MakeTestCov<2, full>(2))};

    // n1 <= 1 throws
    CheckThrows("1: n1=0", [&] { StatisticalTSquaredTest{e2, 0.0, e2, 20.0}; });
    CheckThrows("1: n1=1", [&] { StatisticalTSquaredTest{e2, 1.0, e2, 20.0}; });

    // n2 <= 1 throws
    CheckThrows("1: n2=0", [&] { StatisticalTSquaredTest{e2, 20.0, e2, 0.0}; });
    CheckThrows("1: n2=1", [&] { StatisticalTSquaredTest{e2, 20.0, e2, 1.0}; });

    // One-sample: n <= 1 throws
    Eigen::Vector2d mu;
    mu << 1.0, 2.0;
    CheckThrows("1: one-sample n=0", [&] { StatisticalTSquaredTest{e2, 0, mu}; });
    CheckThrows("1: one-sample n=1", [&] { StatisticalTSquaredTest{e2, 1, mu}; });

    // nConstraint out of range
    CheckThrows("1: nConstraint = -1", [&] { StatisticalTSquaredTest{e2, 20, mu, -1}; });
    CheckThrows("1: nConstraint = dim", [&] { StatisticalTSquaredTest{e2, 20, mu, 2}; });
}};

// =========================================================================
// Section 2: Two-sample identical → T²≈0
// =========================================================================

constexpr auto sec2Identical{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    auto est{MakeEstimate<K, C>(x, cov)};

    StatisticalTSquaredTest t{est, 100.0, est, 100.0};
    CheckClose(t.Value(), 0.0, "2: T² ≈ 0 for identical estimates");
    CheckBetween(t.PValue(), 0.99, 1.01, "2: PValue ≈ 1 for identical");
    CheckEq(t.Significance() < 0, true, "2: Significance < 0 for PValue≈1");
}};

// =========================================================================
// Section 3: Two-sample different → T²>0
// =========================================================================

constexpr auto sec3Different{[] {
    constexpr auto full{CovarianceOption::Full};

    // Build different means
    Eigen::Vector2d x1, x2;
    x1 << 10.0, 20.0;
    x2 << 5.0, 15.0;

    Eigen::Matrix2d cov;
    cov << 4.0, 1.0, 1.0, 3.0;

    Estimate<2, full> est1{x1, cov};
    Estimate<2, full> est2{x2, cov};

    StatisticalTSquaredTest t{est1, 30.0, est2, 30.0};
    CheckNonNegative(t.Value(), "3: T² non-negative");
    CheckEq(t.Value() > 0, true, "3: T² > 0 for different means");
    CheckBetween(t.PValue(), 0.0, 1.0, "3: PValue in [0,1]");
}};

// =========================================================================
// Section 4: One-sample no effect
// =========================================================================

constexpr auto sec4OneSample{[] {
    constexpr auto full{CovarianceOption::Full};

    Eigen::Vector2d mu;
    mu << 1.0, 2.0;
    auto est{MakeEstimate<2, full>(mu, MakeTestCov<2, full>(2))};

    StatisticalTSquaredTest t{est, 20, mu};
    CheckClose(t.Value(), 0.0, "4: T² ≈ 0 for no effect");
}};

// =========================================================================
// Section 5: nConstraint and NDF
// =========================================================================

constexpr auto sec5NDF{[] {
    constexpr auto full{CovarianceOption::Full};
    constexpr int dim{3};

    Eigen::Vector3d mu;
    mu << 1.0, 2.0, 3.0;
    auto est{MakeEstimate<3, full>(MakeTestValue<3, full>(dim), MakeTestCov<3, full>(dim))};

    // Without constraint
    {
        StatisticalTSquaredTest t{est, 20, mu, 0};
        CheckEq(t.NDF1(), 3.0, "5: NDF1 = dim - 0 = 3");
        CheckEq(t.NDF2(), 19.0, "5: NDF2 = n - 1 = 19");
    }

    // With constraint
    {
        StatisticalTSquaredTest t{est, 20, mu, 1};
        CheckEq(t.NDF1(), 2.0, "5: NDF1 = dim - nConstraint = 2");
        CheckEq(t.NDF2(), 19.0, "5: NDF2 unchanged with nConstraint");
    }

    // Two-sample: NDF1 = Dimension
    {
        StatisticalTSquaredTest t{est, 100.0, est, 100.0};
        CheckEq(t.NDF1(), 3.0, "5: two-sample NDF1 = Dimension");
    }

    // Dimension check
    {
        StatisticalTSquaredTest t{est, 20, mu};
        CheckEq(t.Dimension(), 3, "5: Dimension matches");
    }
}};

// =========================================================================
// Section 6: Property checks
// =========================================================================

constexpr auto sec6Props{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    auto est{MakeEstimate<K, C>(x, cov)};

    // Two-sample with identical estimates
    StatisticalTSquaredTest t{est, 50.0, est, 50.0};
    CheckNonNegative(t.Value(), "6: T² >= 0");
    CheckBetween(t.PValue(), 0.0, 1.0, "6: PValue in [0,1]");
    CheckNonNegative(1.0 - t.PValue(), "6: 1-PValue >= 0");
}};

// =========================================================================
// Section 7: Statistic delegation
// =========================================================================

constexpr auto sec7Delegation{[] {
    constexpr auto full{CovarianceOption::Full};
    constexpr int dim{2};

    auto x1{MakeTestValue<2, full>(dim)};
    auto x2{MakeTestValue<2, full>(dim)};
    x2[0] = 3.0;

    auto stat1{TestStatistic::MakeStatistic<2, full>(dim)};
    for (auto i{0}; i < 30; ++i) {
        auto v1{x1};
        v1[0] += (i % 2) ? 1.0 : -1.0;
        stat1.Fill(v1);
    }
    auto stat2{TestStatistic::MakeStatistic<2, full>(dim)};
    for (auto i{0}; i < 30; ++i) {
        auto v2{x2};
        v2[1] += (i % 2) ? 1.0 : -1.0;
        stat2.Fill(v2);
    }

    StatisticalTSquaredTest ts{stat1, stat2};
    StatisticalTSquaredTest te{stat1.MeanEstimate(), stat1.EffectiveSampleSize(),
                               stat2.MeanEstimate(), stat2.EffectiveSampleSize()};

    CheckClose(ts.Value(), te.Value(), "7: T² from Stat vs Est match");
    CheckClose(ts.NDF1(), te.NDF1(), "7: NDF1 from Stat vs Est match");
    CheckClose(ts.NDF2(), te.NDF2(), "7: NDF2 from Stat vs Est match");
}};

} // namespace TestStatisticalTSquaredTestSection

auto TestStatisticalTSquaredTest::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestStatisticalTSquaredTestSection;

    PrintLn("--- Smoke: All constructor combinations ---");
    RunOverAllDims<MultiDims>(secSmoke);
    secSmokeCrossType();
    PrintLn("  smoke passed: all constructor combinations (K=1,2,3,5 + dynamic × Full/Diag + cross-type)");

    PrintLn("--- Section 1: Constructor validation ---");
    sec1Validation();
    PrintLn("  1 passed: dim mismatch, n≤1, nConstraint range throw");

    PrintLn("--- Section 2: Two-sample identical → T²≈0 ---");
    RunOverAllDims<MultiDims>(sec2Identical);
    PrintLn("  2 passed: identical estimates give T²≈0, p≈1 (K=1,2,3,5 + dynamic × Full/Diag)");

    PrintLn("--- Section 3: Two-sample different → T²>0 ---");
    sec3Different();
    PrintLn("  3 passed: different estimates give T²>0");

    PrintLn("--- Section 4: One-sample no effect ---");
    sec4OneSample();
    PrintLn("  4 passed: estimate matches mu gives T²≈0");

    PrintLn("--- Section 5: nConstraint and NDF ---");
    sec5NDF();
    PrintLn("  5 passed: NDF1 = dim - nConstraint, NDF2 = n-1");

    PrintLn("--- Section 6: Property checks ---");
    RunOverAllDims<MultiDims>(sec6Props);
    PrintLn("  6 passed: T²≥0, PValue∈[0,1] (K=1,2,3,5 + dynamic × Full/Diag)");

    PrintLn("--- Section 7: Statistic delegation ---");
    sec7Delegation();
    PrintLn("  7 passed: Statistic constructor matches Estimate constructor");

    PrintLn("All TestStatisticalTSquaredTest tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
