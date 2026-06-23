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

TestStatistic5::TestStatistic5() :
    Subprogram{"TestStatistic5", "Test Mustard::Statistic (Section 5: Negative Weights)."} {}

namespace TestStatisticSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestStatistic;

// =========================================================================
// Section 5 Smoke: Negative Weights
// =========================================================================

constexpr auto sec5Smoke{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr auto dim{isDynamic ? 2 : K};

    auto fillSeq{[&](Stat& s, double multiplier, double w = 1.0) {
        if constexpr (K == 1) {
            s.Fill(multiplier, w);
        } else {
            s.Fill(multiplier * MakeSeqVector<K>(dim, 1.0), w);
        }
    }};

    // Negative weight fill
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillSeq(s, 1.0, 5.0);
        fillSeq(s, 2.0, -2.0);
        [[maybe_unused]] auto n{s.SampleSize()};
    }

    // Mixed positive/negative weights
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillSeq(s, 1.0, 2.0);
        fillSeq(s, 3.0, -1.0);
        fillSeq(s, 5.0, 1.0);
        // Should not crash
        if constexpr (K == 1) {
            [[maybe_unused]] auto v{s.Variance()};
        } else {
            [[maybe_unused]] auto v{s.Variance(0)};
        }
    }

    // Weights summing to zero (cancellation)
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillSeq(s, 1.0, 1.0);
        fillSeq(s, 2.0, -1.0);
        [[maybe_unused]] auto ws{s.WeightSum()};
    }

    // Negative weight alone on single sample
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillSeq(s, 5.0, -1.0);
        [[maybe_unused]] auto n{s.SampleSize()};
    }
}};

// =========================================================================
// Section 5: Negative Weights (K>=1)
// =========================================================================

constexpr auto sec5NegativeWeightVectors{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr auto dim{isDynamic ? 2 : K};

    // Fill helpers
    auto fillSeq{[&](Stat& s, double multiplier, double w = 1.0) {
        if constexpr (K == 1) {
            s.Fill(multiplier, w);
        } else {
            s.Fill(multiplier * MakeSeqVector<K>(dim, 1.0), w);
        }
    }};

    // Negative weight fill
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillSeq(s, 1.0, 5.0);
        fillSeq(s, 2.0, -2.0);

        CheckEq(s.SampleSize(), 2LL, "5: SampleSize (neg weight)");
        CheckClose(s.WeightSum(), 3.0, "5: WeightSum (5 + (-2))");
        CheckClose(s.WeightSquareSum(), 29.0, "5: WeightSquareSum (25 + 4)");
        // m[i] = 5*(i+1) + (-2)*2*(i+1) = (i+1)
        if constexpr (K == 1) {
            CheckClose(s.Sum(), 1.0, "5: Sum()");
            CheckClose(s.Mean(), 1.0 / 3.0, "5: Mean()");
        } else {
            for (auto i{0}; i < dim; ++i) {
                const auto coeff{static_cast<double>(i + 1)};
                CheckClose(s.Sum(i), coeff, fmt::format("5: Sum({})", i));
                CheckClose(s.Mean(i), coeff / 3.0, fmt::format("5: Mean({})", i));
            }
        }
    }

    // Mixed positive/negative weights
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillSeq(s, 1.0, 2.0);  // w=2
        fillSeq(s, 3.0, -1.0); // w=-1
        fillSeq(s, 5.0, 1.0);  // w=1

        CheckEq(s.SampleSize(), 3LL, "5: SampleSize (mixed signs)");
        CheckClose(s.WeightSum(), 2.0, "5: WeightSum (mixed signs)");
        CheckClose(s.WeightSquareSum(), 6.0, "5: WeightSquareSum (4+1+1)");
        // m[i] = 2*(i+1)*1 + (-1)*(i+1)*3 + 1*(i+1)*5 = (i+1)*(2 -3 +5) = 4*(i+1)
        if constexpr (K == 1) {
            CheckClose(s.Sum(), 4.0, "5: Sum() mixed");
            CheckClose(s.Mean(), 2.0, "5: Mean() mixed");
            // Should not crash
            s.Variance();
            s.StdDev();
        } else {
            for (auto i{0}; i < dim; ++i) {
                const auto coeff{static_cast<double>(i + 1)};
                CheckClose(s.Sum(i), 4.0 * coeff, fmt::format("5: Sum({}) mixed", i));
                CheckClose(s.Mean(i), 2.0 * coeff, fmt::format("5: Mean({}) mixed", i));
            }
            // Should not crash
            s.Variance(0);
            s.StdDev(0);
        }
    }

    // Weights summing to zero (cancellation)
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillSeq(s, 1.0, 1.0);
        fillSeq(s, 2.0, -1.0);

        CheckEq(s.SampleSize(), 2LL, "5: SampleSize (cancellation)");
        CheckClose(s.WeightSum(), 0.0, "5: WeightSum (cancellation)");
        CheckClose(s.WeightSquareSum(), 2.0, "5: WeightSquareSum (cancellation)");
        if constexpr (K == 1) {
            CheckClose(s.Variance(), 0.0, "5: Variance() (weight cancellation)");
        } else {
            CheckClose(s.Variance(0), 0.0, "5: Variance(0) (weight cancellation)");
        }

        if constexpr (std::numeric_limits<double>::is_iec559) {
            if constexpr (K == 1) {
                if (not std::isinf(s.Mean())) {
                    Throw<std::runtime_error>("5: Mean() with w=0 should be inf under IEEE 754");
                }
            } else {
                if (not std::isinf(s.Mean(0))) {
                    Throw<std::runtime_error>("5: Mean(0) with w=0 should be inf under IEEE 754");
                }
            }
        }
    }

    // Negative weight alone on single sample
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillSeq(s, 5.0, -1.0);
        CheckEq(s.SampleSize(), 1LL, "5: SampleSize (single neg)");
        CheckClose(s.WeightSum(), -1.0, "5: WeightSum (single neg)");
        CheckClose(s.WeightSquareSum(), 1.0, "5: WeightSquareSum (single neg)");
        if constexpr (K == 1) {
            CheckClose(s.Sum(), -5.0, "5: Sum() single neg");
            CheckClose(s.Mean(), 5.0, "5: Mean() single neg");
        } else {
            for (auto i{0}; i < dim; ++i) {
                const auto coeff{static_cast<double>(i + 1)};
                CheckClose(s.Sum(i), -5.0 * coeff, fmt::format("5: Sum({}) single neg", i));
                CheckClose(s.Mean(i), 5.0 * coeff, fmt::format("5: Mean({}) single neg", i));
            }
        }
    }
}};

} // namespace TestStatisticSection

auto TestStatistic5::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestStatisticSection;

    PrintLn("--- Section 5: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec5Smoke);
    PrintLn("  5 smoke passed: negative weights compilation smoke (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 5: Negative Weights ---");
    RunOverAllDims<AllStaticDims>(sec5NegativeWeightVectors);
    PrintLn("  5 passed: negative weights (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestStatistic5 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
