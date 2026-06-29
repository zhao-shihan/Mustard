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

TestStatisticalChiSquaredTest::TestStatisticalChiSquaredTest() :
    Subprogram{"TestStatisticalChiSquaredTest", "Test Mustard::Math::StatisticalChiSquaredTest."} {}

namespace TestStatisticalChiSquaredTestSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;
using namespace Mustard::Testing::TestStatisticalTest;

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
        StatisticalChiSquaredTest t{est, est};
        scalar = t.Value();
    }

    // ---- Two-sample: same K, different C ----
    {
        StatisticalChiSquaredTest t{est, estOpp};
        scalar = t.Value();
    }

    // ---- One-sample from Estimate ----
    {
        StatisticalChiSquaredTest t{est, muVec};
        scalar = t.Value();
    }
    {
        StatisticalChiSquaredTest t{est, muVec, 0};
        scalar = t.Value();
    }
    if constexpr (dim >= 2) {
        StatisticalChiSquaredTest t{est, muVec, 1};
        scalar = t.Value();
    }

    // ---- One-sample cross-C ----
    {
        StatisticalChiSquaredTest t{estOpp, muVec};
        scalar = t.Value();
    }

    // ---- Access all methods ----
    {
        StatisticalChiSquaredTest t{est, muVec};
        scalar = t.Value();
        scalar = t.Dimension();
        scalar = t.NDF();
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
        StatisticalChiSquaredTest t{eF2, eD2};
        scalar = t.Value();
    }

    // B: Static × Dynamic (K=2)
    {
        auto eS2{MakeEstimate<2, full>(MakeTestValue<2, full>(2), MakeTestCov<2, full>(2))};
        auto eDy2{MakeEstimate<dyn, full>(MakeTestValue<dyn, full>(2), MakeTestCov<dyn, full>(2))};
        StatisticalChiSquaredTest t{eS2, eDy2};
        scalar = t.Value();
    }

    // C: Dynamic × Dynamic different C (dim=2)
    {
        auto eDyF2{MakeEstimate<dyn, full>(MakeTestValue<dyn, full>(2), MakeTestCov<dyn, full>(2))};
        auto eDyD2{MakeEstimate<dyn, diag>(MakeTestValue<dyn, diag>(2), MakeTestCov<dyn, diag>(2))};
        StatisticalChiSquaredTest t{eDyF2, eDyD2};
        scalar = t.Value();
    }

    // D: One-sample Static Full with Dynamic mu
    {
        auto eS2{MakeEstimate<2, full>(MakeTestValue<2, full>(2), MakeTestCov<2, full>(2))};
        Eigen::Vector2d mu;
        mu << 1.0, 2.0;
        StatisticalChiSquaredTest t{eS2, mu};
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

    // Dimension mismatch throws
    CheckThrows("1: dim mismatch 2 vs 3", [&] { StatisticalChiSquaredTest{ed2, ed3}; });

    // nConstraint out of range
    Eigen::Vector2d mu2;
    mu2 << 1.0, 2.0;
    auto e2{MakeEstimate<2, full>(MakeTestValue<2, full>(2), MakeTestCov<2, full>(2))};
    CheckThrows("1: nConstraint = -1", [&] { StatisticalChiSquaredTest{e2, mu2, -1}; });
    CheckThrows("1: nConstraint = dim", [&] { StatisticalChiSquaredTest{e2, mu2, 2}; });
}};

// =========================================================================
// Section 2: Two-sample identical → χ²≈0
// =========================================================================

constexpr auto sec2Identical{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    auto est{MakeEstimate<K, C>(x, cov)};

    StatisticalChiSquaredTest t{est, est};
    CheckClose(t.Value(), 0.0, "2: χ² ≈ 0 for identical estimates");
    CheckBetween(t.PValue(), 0.99, 1.01, "2: PValue ≈ 1 for identical");
    CheckEq(t.Significance() < 0, true, "2: Significance < 0 for PValue≈1");
}};

// =========================================================================
// Section 3: Two-sample different → χ²>0
// =========================================================================

constexpr auto sec3Different{[] {
    constexpr auto full{CovarianceOption::Full};

    Eigen::Vector2d x1, x2;
    x1 << 10.0, 20.0;
    x2 << 5.0, 15.0;

    Eigen::Matrix2d cov;
    cov << 4.0, 1.0, 1.0, 3.0;

    Estimate<2, full> est1{x1, cov};
    Estimate<2, full> est2{x2, cov};

    StatisticalChiSquaredTest t{est1, est2};
    CheckNonNegative(t.Value(), "3: χ² non-negative");
    CheckEq(t.Value() > 0, true, "3: χ² > 0 for different means");
    CheckBetween(t.PValue(), 0.0, 1.0, "3: PValue in [0,1]");
}};

// =========================================================================
// Section 4: One-sample no effect + nConstraint
// =========================================================================

constexpr auto sec4OneSample{[] {
    constexpr auto full{CovarianceOption::Full};
    constexpr int dim{3};

    Eigen::Vector3d mu;
    mu << 1.0, 2.0, 3.0;
    auto est{MakeEstimate<3, full>(mu, MakeTestCov<3, full>(dim))};

    // No constraint
    {
        StatisticalChiSquaredTest t{est, mu, 0};
        CheckClose(t.Value(), 0.0, "4: χ² ≈ 0 for no effect");
        CheckEq(t.NDF(), 3.0, "4: NDF = dim without constraint");
    }

    // With constraint
    {
        StatisticalChiSquaredTest t{est, mu, 1};
        CheckEq(t.NDF(), 2.0, "4: NDF = dim - nConstraint");
    }

    // Two-sample NDF = Dimension
    {
        StatisticalChiSquaredTest t{est, est};
        CheckEq(t.NDF(), 3.0, "4: two-sample NDF = Dimension");
    }
}};

// =========================================================================
// Section 5: Property checks
// =========================================================================

constexpr auto sec5Props{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    auto est{MakeEstimate<K, C>(x, cov)};

    StatisticalChiSquaredTest t{est, est};
    CheckNonNegative(t.Value(), "5: χ² >= 0");
    CheckBetween(t.PValue(), 0.0, 1.0, "5: PValue in [0,1]");
    CheckNonNegative(1.0 - t.PValue(), "5: 1-PValue >= 0");
}};

// =========================================================================
// Section 6: Statistic delegation
// =========================================================================

constexpr auto sec6Delegation{[] {
    constexpr auto full{CovarianceOption::Full};
    constexpr int dim{2};

    auto x1{MakeTestValue<2, full>(dim)};
    auto x2{MakeTestValue<2, full>(dim)};
    x2[0] = 3.0;

    auto stat1{TestStatistic::MakeStatistic<2, full>(dim)};
    for (auto i{0}; i < 30; ++i) {
        stat1.Fill(x1);
    }
    auto stat2{TestStatistic::MakeStatistic<2, full>(dim)};
    for (auto i{0}; i < 30; ++i) {
        stat2.Fill(x2);
    }

    StatisticalChiSquaredTest ts{stat1, stat2};
    StatisticalChiSquaredTest te{stat1.MeanEstimate(), stat2.MeanEstimate()};

    CheckClose(ts.Value(), te.Value(), "6: χ² from Stat vs Est match");
    CheckClose(ts.NDF(), te.NDF(), "6: NDF from Stat vs Est match");

    // One-sample
    Eigen::Vector2d mu;
    mu << 0.0, 0.0;
    StatisticalChiSquaredTest ost{stat1, mu};
    StatisticalChiSquaredTest oet{stat1.MeanEstimate(), mu, 0};
    CheckClose(ost.Value(), oet.Value(), "6: one-sample χ² from Stat vs Est match");
}};

} // namespace TestStatisticalChiSquaredTestSection

auto TestStatisticalChiSquaredTest::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestStatisticalChiSquaredTestSection;

    PrintLn("--- Smoke: All constructor combinations ---");
    RunOverAllDims<MultiDims>(secSmoke);
    secSmokeCrossType();
    PrintLn("  smoke passed: all constructor combinations (K=1,2,3,5 + dynamic × Full/Diag + cross-type)");

    PrintLn("--- Section 1: Constructor validation ---");
    sec1Validation();
    PrintLn("  1 passed: dim mismatch, nConstraint range throw");

    PrintLn("--- Section 2: Two-sample identical → χ²≈0 ---");
    RunOverAllDims<MultiDims>(sec2Identical);
    PrintLn("  2 passed: identical estimates give χ²≈0, p≈1 (K=1,2,3,5 + dynamic × Full/Diag)");

    PrintLn("--- Section 3: Two-sample different → χ²>0 ---");
    sec3Different();
    PrintLn("  3 passed: different estimates give χ²>0");

    PrintLn("--- Section 4: One-sample no effect + nConstraint ---");
    sec4OneSample();
    PrintLn("  4 passed: estimate matches mu gives χ²≈0, NDF=dim-nConstraint");

    PrintLn("--- Section 5: Property checks ---");
    RunOverAllDims<MultiDims>(sec5Props);
    PrintLn("  5 passed: χ²≥0, PValue∈[0,1] (K=1,2,3,5 + dynamic × Full/Diag)");

    PrintLn("--- Section 6: Statistic delegation ---");
    sec6Delegation();
    PrintLn("  6 passed: Statistic constructor matches Estimate constructor");

    PrintLn("All TestStatisticalChiSquaredTest tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
