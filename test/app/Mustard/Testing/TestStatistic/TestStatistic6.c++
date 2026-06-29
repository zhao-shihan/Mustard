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

TestStatistic6::TestStatistic6() :
    Subprogram{"TestStatistic6", "Test Mustard::Statistic (Section 6: Numerical Stress)."} {}

namespace TestStatisticSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestStatistic;

// =========================================================================
// Section 6 Smoke: Numerical Stress
// =========================================================================

constexpr auto sec6Smoke{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr auto dim{isDynamic ? 2 : K};

    auto fillConst{[&](Stat& s, double val, double w = 1.0) {
        if constexpr (K == 1) {
            s.Fill(val, w);
        } else {
            s.Fill(MakeConstVector<K>(dim, val), w);
        }
    }};

    // Very large values
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillConst(s, 1.0e15);
        fillConst(s, 1.0e15 + 1.0);
        fillConst(s, 1.0e15 + 2.0);
        if constexpr (K == 1) {
            [[maybe_unused]] auto m{s.Mean()};
        } else {
            [[maybe_unused]] auto m{s.Mean(0)};
        }
    }

    // Very small values
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillConst(s, 1.0e-15);
        fillConst(s, 2.0e-15);
        fillConst(s, 3.0e-15);
        if constexpr (K == 1) {
            [[maybe_unused]] auto m{s.Mean()};
        } else {
            [[maybe_unused]] auto m{s.Mean(0)};
        }
    }

    // Alternating large positive/negative
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillConst(s, 1.0e10);
        fillConst(s, -1.0e10);
        fillConst(s, 1.0e10);
        fillConst(s, -1.0e10);
        if constexpr (K == 1) {
            [[maybe_unused]] auto m{s.Mean()};
        } else {
            [[maybe_unused]] auto m{s.Mean(0)};
        }
    }

    // Large mean offset
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillConst(s, 1.0e9 + 1.0);
        fillConst(s, 1.0e9 + 2.0);
        fillConst(s, 1.0e9 + 3.0);
        if constexpr (K == 1) {
            [[maybe_unused]] auto m{s.Mean()};
        } else {
            [[maybe_unused]] auto m{s.Mean(0)};
        }
    }

    // Very small weights
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillConst(s, 5.0, 1.0e-300);
        [[maybe_unused]] auto n{s.SampleSize()};
    }

    // Large negative weight
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillConst(s, 1.0, -1.0e15);
        [[maybe_unused]] auto ws{s.WeightSum()};
    }
}};

// =========================================================================
// Section 6: Numerical Stress (K>=1)
// =========================================================================

constexpr auto sec6NumericalStressVectors{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr auto dim{isDynamic ? 2 : K};

    // Fill helper
    auto fillConst{[&](Stat& s, double val, double w = 1.0) {
        if constexpr (K == 1) {
            s.Fill(val, w);
        } else {
            s.Fill(MakeConstVector<K>(dim, val), w);
        }
    }};
    auto checkMean{[&](const Stat& s, double expected, const std::string& ctx) {
        if constexpr (K == 1) {
            CheckClose(s.Mean(), expected, ctx);
        } else {
            CheckClose(s.Mean(0), expected, ctx);
        }
    }};
    auto checkVariance{[&](const Stat& s, double expected, const std::string& ctx) {
        if constexpr (K == 1) {
            CheckClose(s.Variance(), expected, ctx);
        } else {
            CheckClose(s.Variance(0), expected, ctx);
        }
    }};

    // Very large values (~1e15)
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillConst(s, 1.0e15);
        fillConst(s, 1.0e15 + 1.0);
        fillConst(s, 1.0e15 + 2.0);

        checkMean(s, 1.0e15 + 1.0, "6: Mean (large)");
        checkVariance(s, 1.0, "6: Variance (large)");
    }

    // Very small values (~1e-15)
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillConst(s, 1.0e-15);
        fillConst(s, 2.0e-15);
        fillConst(s, 3.0e-15);

        checkMean(s, 2.0e-15, "6: Mean (small)");
        checkVariance(s, 1.0e-30, "6: Variance (small)");
    }

    // Alternating large positive/negative values
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillConst(s, 1.0e10);
        fillConst(s, -1.0e10);
        fillConst(s, 1.0e10);
        fillConst(s, -1.0e10);

        checkMean(s, 0.0, "6: Mean (alternating)");
        checkVariance(s, 4.0e20 / 3.0, "6: Variance (alternating)");
    }

    // Values with large mean offset
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillConst(s, 1.0e9 + 1.0);
        fillConst(s, 1.0e9 + 2.0);
        fillConst(s, 1.0e9 + 3.0);

        checkMean(s, 1.0e9 + 2.0, "6: Mean (offset)");
        checkVariance(s, 1.0, "6: Variance (offset)");
    }

    // Very small weights
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillConst(s, 5.0, 1.0e-300);

        CheckEq(s.SampleSize(), 1LL, "6: SampleSize (tiny weight)");
        CheckClose(s.WeightSum(), 1.0e-300, "6: WeightSum (tiny weight)");
        checkMean(s, 5.0, "6: Mean (tiny weight)");
    }

    // Single large negative weight
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillConst(s, 1.0, -1.0e15);
        CheckEq(s.SampleSize(), 1LL, "6: SampleSize (large neg)");
        CheckClose(s.WeightSum(), -1.0e15, "6: WeightSum (large neg)");
        CheckClose(s.WeightSquareSum(), 1.0e30, "6: WeightSquareSum (large neg)");
        checkMean(s, 1.0, "6: Mean (large neg)");
    }
}};

} // namespace TestStatisticSection

auto TestStatistic6::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestStatisticSection;

    PrintLn("--- Section 6: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec6Smoke);
    PrintLn("  6 smoke passed: numerical stress compilation smoke (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 6: Numerical Stress ---");
    RunOverAllDims<AllStaticDims>(sec6NumericalStressVectors);
    PrintLn("  6 passed: numerical stress (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestStatistic6 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
