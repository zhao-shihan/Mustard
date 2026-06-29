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
#include "Mustard/Testing/TestStatistic/TestStatistic.h++"
#include "Mustard/Testing/TestStatistic/TestStatisticCommon.h++"

#include <cstdlib>

namespace Mustard::Testing {

TestStatistic8::TestStatistic8() :
    Subprogram{"TestStatistic8", "Test Mustard::Statistic (Section 8: Partition and Correlation Consistency)."} {}

namespace TestStatisticSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestStatistic;

// =========================================================================
// Section 8 Smoke: Partition and Correlation
// =========================================================================

constexpr auto sec8Smoke{[]<int K, CovarianceOption C>() {
    constexpr auto dim{(K == Eigen::Dynamic) ? 2 : K};
    constexpr bool isFull{C == CovarianceOption::Full};

    // Partition merge smoke
    {
        using Stat = Statistic<K, C>;
        Stat s{MakeStatistic<K, C>(dim)};
        if constexpr (K == 1) {
            s.Fill(1.0);
            s.Fill(2.0);
        } else {
            s.Fill(MakeSeqVector<K>(dim, 1.0));
            s.Fill(MakeSeqVector<K>(dim, 2.0));
        }
        Stat s2{MakeStatistic<K, C>(dim)};
        if constexpr (K == 1) {
            s2.Fill(3.0);
        } else {
            s2.Fill(MakeSeqVector<K>(dim, 3.0));
        }
        s += s2;
        [[maybe_unused]] auto m{s.Mean()};
    }

    // Correlation zero-variance smoke
    if constexpr (isFull and K != 1) {
        using Stat = Statistic<K, C>;
        Stat s{MakeStatistic<K, C>(dim)};
        auto v{MakeSeqVector<K>(dim, 1.0)};
        s.Fill(v);
        s.Fill(v);
        [[maybe_unused]] auto c{s.Correlation(0, 0)};
    }
}};

// =========================================================================
// Section 8: Partition Consistency (K>=1)
// =========================================================================

constexpr auto sec8PartitionConsistency{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr auto dim{isDynamic ? 2 : K};

    auto fillStat{[&](Stat& s, int startVal, int endVal) {
        if constexpr (K == 1) {
            for (auto v{startVal}; v <= endVal; ++v) {
                s.Fill(static_cast<double>(v));
            }
        } else {
            for (auto v{startVal}; v <= endVal; ++v) {
                s.Fill(v * MakeSeqVector<K>(dim, 1.0));
            }
        }
    }};

    Stat sFull{MakeStatistic<K, C>(dim)};
    fillStat(sFull, 1, 6);

    Stat sPart{MakeStatistic<K, C>(dim)};
    fillStat(sPart, 1, 3);
    Stat sOther{MakeStatistic<K, C>(dim)};
    fillStat(sOther, 4, 6);

    sPart += sOther;

    if constexpr (K == 1) {
        CheckClose(sPart.Mean(), sFull.Mean(), "8: partition consistency Mean()");
        CheckClose(sPart.Variance(), sFull.Variance(), "8: partition consistency Variance()");
    } else {
        CheckClose(sPart.Mean(0), sFull.Mean(0), "8: partition consistency Mean(0)");
        CheckClose(sPart.Variance(0), sFull.Variance(0), "8: partition consistency Variance(0)");
    }
    CheckEq(sPart.SampleSize(), sFull.SampleSize(), "8: partition consistency SampleSize");
}};

// =========================================================================
// Section 8: Correlation — zero-variance edge case
// =========================================================================

constexpr auto sec8CorrZeroVar{[]<int K, CovarianceOption C>() {
    constexpr auto dim{(K == Eigen::Dynamic) ? 2 : K};
    constexpr bool isFull{C == CovarianceOption::Full};

    if constexpr (isFull and K != 1) {
        using Stat = Statistic<K, C>;
        Stat s{MakeStatistic<K, C>(dim)};
        // Feed the same observation twice → zero variance for all components
        auto v{MakeSeqVector<K>(dim, 1.0)};
        s.Fill(v);
        s.Fill(v);

        CheckClose(s.Correlation(0, 0), 1.0, "8: Corr(0,0)=1 even Var=0");
        if constexpr (dim >= 2) {
            if (not std::isnan(s.Correlation(0, 1))) {
                Throw<std::runtime_error>("8: Corr(0,1) should be NaN when Var=0");
            }
            if (not std::isnan(s.CorrelationOfMean(0, 1))) {
                Throw<std::runtime_error>("8: CorrOfMean(0,1) should be NaN when Var=0");
            }
            CheckClose(s.Correlation(1, 1), 1.0, "8: Corr(1,1)=1");
            CheckClose(s.CorrelationOfMean(0, 0), 1.0, "8: CorrOfMean(0,0)=1");
            CheckClose(s.CorrelationOfMean(1, 1), 1.0, "8: CorrOfMean(1,1)=1");
        }
    }
}};

} // namespace TestStatisticSection

auto TestStatistic8::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestStatisticSection;

    PrintLn("--- Section 8: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec8Smoke);
    PrintLn("  8 smoke passed: partition and correlation compilation smoke (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 8: Partition Consistency ---");
    RunOverAllDims<AllStaticDims>(sec8PartitionConsistency);
    PrintLn("  8 passed: partition consistency (K=1,2,3,5,10 Full/Diag + dynamic)");

    RunOverAllDims<AllStaticDims>(sec8CorrZeroVar);
    PrintLn("  8 passed: correlation zero-variance edge case (K=2,3,5,10 Full + dynamic)");

    PrintLn("All TestStatistic8 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
