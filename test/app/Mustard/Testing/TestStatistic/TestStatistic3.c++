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

TestStatistic3::TestStatistic3() :
    Subprogram{"TestStatistic3", "Test Mustard::Statistic (Section 3: POD and Base64)."} {}

namespace TestStatisticSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestStatistic;

// =========================================================================
// Section 3 Smoke: POD and Base64 (from sec0SmokePOD)
// =========================================================================

constexpr auto sec3Smoke{[]<int K, CovarianceOption C>() {
    static_assert(K != Eigen::Dynamic, "POD requires static dimension");
    using Stat = Statistic<K, C>;
    using POD = StatisticPOD<K, C>;

    Stat s;
    if constexpr (K == 1) {
        s.Fill(5.0);
    } else {
        s.Fill(MakeSeqVector<K>(K, 1.0));
    }
    auto pod{s.ToPOD()};
    [[maybe_unused]] POD podCopy{pod};

    Stat s2;
    if constexpr (K == 1) {
        s2.Fill(6.0);
    } else {
        s2.Fill(MakeSeqVector<K>(K, 3.0));
    }
    auto pod2{s2.ToPOD()};
    [[maybe_unused]] auto podSum{pod + pod2};

    // Base64 roundtrip
    auto b64{ToBase64(pod)};
    [[maybe_unused]] auto decoded{FromBase64<POD>(b64)};
    Stat s3{decoded};
    [[maybe_unused]] auto d{s3.Dimension()};
}};

// =========================================================================
// Section 3: POD roundtrip (static K only)
// =========================================================================

constexpr auto sec3POD{[]<int K, CovarianceOption C>() {
    static_assert(K != Eigen::Dynamic, "POD requires static dimension");
    using Stat = Statistic<K, C>;
    constexpr auto dim{K};

    Stat s1;
    if constexpr (K == 1) {
        for (auto v{1}; v <= 3; ++v) {
            s1.Fill(static_cast<double>(v));
        }
    } else {
        for (auto v{1}; v <= 3; ++v) {
            s1.Fill(v * MakeSeqVector<K>(dim, 1.0));
        }
    }

    auto pod{s1.ToPOD()};
    Stat s2{pod};

    CheckEq(s2.SampleSize(), s1.SampleSize(), "3: SampleSize roundtrip");
    CheckClose(s2.WeightSum(), s1.WeightSum(), "3: WeightSum roundtrip");
    if constexpr (K == 1) {
        CheckClose(s2.Mean(), s1.Mean(), "3: Mean() roundtrip");
        CheckClose(s2.Variance(), s1.Variance(), "3: Variance() roundtrip");
    } else {
        for (auto i{0}; i < dim; ++i) {
            CheckClose(s2.Mean(i), s1.Mean(i),
                       fmt::format("3: Mean({}) roundtrip", i));
            CheckClose(s2.Variance(i), s1.Variance(i),
                       fmt::format("3: Variance({}) roundtrip", i));
        }
        if constexpr (C == CovarianceOption::Full) {
            CheckClose(s2.Covariance(0, 1), s1.Covariance(0, 1), "3: Cov(0,1) roundtrip");
        }
    }
}};

// =========================================================================
// Section 3: Base64 Encoding (static K only)
// =========================================================================

constexpr auto sec3Base64Encoding{[]<int K, CovarianceOption C>() {
    static_assert(K != Eigen::Dynamic, "Base64 requires static dimension");
    using Stat = Statistic<K, C>;
    using POD = StatisticPOD<K, C>;

    Stat s1;
    if constexpr (K == 1) {
        for (auto v{1}; v <= 3; ++v) {
            s1.Fill(static_cast<double>(v));
        }
    } else {
        for (auto v{1}; v <= 3; ++v) {
            s1.Fill(v * MakeSeqVector<K>(K, 1.0));
        }
    }

    auto pod{s1.ToPOD()};
    auto b64{ToBase64(pod)};
    auto decoded{FromBase64<POD>(b64)};
    Stat s2{decoded};

    if constexpr (K == 1) {
        CheckClose(s2.Mean(), s1.Mean(), "3: Mean() Base64 roundtrip");
        CheckClose(s2.Variance(), s1.Variance(), "3: Variance() Base64 roundtrip");
    } else {
        CheckClose(s2.Mean(0), s1.Mean(0), "3: Mean(0) Base64 roundtrip");
        CheckClose(s2.Variance(0), s1.Variance(0), "3: Variance(0) Base64 roundtrip");
        if constexpr (C == CovarianceOption::Full) {
            CheckClose(s2.Covariance(0, 1), s1.Covariance(0, 1), "3: Cov(0,1) Base64 roundtrip");
        }
    }
}};

// =========================================================================
// Section 3: Invalid Base64
// =========================================================================

constexpr auto sec3InvalidBase64{[] {
    StatisticPOD<1> data;
    auto threw{false};
    try {
        data = FromBase64<StatisticPOD<1>>("not_valid_base64!!!");
    } catch (const std::runtime_error&) {
        threw = true;
    }
    if (not threw) {
        Throw<std::runtime_error>("3: invalid Base64 should throw std::runtime_error");
    }
}};

// =========================================================================
// Section 3: StatisticPOD operator+
// =========================================================================

constexpr auto sec3PODAdd{[] {
    Statistic<1> s1;
    s1.Fill(1.0);
    s1.Fill(2.0);
    Statistic<1> s2;
    s2.Fill(3.0);
    s2.Fill(4.0);

    auto pod1{s1.ToPOD()};
    auto pod2{s2.ToPOD()};
    auto pod3{pod1 + pod2};

    Statistic<1> sFull;
    sFull.Fill(1.0);
    sFull.Fill(2.0);
    sFull.Fill(3.0);
    sFull.Fill(4.0);

    Statistic<1> s3{pod3};
    CheckClose(s3.Mean(), sFull.Mean(), "3: Mean from StatisticPOD::operator+");
    CheckClose(s3.Variance(), sFull.Variance(), "3: Variance from StatisticPOD::operator+");
}};

} // namespace TestStatisticSection

auto TestStatistic3::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestStatisticSection;

    PrintLn("--- Section 3: Smoke Test ---");
    RunOverStaticDims<AllStaticDims>(sec3Smoke);
    PrintLn("  3 smoke passed: POD compilation smoke (K=1,2,3,5,10 Full/Diag)");

    PrintLn("--- Section 3: POD ---");
    RunOverStaticDims<AllStaticDims>(sec3POD);
    PrintLn("  3 passed: ToPOD/FromPOD Full/Diag (K=1,2,3,5,10)");

    PrintLn("--- Section 3: Base64 ---");
    RunOverStaticDims<AllStaticDims>(sec3Base64Encoding);
    PrintLn("  3 passed: Base64 Full/Diag (K=1,2,3,5,10)");
    sec3InvalidBase64();
    PrintLn("  3 passed: invalid Base64 throws");
    sec3PODAdd();
    PrintLn("  3 passed: StatisticPOD operator+");

    PrintLn("All TestStatistic3 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
