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

TestEstimate26::TestEstimate26() :
    Subprogram{"TestEstimate26", "Test Mustard::Math::Estimate (Section 26: Norm)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Section 23: Norm
// =========================================================================

// Reduction Smoke Test (from sec0Smoke)
constexpr auto sec23Smoke{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;
        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{v, cov};
        [[maybe_unused]] double scalar{};
        scalar = e.Norm().Value();
    }
}};

constexpr auto sec23Norm{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};

        double expectedValue{0.0};
        for (auto i{0}; i < dim; ++i) {
            expectedValue += v(i) * v(i);
        }
        expectedValue = std::sqrt(expectedValue);

        // 25a. Norm() const&
        {
            Est e{v, cov};
            const auto result{e.Norm()};
            CheckClose(result.Value(), expectedValue, "26a: Norm() const& value");
        }

        // 25b. Norm() &&
        {
            Est e{v, cov};
            const auto result{std::move(e).Norm()};
            CheckClose(result.Value(), expectedValue, "26b: Norm() and value");
        }

        // 25c. Norm() == SquaredNorm().Sqrt()
        {
            Est e1{v, cov};
            Est e2{v, cov};
            const auto normResult{e1.Norm()};
            auto sqNormResult{std::move(e2).SquaredNorm()};
            sqNormResult.SqrtInPlace();
            CheckClose(normResult.Value(), sqNormResult.Value(), "26c: Norm == SquaredNorm().Sqrt() value");
        }
    }
}};

} // namespace TestEstimateSection

auto TestEstimate26::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 26: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec23Smoke);
    PrintLn("  26 smoke passed: Norm compilation smoke");

    PrintLn("--- Section 26: Norm ---");
    RunOverAllDims<AllStaticDims>(sec23Norm);
    PrintLn("  26 passed: norm (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestEstimate26 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
