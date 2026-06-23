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

TestStatistic4::TestStatistic4() :
    Subprogram{"TestStatistic4", "Test Mustard::Statistic (Section 4: Edge Cases)."} {}

namespace TestStatisticSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestStatistic;

// =========================================================================
// Section 4 Smoke: Edge Cases
// =========================================================================

constexpr auto sec4Smoke{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr auto dim{isDynamic ? 2 : K};

    auto makeStat{[&] {
        if constexpr (isDynamic) {
            return Stat(dim);
        } else {
            return Stat{};
        }
    }};

    auto fillConst{[&](Stat& s, double val, double w = 1.0) {
        if constexpr (K == 1) {
            s.Fill(val, w);
        } else {
            s.Fill(MakeConstVector<K>(dim, val), w);
        }
    }};
    auto fillSeq{[&](Stat& s, double start = 1.0, double w = 1.0) {
        if constexpr (K == 1) {
            s.Fill(start, w);
        } else {
            s.Fill(MakeSeqVector<K>(dim, start), w);
        }
    }};

    // Zero-weight fill
    {
        Stat s{makeStat()};
        fillConst(s, 1.0, 0.0);
        [[maybe_unused]] auto n{s.SampleSize()};
    }

    // Single sample
    {
        Stat s{makeStat()};
        fillSeq(s);
        [[maybe_unused]] auto n{s.SampleSize()};
    }

    // Identical values
    {
        Stat s{makeStat()};
        for (auto j{0}; j < 10; ++j) {
            fillSeq(s);
        }
        if constexpr (K == 1) {
            [[maybe_unused]] auto v{s.Variance()};
        } else {
            [[maybe_unused]] auto v{s.Variance(0)};
        }
    }

    // Large sample count
    {
        Stat s{makeStat()};
        for (auto j{0}; j < 1000; ++j) {
            fillConst(s, 1.0);
        }
        [[maybe_unused]] auto n{s.SampleSize()};
    }

    // Empty statistic
    {
        Stat s{makeStat()};
        if constexpr (K == 1) {
            [[maybe_unused]] auto m{s.Mean()};
        } else {
            [[maybe_unused]] auto m{s.Mean(0)};
        }
    }
}};

// =========================================================================
// Section 4: Edge Cases (K>=1)
// =========================================================================

constexpr auto sec4EdgeCases{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr auto dim{isDynamic ? 2 : K};

    // Construction helper
    auto makeStat{[&] {
        if constexpr (isDynamic) {
            return Stat(dim);
        } else {
            return Stat{};
        }
    }};

    // Fill helpers
    auto fillConst{[&](Stat& s, double val, double w = 1.0) {
        if constexpr (K == 1) {
            s.Fill(val, w);
        } else {
            s.Fill(MakeConstVector<K>(dim, val), w);
        }
    }};
    auto fillSeq{[&](Stat& s, double start = 1.0, double w = 1.0) {
        if constexpr (K == 1) {
            s.Fill(start, w);
        } else {
            s.Fill(MakeSeqVector<K>(dim, start), w);
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

    // Zero-weight fill is a no-op
    {
        Stat s{makeStat()};
        fillConst(s, 1.0, 0.0);
        CheckEq(s.SampleSize(), 0LL, "4: SampleSize (zero weight)");
        CheckClose(s.WeightSum(), 0.0, "4: WeightSum (zero weight)");

        // After a real fill, zero-weight should not change anything
        fillSeq(s);
        const auto n{s.SampleSize()};
        const auto w{s.WeightSum()};

        fillConst(s, 1.0, 0.0);
        CheckEq(s.SampleSize(), n, "4: SampleSize unchanged by zero weight");
        CheckClose(s.WeightSum(), w, "4: WeightSum unchanged by zero weight");
    }

    // Single sample (variance is NaN)
    {
        Stat s{makeStat()};
        fillSeq(s);
        CheckEq(s.SampleSize(), 1LL, "4: SampleSize");

        // CovCoeff = 1/(1-1) = inf, m2=0 => Variance = inf*0 = NaN
        if constexpr (K == 1) {
            if (not std::isnan(s.Variance())) {
                Throw<std::runtime_error>(fmt::format("4: Variance() should be NaN for single sample, got {}", s.Variance()));
            }
        } else {
            if (not std::isnan(s.Variance(0))) {
                Throw<std::runtime_error>(fmt::format("4: Variance(0) should be NaN for single sample, got {}", s.Variance(0)));
            }
        }
    }

    // Identical values (zero variance)
    {
        Stat s{makeStat()};
        for (auto j{0}; j < 10; ++j) {
            fillSeq(s);
        }
        checkVariance(s, 0.0, "4: Variance (identical values)");
    }

    // Large sample count
    {
        Stat s{makeStat()};
        constexpr long long kN{10'000};
        for (auto j{0}; j < kN; ++j) {
            fillConst(s, 1.0);
        }
        CheckEq(s.SampleSize(), kN, "4: SampleSize");
        checkMean(s, 1.0, "4: Mean");
        checkVariance(s, 0.0, "4: Variance");
    }

    // Empty statistic accessors
    if constexpr (std::numeric_limits<double>::is_iec559) {
        Stat s{makeStat()};
        if constexpr (K == 1) {
            if (not std::isnan(s.Mean())) {
                Throw<std::runtime_error>("4: Mean() of empty statistic should be NaN under IEEE 754");
            }
        } else {
            if (not std::isnan(s.Mean(0))) {
                Throw<std::runtime_error>("4: Mean(0) of empty statistic should be NaN under IEEE 754");
            }
        }
    }
}};

} // namespace TestStatisticSection

auto TestStatistic4::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestStatisticSection;

    PrintLn("--- Section 4: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec4Smoke);
    PrintLn("  4 smoke passed: edge cases compilation smoke (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 4: Edge Cases ---");
    RunOverAllDims<AllStaticDims>(sec4EdgeCases);
    PrintLn("  4 passed: edge cases (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestStatistic4 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
