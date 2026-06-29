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
#include "Mustard/Testing/TestEstimate/TestEstimate.h++"
#include "Mustard/Testing/TestEstimate/TestEstimateCommon.h++"

#include <cstdlib>

namespace Mustard::Testing {

TestEstimate27::TestEstimate27() :
    Subprogram{"TestEstimate27", "Test Mustard::Math::Estimate (Section 27: Mean)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// Reduction Smoke Test (from sec0Smoke)
constexpr auto sec24Smoke{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;
        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{v, cov};
        [[maybe_unused]] double scalar{};
        scalar = e.Mean().Value();
    }
}};

constexpr auto sec24Mean{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{v, cov};

        double expectedValue{0.0};
        for (auto i{0}; i < dim; ++i) {
            expectedValue += v(i);
        }
        expectedValue /= dim;

        // 24a. Mean value
        const auto result{e.Mean()};
        CheckClose(result.Value(), expectedValue, "27a: Mean value");

        // 24b. Mean variance == Sum variance / dim^2
        const auto sumResult{e.Sum()};
        CheckClose(result.Variance(), sumResult.Variance() / (dim * dim),
                   "27b: Mean variance = Sum variance / dim^2");
    }
}};

} // namespace TestEstimateSection

auto TestEstimate27::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 27: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec24Smoke);
    PrintLn("  27 smoke passed: Mean compilation smoke");

    PrintLn("--- Section 27: Mean ---");
    RunOverAllDims<AllStaticDims>(sec24Mean);
    PrintLn("  27 passed: mean (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestEstimate27 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
