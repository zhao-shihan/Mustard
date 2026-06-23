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

TestStatistic7::TestStatistic7() :
    Subprogram{"TestStatistic7", "Test Mustard::Statistic (Section 7: Dimension and Move Semantics)."} {}

namespace TestStatisticSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestStatistic;

// =========================================================================
// Section 7 Smoke: Dimension Mismatch, Self-Assignment, Post-Move
// =========================================================================

constexpr auto sec7Smoke{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr auto dim{isDynamic ? 2 : K};

    // Self copy-assign and move-assign
    {
        Stat s{MakeStatistic<K, C>(dim)};
        if constexpr (K == 1) {
            s.Fill(1.0);
        } else {
            s.Fill(MakeSeqVector<K>(dim, 1.0));
        }
        auto& sRef{s};
        s = sRef;
        s = std::move(sRef);
        [[maybe_unused]] auto n{s.SampleSize()};
    }

    // Post-move construction and assignment
    {
        Stat s1{MakeStatistic<K, C>(dim)};
        if constexpr (K == 1) {
            s1.Fill(5.0);
        } else {
            s1.Fill(MakeSeqVector<K>(dim, 5.0));
        }
        Stat s2{std::move(s1)};
        [[maybe_unused]] auto n{s2.SampleSize()};
        Stat s3{MakeStatistic<K, C>(dim)};
        s3 = std::move(s2);
        [[maybe_unused]] auto n3{s3.SampleSize()};
    }
}};

// =========================================================================
// Section 7 Smoke CrossType: Dim Mismatch and Dynamic Assign
// =========================================================================

constexpr auto sec7SmokeCrossType{[] {
    // Dim mismatch operator+= throws
    {
        Statistic<Eigen::Dynamic, CovarianceOption::Full> s1(3);
        Statistic<Eigen::Dynamic, CovarianceOption::Full> s2(5);
        auto threw{false};
        try {
            s1 += s2;
        } catch (const std::invalid_argument&) { threw = true; }
        if (not threw) {
            Throw<std::runtime_error>("7e: mismatch operator+= should throw");
        }
    }

    // Dim mismatch operator+ throws
    {
        Statistic<Eigen::Dynamic> s1(3);
        Statistic<Eigen::Dynamic> s2(5);
        auto threw{false};
        try {
            (void)(s1 + s2);
        } catch (const std::invalid_argument&) { threw = true; }
        if (not threw) {
            Throw<std::runtime_error>("7e: mismatch operator+ should throw");
        }
    }

    // Dynamic assign resizes
    {
        Statistic<Eigen::Dynamic> s1(3);
        Statistic<Eigen::Dynamic> s2(5);
        s1.Fill(Eigen::VectorXd::Ones(3));
        s2.Fill(Eigen::VectorXd::Ones(5));
        s1 = s2;
        [[maybe_unused]] auto d{s1.Dimension()};
    }

    // Static from dynamic with mismatched dimension throws
    {
        Statistic<Eigen::Dynamic> sDyn(5);
        sDyn.Fill(Eigen::VectorXd::Ones(5));
        auto threw{false};
        try {
            Statistic<3> sStat(sDyn);
        } catch (const std::invalid_argument&) { threw = true; }
        if (not threw) {
            Throw<std::runtime_error>("7e: static-from-dynamic mismatch should throw");
        }
    }

    // Cross-dimension-type smoke (from sec0SmokeCrossDim)
    {
        Statistic<Eigen::Dynamic> sDyn(3);
        sDyn.Fill(Eigen::Vector3d{1.0, 2.0, 3.0});
        Statistic<3> sStat{sDyn};
        sStat = sDyn;
        sStat += sDyn;
        [[maybe_unused]] auto r{sStat + sDyn};
    }

    {
        Statistic<3> sStat;
        sStat.Fill(Eigen::Vector3d{1.0, 2.0, 3.0});
        Statistic<Eigen::Dynamic> sDyn{sStat};
        sDyn = sStat;
        sDyn += sStat;
        [[maybe_unused]] auto r{sDyn + sStat};
    }
}};

// =========================================================================
// Section 7: Dimension Mismatch (dynamic dimension only)
// =========================================================================

constexpr auto sec7DimMismatch{[] {
    // operator+= with mismatched dynamic dimensions
    {
        Statistic<Eigen::Dynamic, CovarianceOption::Full> s1(3);
        Statistic<Eigen::Dynamic, CovarianceOption::Full> s2(5);
        auto threw{false};
        try {
            s1 += s2;
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        if (not threw) {
            Throw<std::runtime_error>("7: mismatch operator+= should throw");
        }
    }
    // operator+ with mismatched dynamic dimensions
    {
        Statistic<Eigen::Dynamic> s1(3);
        Statistic<Eigen::Dynamic> s2(5);
        auto threw{false};
        try {
            (void)(s1 + s2);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        if (not threw) {
            Throw<std::runtime_error>("7: mismatch operator+ should throw");
        }
    }
    // Dynamic-dynamic assignment with different dimensions should succeed
    {
        Statistic<Eigen::Dynamic> s1(3);
        Statistic<Eigen::Dynamic> s2(5);
        s1.Fill(Eigen::VectorXd::Ones(3));
        s2.Fill(Eigen::VectorXd::Ones(5));
        s1 = s2;
        CheckEq(s1.Dimension(), 5, "7: dynamic assign resizes dimension");
        CheckEq(s1.SampleSize(), s2.SampleSize(), "7: dynamic assign copies data");
    }
    // Static-from-dynamic with mismatched dimension should throw
    {
        Statistic<Eigen::Dynamic> sDyn(5);
        sDyn.Fill(Eigen::VectorXd::Ones(5));
        auto threw{false};
        try {
            Statistic<3> sStat(sDyn);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        if (not threw) {
            Throw<std::runtime_error>("7: static-from-dynamic mismatch should throw");
        }
    }
}};

// =========================================================================
// Section 7: Self-Assignment (K>=1)
// =========================================================================

constexpr auto sec7SelfAssignment{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr auto dim{isDynamic ? 2 : K};

    Stat s{MakeStatistic<K, C>(dim)};
    if constexpr (K == 1) {
        s.Fill(1.0);
        s.Fill(2.0);
    } else {
        s.Fill(MakeSeqVector<K>(dim, 1.0));
        s.Fill(MakeSeqVector<K>(dim, 2.0));
    }

    const auto n{s.SampleSize()};
    const auto w{s.WeightSum()};

    // Self copy-assign
    auto& sRef{s};
    s = sRef;
    CheckEq(s.SampleSize(), n, "7: SampleSize after self copy-assign");
    CheckClose(s.WeightSum(), w, "7: WeightSum after self copy-assign");

    // Self move-assign
    s = std::move(sRef);
    CheckEq(s.SampleSize(), n, "7: SampleSize after self move-assign");
    CheckClose(s.WeightSum(), w, "7: WeightSum after self move-assign");
}};

// =========================================================================
// Section 7: Post-Move State (K>=1)
// =========================================================================

constexpr auto sec7PostMove{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr auto dim{isDynamic ? 2 : K};

    Stat s1{MakeStatistic<K, C>(dim)};
    if constexpr (K == 1) {
        s1.Fill(5.0);
    } else {
        s1.Fill(MakeSeqVector<K>(dim, 5.0));
    }

    const auto n{s1.SampleSize()};
    const auto w{s1.WeightSum()};

    // Move construct
    Stat s2{std::move(s1)};
    CheckEq(s2.SampleSize(), n, "7: SampleSize after move");
    CheckClose(s2.WeightSum(), w, "7: WeightSum after move");

    // Move assignment
    Stat s3{MakeStatistic<K, C>(dim)};
    s3 = std::move(s2);
    CheckEq(s3.SampleSize(), n, "7: SampleSize after move assign");
    CheckClose(s3.WeightSum(), w, "7: WeightSum after move assign");
}};

} // namespace TestStatisticSection

auto TestStatistic7::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestStatisticSection;

    PrintLn("--- Section 7: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec7Smoke);
    PrintLn("  7 smoke passed: self-assignment and post-move compilation smoke (K=1,2,3,5,10 Full/Diag + dynamic)");

    sec7SmokeCrossType();
    PrintLn("  7 smoke cross-type passed: dimension mismatch and cross-dim compilation smoke");

    PrintLn("--- Section 7: Dimension and Move Semantics ---");
    sec7DimMismatch();
    PrintLn("  7 passed: dimension mismatch");
    RunOverAllDims<AllStaticDims>(sec7SelfAssignment);
    PrintLn("  7 passed: self-assignment (K=1,2,3,5,10 Full/Diag + dynamic)");
    RunOverAllDims<AllStaticDims>(sec7PostMove);
    PrintLn("  7 passed: post-move state (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestStatistic7 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
