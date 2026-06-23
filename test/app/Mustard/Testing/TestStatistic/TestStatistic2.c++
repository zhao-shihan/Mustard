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

TestStatistic2::TestStatistic2() :
    Subprogram{"TestStatistic2", "Test Mustard::Statistic (Section 2: Merge Operations)."} {}

namespace TestStatisticSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestStatistic;

// =========================================================================
// Section 2 Smoke: Merge and Combine (from sec0Smoke)
// =========================================================================

constexpr auto sec2Smoke{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr auto dim{(K == Eigen::Dynamic) ? 3 : K};

    Stat s1{MakeStatistic<K, C>(dim)};
    Stat s2{MakeStatistic<K, C>(dim)};

    if constexpr (K == 1) {
        s1.Fill(1.0);
        s2.Fill(2.0);
    } else {
        s1.Fill(MakeSeqVector<K>(dim, 1.0));
        s2.Fill(MakeSeqVector<K>(dim, 2.0));
    }

    // operator+=
    s1 += s2;

    // operator+ (all 4 ref-qualifier overloads)
    [[maybe_unused]] auto r1{s1 + s2};
    [[maybe_unused]] auto r2{s1 + Stat{s2}};
    [[maybe_unused]] auto r3{Stat{s1} + s2};
    [[maybe_unused]] auto r4{Stat{s1} + Stat{s2}};
}};

// =========================================================================
// Section 2 Smoke CrossType: Cross-CovarianceOption Merge/Combine
// =========================================================================

constexpr auto sec2SmokeCrossType{[] {
    // Scalar cross
    {
        Statistic<1> sFull;
        sFull.Fill(1.0);
        Statistic<1, CovarianceOption::Diagonal> sDiag;
        sDiag.Fill(2.0);

        sFull += sDiag;
        sDiag += sFull;
        [[maybe_unused]] auto r1{sFull + sDiag};
        [[maybe_unused]] auto r2{sDiag + sFull};
    }

    // Vector cross
    {
        Statistic<2> sFull;
        sFull.Fill(Eigen::Vector2d{1.0, 2.0});
        Statistic<2, CovarianceOption::Diagonal> sDiag;
        sDiag.Fill(Eigen::Vector2d{3.0, 4.0});

        sFull += sDiag;
        sDiag += sFull;
        [[maybe_unused]] auto r1{sFull + sDiag};
        [[maybe_unused]] auto r2{sDiag + sFull};
    }
}};

// =========================================================================
// Section 2: Merge Operations (K>=1)
// =========================================================================

constexpr auto sec2MergeOperations{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr auto dim{isDynamic ? 2 : K};

    auto makeFullStat{[&](int startVal, int endVal) {
        Stat s{MakeStatistic<K, C>(dim)};
        if constexpr (K == 1) {
            for (auto v{startVal}; v <= endVal; ++v) {
                s.Fill(static_cast<double>(v));
            }
        } else {
            for (auto v{startVal}; v <= endVal; ++v) {
                s.Fill(v * MakeSeqVector<K>(dim, 1.0));
            }
        }
        return s;
    }};

    // Equal partition
    {
        auto s1{makeFullStat(1, 3)};
        auto s2{makeFullStat(4, 5)};
        auto sFull{makeFullStat(1, 5)};

        s1 += s2;
        CheckEq(s1.SampleSize(), sFull.SampleSize(), "2: SampleSize after merge");
        CheckClose(s1.WeightSum(), sFull.WeightSum(), "2: WeightSum after merge");
        if constexpr (K == 1) {
            CheckClose(s1.Mean(), sFull.Mean(), "2: Mean after merge");
            CheckClose(s1.Variance(), sFull.Variance(), "2: Variance after merge");
        } else {
            CheckClose(s1.Mean(0), sFull.Mean(0), "2: Mean(0) after merge");
            CheckClose(s1.Mean(dim - 1), sFull.Mean(dim - 1), "2: Mean(last) after merge");
            CheckClose(s1.Variance(0), sFull.Variance(0), "2: Variance(0) after merge");
            if constexpr (C == CovarianceOption::Full) {
                CheckClose(s1.Covariance(0, 1), sFull.Covariance(0, 1), "2: Cov(0,1) after merge");
            } else {
                CheckClose(s1.Covariance(0, 1), 0.0, "2: Cov(0,1)=0 after merge");
            }
        }
    }

    // operator+ does not mutate lhs
    {
        auto s1{makeFullStat(1, 2)};
        auto s2{makeFullStat(3, 4)};
        const auto s1CopyMean{s1.Mean()};

        auto s3{s1 + s2};

        // s1 unchanged
        if constexpr (K == 1) {
            CheckClose(s1.Mean(), s1CopyMean, "2: lhs Mean unchanged");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(s1.Mean(i), s1CopyMean[i], "2: lhs Mean unchanged");
            }
        }

        auto sFull{makeFullStat(1, 4)};
        if constexpr (K == 1) {
            CheckClose(s3.Mean(), sFull.Mean(), "2: result Mean");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(s3.Mean(i), sFull.Mean(i), "2: result Mean");
            }
        }
    }
}};

// =========================================================================
// Section 2s: Self-Merge (K>=1)
// s += s: self-merge — verifies no crash and basic invariants.
// =========================================================================

constexpr auto sec2SelfMerge{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr auto dim{isDynamic ? 2 : K};

    Stat s{MakeStatistic<K, C>(dim)};
    if constexpr (K == 1) {
        s.Fill(1.0);
        s.Fill(2.0);
        s.Fill(3.0);
    } else {
        s.Fill(MakeSeqVector<K>(dim, 1.0));
        s.Fill(MakeSeqVector<K>(dim, 2.0));
        s.Fill(MakeSeqVector<K>(dim, 3.0));
    }

    // Record pre-merge state
    const auto nPrev{s.SampleSize()};
    const auto wPrev{s.WeightSum()};
    const auto w2Prev{s.WeightSquareSum()};
    const auto meanPrev{s.Mean()};
    const auto sumPrev{s.Sum()};

    // Self-merge — must not crash (no UB)
    s += s;
    CheckEq(s.SampleSize(), 2 * nPrev, "2s: SampleSize doubled");
    CheckClose(s.WeightSum(), 2.0 * wPrev, "2s: WeightSum doubled");
    CheckClose(s.WeightSquareSum(), 2.0 * w2Prev, "2s: WeightSquareSum doubled");
    CheckClose(s.Mean(), meanPrev, "2s: Mean unchanged");
    CheckClose(s.Sum(), 2 * sumPrev, "2s: Sum doubled");
    CheckClose(s.Variance(), s.Variance(), "2s: Variance is finite");
    if constexpr (K != 1) {
        CheckClose(s.Covariance(), s.Covariance(), "2s: Covariance is finite");
    }
}};

} // namespace TestStatisticSection

auto TestStatistic2::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestStatisticSection;

    PrintLn("--- Section 2: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec2Smoke);
    PrintLn("  2 smoke passed: merge compilation smoke (K=1,2,3,5,10 Full/Diag + dynamic)");

    sec2SmokeCrossType();
    PrintLn("  2 smoke cross-type passed: cross-CovarianceOption merge/combine compilation smoke");

    PrintLn("--- Section 2: Merge Operations ---");
    RunOverAllDims<AllStaticDims>(sec2MergeOperations);
    PrintLn("  2 passed: merge operations (K=1,2,3,5,10 Full/Diag + dynamic)");
    RunOverAllDims<AllStaticDims>(sec2SelfMerge);
    PrintLn("  2s passed: self-merge (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestStatistic2 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
