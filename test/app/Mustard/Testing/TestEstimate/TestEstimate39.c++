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

TestEstimate39::TestEstimate39() :
    Subprogram{"TestEstimate39", "Test Mustard::Math::Estimate (Section 39: User-defined Reduce)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// K>=2 static dimensions to test Reduce (K=1 not supported)
using ReduceStaticDims = std::integer_sequence<int, 2, 3, 5, 10>;

// =========================================================================
// Section 39 Smoke: Reduce compilation (K>=2 + dynamic)
// =========================================================================

constexpr auto sec39Smoke{[]<int K, CovarianceOption C>() {
    using Est = Estimate<K, C>;
    constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    [[maybe_unused]] double scalar{};
    [[maybe_unused]] Estimate<1, C> sEst{};

    const auto newEst{[&] {
        if constexpr (K == 1) {
            return Est{1.0, 0.5};
        } else {
            return Est{x, cov};
        }
    }};

    auto est{newEst()};

    constexpr auto f{[]<typename T>(const T& v) -> typename T::Scalar { return v.squaredNorm(); }};
    auto r1{est.Reduce(f)};
    auto r2{newEst().Reduce(f)};
    scalar = r1.Value();
    scalar = r2.Value();
}};

// =========================================================================
// Section 39: Reduce with squaredNorm
// =========================================================================

constexpr auto sec39ReduceSquaredNorm{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    auto e{MakeEstimate<K, C>(x, cov)};

    auto eReduce{e.Reduce([]<typename T>(const T& v) -> typename T::Scalar { return v.squaredNorm(); })};

    // Analytic: f = sum(x_i^2), df/dx_i = 2*x_i
    double expectedVal{0};
    double expectedVar{0};
    for (int i{0}; i < dim; ++i) {
        expectedVal += x(i) * x(i);
    }
    for (int i{0}; i < dim; ++i) {
        for (int j{0}; j < dim; ++j) {
            double covIJ;
            if constexpr (C == CovarianceOption::Full) {
                covIJ = cov(i, j);
            } else {
                covIJ = (i == j) ? cov.diagonal()(i) : 0.0;
            }
            expectedVar += 2 * x(i) * covIJ * 2 * x(j);
        }
    }
    CheckClose(eReduce.Value(), expectedVal, "39a: Reduce squaredNorm value");
    CheckClose(eReduce.Variance(), expectedVar, "39a: Reduce squaredNorm var");
}};

// =========================================================================
// Section 39: Reduce with sum
// =========================================================================

constexpr auto sec39ReduceSum{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    auto e{MakeEstimate<K, C>(x, cov)};

    auto eReduce{e.Reduce([]<typename T>(const T& v) -> typename T::Scalar { return v.sum(); })};

    double expectedVal{0};
    double expectedVar{0};
    for (int i{0}; i < dim; ++i) {
        expectedVal += x(i);
        for (int j{0}; j < dim; ++j) {
            if constexpr (C == CovarianceOption::Full) {
                expectedVar += cov(i, j);
            } else {
                if (i == j) {
                    expectedVar += cov.diagonal()(i);
                }
            }
        }
    }
    CheckClose(eReduce.Value(), expectedVal, "39b: Reduce sum value");
    CheckClose(eReduce.Variance(), expectedVar, "39b: Reduce sum var");
}};

// =========================================================================
// Section 39: Reduce with composite f(v) = v(0)^2 * exp(v(1))
// =========================================================================

constexpr auto sec39ReduceComposite{[]<int K, CovarianceOption C>() {
    constexpr int dim{2};

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    auto e{MakeEstimate<K, C>(x, cov)};

    auto result{e.Reduce([]<typename T>(const T& v) -> typename T::Scalar {
        return v(0) * v(0) * exp(v(1));
    })};

    double val{x(0) * x(0) * std::exp(x(1))};
    CheckClose(result.Value(), val, "39c: Reduce composite value");

    double j0{2 * x(0) * std::exp(x(1))};
    double j1{x(0) * x(0) * std::exp(x(1))};
    double expectedVar;
    if constexpr (C == CovarianceOption::Full) {
        expectedVar = j0 * j0 * cov(0, 0) + j1 * j1 * cov(1, 1) + 2 * j0 * j1 * cov(0, 1);
    } else {
        expectedVar = j0 * j0 * cov.diagonal()(0) + j1 * j1 * cov.diagonal()(1);
    }
    CheckClose(result.Variance(), expectedVar, "39c: Reduce composite var");
}};

// =========================================================================
// Section 39: Reduce with constant function f(v) = 3.0
// =========================================================================

constexpr auto sec39ReduceConstant{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    auto e{MakeEstimate<K, C>(x, cov)};

    auto result{e.Reduce([]<typename T>(const T&) -> typename T::Scalar { return 3.0; })};

    CheckClose(result.Value(), 3.0, "39d: Reduce constant value");
    CheckClose(result.Variance(), 0.0, "39d: Reduce constant var");
}};

// =========================================================================
// Section 39: Reduce with linear f(v) = 2*v(0) + 3*v(1)
// =========================================================================

constexpr auto sec39ReduceLinear{[]<int K, CovarianceOption C>() {
    constexpr int dim{2};

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    auto e{MakeEstimate<K, C>(x, cov)};

    auto result{e.Reduce([]<typename T>(const T& v) -> typename T::Scalar { return 2 * v(0) + 3 * v(1); })};

    CheckClose(result.Value(), 2 * x(0) + 3 * x(1), "39e: Reduce linear value");

    double expectedVar;
    if constexpr (C == CovarianceOption::Full) {
        expectedVar = 4 * cov(0, 0) + 9 * cov(1, 1) + 12 * cov(0, 1);
    } else {
        expectedVar = 4 * cov.diagonal()(0) + 9 * cov.diagonal()(1);
    }
    CheckClose(result.Variance(), expectedVar, "39e: Reduce linear var");
}};

// =========================================================================
// Section 39: Reduce with dynamic dimension
// =========================================================================

constexpr auto sec39ReduceDynamic{[] {
    constexpr int dyn{Eigen::Dynamic};
    constexpr auto full{CovarianceOption::Full};
    constexpr int dim{3};

    const auto x{MakeTestValue<dyn, full>(dim)};
    const auto cov{MakeTestCov<dyn, full>(dim)};
    auto e{MakeEstimate<dyn, full>(x, cov)};

    auto result{e.Reduce([]<typename T>(const T& v) -> typename T::Scalar {
        return v(0) * v(0) + v(1) * v(2);
    })};

    double val{x(0) * x(0) + x(1) * x(2)};
    CheckClose(result.Value(), val, "39f: Reduce dynamic value");

    Eigen::VectorXd j(3);
    j << 2 * x(0), x(2), x(1);
    double expectedVar{j.transpose() * cov * j};
    CheckClose(result.Variance(), expectedVar, "39f: Reduce dynamic var");
}};

} // namespace TestEstimateSection

auto TestEstimate39::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 39: Smoke Test ---");
    RunOverStaticDims<ReduceStaticDims>(sec39Smoke);
    RunOverDynamic(sec39Smoke);
    PrintLn("  39 smoke passed: compilation (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 39: Reduce squaredNorm ---");
    RunOverStaticDims<ReduceStaticDims>(sec39ReduceSquaredNorm);
    RunOverDynamic(sec39ReduceSquaredNorm);
    PrintLn("  39a passed: Reduce squaredNorm");

    PrintLn("--- Section 39: Reduce sum ---");
    RunOverStaticDims<ReduceStaticDims>(sec39ReduceSum);
    RunOverDynamic(sec39ReduceSum);
    PrintLn("  39b passed: Reduce sum");

    PrintLn("--- Section 39: Reduce composite f(v)=v0^2*exp(v1) ---");
    RunOverStaticDims<ReduceStaticDims>(sec39ReduceComposite);
    PrintLn("  39c passed: composite function covariance");

    PrintLn("--- Section 39: Reduce constant f(v)=3.0 ---");
    RunOverStaticDims<ReduceStaticDims>(sec39ReduceConstant);
    RunOverDynamic(sec39ReduceConstant);
    PrintLn("  39d passed: constant function zeroes variance");

    PrintLn("--- Section 39: Reduce linear f(v)=2*v0+3*v1 ---");
    RunOverStaticDims<ReduceStaticDims>(sec39ReduceLinear);
    PrintLn("  39e passed: linear function gradient=[2,3]");

    PrintLn("--- Section 39: Reduce dynamic dimension ---");
    sec39ReduceDynamic();
    PrintLn("  39f passed: dynamic dimension");

    PrintLn("All TestEstimate39 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
