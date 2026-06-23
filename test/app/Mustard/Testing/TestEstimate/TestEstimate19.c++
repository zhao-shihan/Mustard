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
#include "Mustard/Testing/TestEstimate/TestEstimate.h++"
#include "Mustard/Testing/TestEstimate/TestEstimateCommon.h++"

#include <cstdlib>

namespace Mustard::Testing {

TestEstimate19::TestEstimate19() :
    Subprogram{"TestEstimate19", "Test Mustard::Math::Estimate (Section 19: POD / Base64)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Section 16 Smoke: ToPOD / FromPOD / Base64 (from sec0Smoke)
// =========================================================================

constexpr auto sec16Smoke{[]<int K, CovarianceOption C>() {
    // requires static K
    constexpr int dim{K};
    constexpr bool isDynamic{K == Eigen::Dynamic};
    using Est = Estimate<K, C>;

    [[maybe_unused]] double scalar{};
    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    auto e3{MakeEstimate<K, C>(x, cov)};
    auto est{MakeEstimate<K, C>(x, cov)};

    if constexpr (not isDynamic) {
        auto pod{e3.ToPOD()};
        est.FromPOD(pod);
        auto b64{ToBase64(pod)};
        scalar = b64.size();
        pod = FromBase64<typename Est::PODType>(b64);
        scalar = pod.x[0];
    }
}};

// =========================================================================
// Section 16: ToPOD / FromPOD / Base64
// Static K only — ToPOD requires compile-time dimension
// =========================================================================

constexpr auto sec16POD{[]<int K, CovarianceOption C>() {
    // requires static K
    constexpr int dim{K};
    using Est = Estimate<K, C>;

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    // ToPOD/FromPOD roundtrip
    {
        auto e1{MakeEstimate<K, C>(x, cov)};
        const auto pod{e1.ToPOD()};
        Est e2{pod};
        CheckClose(e2.Value(), e1.Value(), "16: POD roundtrip Value");
        if constexpr (K != 1) {
            CheckClose(e2.Covariance(), e1.Covariance(), "16: POD roundtrip Cov");
        }
        if constexpr (K == 1) {
            CheckClose(e2.StdDev(), e1.StdDev(), "16: POD roundtrip StdDev");
            CheckClose(e2.Variance(), e1.Variance(), "16: POD roundtrip Variance");
        } else {
            CheckClose(e2.StdDev(0), e1.StdDev(0), "16: POD roundtrip StdDev");
            if constexpr (dim >= 2) {
                CheckClose(e2.Variance(1), e1.Variance(1), "16: POD roundtrip Variance");
            }
        }
    }

    // FromPOD into existing object
    {
        auto e1{MakeEstimate<K, C>(x, cov)};
        const auto pod{e1.ToPOD()};
        Est e2;
        e2.FromPOD(pod);
        if constexpr (K == 1) {
            CheckClose(e2.Value(), x(0), "16: FromPOD into existing Value()");
        } else {
            CheckClose(e2.Value(0), x(0), "16: FromPOD into existing Value(0)");
            if constexpr (dim >= 2) {
                CheckClose(e2.Covariance(1, 1), e1.Covariance(1, 1), "16: FromPOD into existing Cov(1,1)");
            }
        }
    }

    // Base64 encode/decode roundtrip
    {
        auto e1{MakeEstimate<K, C>(x, cov)};
        const auto pod{e1.ToPOD()};
        const auto b64{ToBase64(pod)};
        const auto decoded{FromBase64<EstimatePOD<K, C>>(b64)};
        CheckClose(decoded.x[0], pod.x[0], "16: Base64 x[0] roundtrip");
        if constexpr (dim >= 2) {
            CheckClose(decoded.x[1], pod.x[1], "16: Base64 x[1] roundtrip");
        }

        Est e2{decoded};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), e1.Value(), "16: Base64 Estimate Value()");
        } else {
            CheckClose(e2.Value(0), e1.Value(0), "16: Base64 Estimate Value(0)");
            if constexpr (dim >= 2) {
                CheckClose(e2.Covariance(0, 1), e1.Covariance(0, 1), "16: Base64 Estimate Cov(0,1)");
            }
        }
    }

    // DecodeBase64 method
    {
        auto e1{MakeEstimate<K, C>(x, cov)};
        const auto pod{e1.ToPOD()};
        const auto b64{ToBase64(pod)};
        const auto decoded{FromBase64<EstimatePOD<K, C>>(b64)};
        CheckClose(decoded.x[0], x(0), "16: DecodeBase64 x[0]");
    }
}};

// =========================================================================
// Section 16: Invalid Base64 Throws
// =========================================================================

constexpr auto sec16InvalidBase64{[] {
    auto threw{false};
    try {
        FromBase64<EstimatePOD<2, CovarianceOption::Full>>("!!!not_valid_base64!!!");
    } catch (const std::runtime_error&) {
        threw = true;
    }
    if (not threw) {
        Throw<std::runtime_error>("19f: invalid Base64 should throw");
    }
}};

} // namespace TestEstimateSection

auto TestEstimate19::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 19: Smoke Test ---");
    RunOverStaticDims<AllStaticDims>(sec16Smoke);
    PrintLn("  19 smoke passed: POD compilation smoke");

    PrintLn("--- Section 19: POD ---");
    RunOverStaticDims<AllStaticDims>(sec16POD);
    PrintLn("  19 passed: ToPOD/FromPOD/Base64 Full/Diag (K=1,2,3,5,10)");

    sec16InvalidBase64();
    PrintLn("  19f passed: invalid Base64 throws");

    PrintLn("All TestEstimate19 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
