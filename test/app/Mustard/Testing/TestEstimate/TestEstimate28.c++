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

TestEstimate28::TestEstimate28() :
    Subprogram{"TestEstimate28", "Test Mustard::Math::Estimate (Section 28: HarmonicMean)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// Reduction Smoke Test (from sec0Smoke)
constexpr auto sec25Smoke{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;
        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{v, cov};
        [[maybe_unused]] double scalar{};
        scalar = e.HarmonicMean().Value();
    }
}};

constexpr auto sec25HarmonicMean{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};

        double expectedValue{0.0};
        for (auto i{0}; i < dim; ++i) {
            expectedValue += 1.0 / v(i);
        }
        expectedValue = dim / expectedValue;

        // 27a. HarmonicMean() const&
        {
            Est e{v, cov};
            const auto result{e.HarmonicMean()};
            CheckClose(result.Value(), expectedValue, "28a: HarmonicMean() const& value");
        }

        // 27b. HarmonicMean() &&
        {
            Est e{v, cov};
            const auto result{std::move(e).HarmonicMean()};
            CheckClose(result.Value(), expectedValue, "28b: HarmonicMean() and value");
        }
    }
}};

} // namespace TestEstimateSection

auto TestEstimate28::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 28: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec25Smoke);
    PrintLn("  28 smoke passed: HarmonicMean compilation smoke");

    PrintLn("--- Section 28: HarmonicMean ---");
    RunOverAllDims<AllStaticDims>(sec25HarmonicMean);
    PrintLn("  28 passed: harmonic mean (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestEstimate28 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
