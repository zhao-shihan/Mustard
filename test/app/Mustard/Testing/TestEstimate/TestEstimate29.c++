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

TestEstimate29::TestEstimate29() :
    Subprogram{"TestEstimate29", "Test Mustard::Math::Estimate (Section 29: GeometricMean)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// Reduction Smoke Test (from sec0Smoke)
constexpr auto sec26Smoke{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;
        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{v, cov};
        [[maybe_unused]] double scalar{};
        scalar = e.GeometricMean().Value();
    }
}};

constexpr auto sec26GeometricMean{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};

        double expectedValue{1.0};
        for (auto i{0}; i < dim; ++i) {
            expectedValue *= v(i);
        }
        expectedValue = std::pow(expectedValue, 1.0 / dim);

        // 28a. GeometricMean() const&
        {
            Est e{v, cov};
            const auto result{e.GeometricMean()};
            CheckClose(result.Value(), expectedValue, "29a: GeometricMean() const& value");
        }

        // 28b. GeometricMean() &&
        {
            Est e{v, cov};
            const auto result{std::move(e).GeometricMean()};
            CheckClose(result.Value(), expectedValue, "29b: GeometricMean() and value");
        }
    }
}};

} // namespace TestEstimateSection

auto TestEstimate29::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 29: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec26Smoke);
    PrintLn("  29 smoke passed: GeometricMean compilation smoke");

    PrintLn("--- Section 29: GeometricMean ---");
    RunOverAllDims<AllStaticDims>(sec26GeometricMean);
    PrintLn("  29 passed: geometric mean (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestEstimate29 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
