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

TestEstimate23::TestEstimate23() :
    Subprogram{"TestEstimate23", "Test Mustard::Math::Estimate (Section 23: Sum)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Section 20: Sum
// =========================================================================

// Reduction Smoke Test (from sec0Smoke)
constexpr auto sec20Smoke{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;
        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{v, cov};
        [[maybe_unused]] double scalar{};
        scalar = e.Sum().Value();
    }
}};

constexpr auto sec20Sum{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{v, cov};

        // 22a. Sum value
        const auto result{e.Sum()};
        double expectedValue{0.0};
        for (auto i{0}; i < dim; ++i) {
            expectedValue += v(i);
        }
        CheckClose(result.Value(), expectedValue, "23a: Sum value");

        // 22b. Sum variance
        double expectedVar{0.0};
        if constexpr (C == CovarianceOption::Full) {
            for (auto i{0}; i < dim; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    expectedVar += e.Covariance(i, j);
                }
            }
        } else {
            for (auto i{0}; i < dim; ++i) {
                expectedVar += e.Variance(i);
            }
        }
        CheckClose(result.Variance(), expectedVar, "23b: Sum variance");
    }
}};

} // namespace TestEstimateSection

auto TestEstimate23::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 23: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec20Smoke);
    PrintLn("  23 smoke passed: Sum compilation smoke");

    PrintLn("--- Section 23: Sum ---");
    RunOverAllDims<AllStaticDims>(sec20Sum);
    PrintLn("  23 passed: sum (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestEstimate23 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
