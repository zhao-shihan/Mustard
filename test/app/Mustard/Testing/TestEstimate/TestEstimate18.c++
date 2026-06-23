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

TestEstimate18::TestEstimate18() :
    Subprogram{"TestEstimate18", "Test Mustard::Math::Estimate (Section 18: Negate and Chaining)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

constexpr auto sec15Smoke{[]<int K, CovarianceOption C>() {
    using Est = Estimate<K, C>;
    constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    const auto newEst{[&] { if constexpr (K == 1) return Est{1.0, 0.5}; else return Est{x, cov}; }};
    auto est{newEst()};
    est = newEst();
    est = std::move(est).Negate();
}};

constexpr auto sec15NegateChaining{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    constexpr bool isFull{C == CovarianceOption::Full};

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    // Negate() in-place
    {
        auto e{MakeEstimate<K, C>(x, cov)};
        e.NegateInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), -x(0), "18a: Negate value");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), -x(i), fmt::format("18a: Negate value({})", i));
            }
        }
        if constexpr (isFull and K != 1) {
            CheckClose(e.Covariance(), cov, "18a: Negate cov unchanged");
        }
    }

    // Unary operator-
    {
        auto e{MakeEstimate<K, C>(x, cov)};
        const auto e2{-e};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), -x(0), "18b: unary- value");
            CheckClose(e.Value(), x(0), "18b: original unchanged");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e2.Value(i), -x(i), fmt::format("18b: unary- value({})", i));
            }
            CheckClose(e.Value(0), x(0), "18b: original unchanged");
        }
    }

    // Chaining: Square().Sqrt() roundtrip
    {
        auto e{MakeEstimate<K, C>(x, cov)};
        e.SquareInPlace().SqrtInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::abs(x(0)), "18c: Square().Sqrt() value");
        } else {
            for (auto i{0}; i < std::min(dim, 2); ++i) {
                CheckClose(e.Value(i), std::abs(x(i)), fmt::format("18c: Square().Sqrt() value({})", i));
            }
        }
    }

    // Exp().Log() roundtrip
    {
        auto e{MakeEstimate<K, C>(x, cov)};
        e.ExpInPlace().LogInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), x(0), "18d: Exp().Log() value");
        } else {
            for (auto i{0}; i < std::min(dim, 2); ++i) {
                CheckClose(e.Value(i), x(i), fmt::format("18d: Exp().Log() value({})", i));
            }
        }
    }

    // Move semantics
    {
        auto e{MakeEstimate<K, C>(x, cov)};
        const auto e2{std::move(e).Square()};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), x(0) * x(0), "18e: move+Square value");
        } else {
            CheckClose(e2.Value(0), x(0) * x(0), "18e: move+Square value(0)");
            if constexpr (dim >= 2) {
                CheckClose(e2.Value(1), x(1) * x(1), "18e: move+Square value(1)");
            }
        }
    }
}};

} // namespace TestEstimateSection

auto TestEstimate18::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 18: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec15Smoke);
    PrintLn("  18 smoke passed: negate compilation smoke");

    PrintLn("--- Section 18: Negate and Chaining ---");
    RunOverAllDims<AllStaticDims>(sec15NegateChaining);
    PrintLn("  18 passed: negate and chaining (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestEstimate18 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
