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

TestStatistic0::TestStatistic0() :
    Subprogram{"TestStatistic0", "Test Mustard::Statistic (Section 0: Construction)."} {}

namespace TestStatisticSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestStatistic;

// =========================================================================
// Section 0 Smoke: Construction (from sec0Smoke)
// =========================================================================

constexpr auto sec0Smoke{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    using POD = StatisticPOD<K, C>;
    constexpr auto dim{(K == Eigen::Dynamic) ? 3 : K};
    constexpr bool isDynamic{K == Eigen::Dynamic};

    // Default construction (static only)
    if constexpr (not isDynamic) {
        Stat s1;
        [[maybe_unused]] auto d{s1.Dimension()};
        POD pod{};
        Stat s4{pod};
        [[maybe_unused]] auto d4{s4.Dimension()};
    }

    // Dimension-based construction (dynamic only)
    if constexpr (isDynamic) {
        Stat s1(dim);
        [[maybe_unused]] auto d{s1.Dimension()};
    }

    // Copy and move construction
    Stat s1{MakeStatistic<K, C>(dim)};
    Stat s2{s1};
    Stat s3{std::move(s2)};
    [[maybe_unused]] auto d2{s2.Dimension()};
    [[maybe_unused]] auto d3{s3.Dimension()};
}};

// =========================================================================
// Section 0 Smoke CrossType: Cross-Type Construction
// =========================================================================

constexpr auto sec0SmokeCrossType{[] {
    [[maybe_unused]] double scalar{};

    // Scalar cross
    {
        Statistic<1> sFull;
        Statistic<1, CovarianceOption::Diagonal> sDiag;

        Statistic<1, CovarianceOption::Diagonal> sDiagFromFull{sFull};
        Statistic<1> sFullFromDiag{sDiag};
        scalar = sDiagFromFull.Dimension();
        scalar = sFullFromDiag.Dimension();
    }

    // Vector cross
    {
        Statistic<2> sFull;
        Statistic<2, CovarianceOption::Diagonal> sDiag;

        Statistic<2, CovarianceOption::Diagonal> sDiagFromFull{sFull};
        Statistic<2> sFullFromDiag{sDiag};
        scalar = sDiagFromFull.Dimension();
        scalar = sFullFromDiag.Dimension();
    }

    // Static from Dynamic
    {
        Statistic<Eigen::Dynamic> sDyn(3);
        Statistic<3> sStat{sDyn};
        scalar = sStat.Dimension();
    }

    // Dynamic from Static
    {
        Statistic<3> sStat;
        Statistic<Eigen::Dynamic> sDyn{sStat};
        scalar = sDyn.Dimension();
    }
}};

// =========================================================================
// Section 0: Construction (K>=1)
// =========================================================================

constexpr auto sec0Construction{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr bool isDynamic{K == Eigen::Dynamic};

    // Static/default construction
    if constexpr (not isDynamic) {
        Stat s;
        CheckEq(s.Dimension(), K, "0: Dimension");
        CheckEq(s.SampleSize(), 0LL, "0: SampleSize");
        CheckClose(s.WeightSum(), 0.0, "0: WeightSum");
        CheckClose(s.WeightSquareSum(), 0.0, "0: WeightSquareSum");
        if constexpr (K == 1) {
            CheckClose(s.Sum(), 0.0, "0: Sum");
        } else {
            CheckClose(s.Sum(0), 0.0, "0: Sum(0)");
        }
    }

    // Dynamic construction with valid dimension
    if constexpr (isDynamic) {
        constexpr auto dim{3};
        {
            Stat s(dim);
            CheckEq(s.Dimension(), dim, "0: Dimension");
            CheckEq(s.SampleSize(), 0LL, "0: SampleSize");
            CheckClose(s.WeightSum(), 0.0, "0: WeightSum");
            CheckClose(s.WeightSquareSum(), 0.0, "0: WeightSquareSum");
            CheckClose(s.Sum(0), 0.0, "0: Sum(0)");
        }

        // dim <= 0 throws
        auto threwZero{false};
        try {
            Stat bad(0);
        } catch (const std::invalid_argument&) { threwZero = true; }
        if (not threwZero) {
            Throw<std::runtime_error>("0: dim=0 should throw std::invalid_argument");
        }

        auto threwNeg{false};
        try {
            Stat bad(-1);
        } catch (const std::invalid_argument&) { threwNeg = true; }
        if (not threwNeg) {
            Throw<std::runtime_error>("0: dim=-1 should throw std::invalid_argument");
        }
    }
}};

} // namespace TestStatisticSection

auto TestStatistic0::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestStatisticSection;

    PrintLn("--- Section 0: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec0Smoke);
    PrintLn("  0 smoke passed: construction compilation smoke (K=1,2,3,5,10 Full/Diag + dynamic)");

    sec0SmokeCrossType();
    PrintLn("  0 smoke cross-type passed: cross-type construction compilation smoke");

    PrintLn("--- Section 0: Construction ---");
    RunOverAllDims<AllStaticDims>(sec0Construction);
    PrintLn("  0 passed: construction (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestStatistic0 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
