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

TestEstimate32::TestEstimate32() :
    Subprogram{"TestEstimate32", "Test Mustard::Math::Estimate (Section 32: LpNorm)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// LpNorm Smoke Test (from sec0Smoke)
constexpr auto sec29Smoke{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;
        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{v, cov};
        [[maybe_unused]] double scalar{};
        scalar = e.template LpNorm<1.0>().Value();
        scalar = e.template LpNorm<2.0>().Value();
        scalar = e.template LpNorm<3.0>().Value();
        scalar = e.template LpNorm<4.0>().Value();
        scalar = e.template LpNorm<42.0>().Value();
    }
}};

constexpr auto sec29LpNorm{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};

        // 31a. LpNorm<1.0> — L1 norm = sum(|x_i|)
        {
            Est e{v, cov};
            const auto result{std::move(e).template LpNorm<1.0>()};
            double expected{0.0};
            for (auto i{0}; i < dim; ++i) {
                expected += std::abs(v(i));
            }
            CheckClose(result.Value(), expected, "32a: LpNorm<1.0> value");
        }

        // 31b. LpNorm<2.0> — L2 norm = sqrt(sum(x_i^2)), matches Norm()
        {
            Est e1{v, cov};
            Est e2{v, cov};
            const auto lpResult{std::move(e1).template LpNorm<2.0>()};
            const auto normResult{std::move(e2).Norm()};
            CheckClose(lpResult.Value(), normResult.Value(), "32b: LpNorm<2.0> == Norm value");
            CheckClose(lpResult.Variance(), normResult.Variance(), "32b: LpNorm<2.0> == Norm variance");
        }

        // 31c. LpNorm<3.0> — L3 norm
        {
            Est e{v, cov};
            const auto result{std::move(e).template LpNorm<3.0>()};
            double expected{0.0};
            for (auto i{0}; i < dim; ++i) {
                expected += std::pow(std::abs(v(i)), 3.0);
            }
            expected = std::cbrt(expected);
            CheckClose(result.Value(), expected, "32c: LpNorm<3.0> value");
        }

        // 31d. LpNorm<4.0> — general case
        {
            Est e{v, cov};
            const auto result{std::move(e).template LpNorm<4.0>()};
            double expected{0.0};
            for (auto i{0}; i < dim; ++i) {
                expected += std::pow(std::abs(v(i)), 4.0);
            }
            expected = std::pow(expected, 0.25);
            CheckClose(result.Value(), expected, "32d: LpNorm<4.0> value");
        }

        // 31e. LpNorm const& overload
        {
            Est e{v, cov};
            const auto result{e.template LpNorm<1.5>()};
            double expected{0.0};
            for (auto i{0}; i < dim; ++i) {
                expected += std::pow(std::abs(v(i)), 1.5);
            }
            expected = std::pow(expected, 1.0 / 1.5);
            CheckClose(result.Value(), expected, "32e: LpNorm<1.5> const& value");
        }
    }
}};

} // namespace TestEstimateSection

auto TestEstimate32::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 32: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec29Smoke);
    PrintLn("  32 smoke passed: LpNorm compilation smoke");

    PrintLn("--- Section 32: LpNorm ---");
    RunOverAllDims<AllStaticDims>(sec29LpNorm);
    PrintLn("  32 passed: LpNorm (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestEstimate32 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
