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

TestEstimate40::TestEstimate40() :
    Subprogram{"TestEstimate40", "Test Mustard::Math::Estimate (Section 40: User-defined Transform)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// K>=2 static dimensions to test Transform
using TransformStaticDims = std::integer_sequence<int, 2, 3, 5>;

// =========================================================================
// Section 40 Smoke: Transform compilation (compile-time L + runtime L via Eigen::Dynamic)
// =========================================================================

constexpr auto sec40Smoke{[]<int K, CovarianceOption C>() {
    using Est = Estimate<K, C>;
    constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    [[maybe_unused]] double scalar{};

    const auto newEst{[&] {
        if constexpr (K == 1) {
            return Est{1.0, 0.5};
        } else {
            return Est{x, cov};
        }
    }};

    auto est{newEst()};

    // ---- compile-time L Transform ----
    const auto f{[]<typename T>(const T& v) -> ADVector<2, K> {
        ADVector<2, K> r;
        r[0] = v[0] * v[0];
        r[1] = v.sum();
        return r;
    }};
    auto r1{est.template Transform<2>(f)};
    auto r2{newEst().template Transform<2>(f)};
    scalar = r1.Value(0);
    scalar = r2.Value(0);

    // ---- runtime L Transform via Eigen::Dynamic ----
    const auto fDyn{[]<typename T>(const T& v) -> ADVector<Eigen::Dynamic, K> {
        ADVector<Eigen::Dynamic, K> r(2);
        r[0] = v[0] * v[0];
        r[1] = v.sum();
        return r;
    }};
    auto r3{est.template Transform<Eigen::Dynamic>(fDyn)};
    auto r4{newEst().template Transform<Eigen::Dynamic>(fDyn)};
    scalar = r3.Value(0);
    scalar = r4.Value(0);
}};

// =========================================================================
// Section 40a: Transform with linear map f(x) = A*x, compare to LeftMultiply
// =========================================================================

constexpr auto sec40aLinear{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    constexpr int outDim{2};

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    auto e{MakeEstimate<K, C>(x, cov)};

    // Build a 2xK matrix A: A(0,:)=[1,2,...,0], A(1,:)=[1,1,...,0]
    using AMat = Eigen::Matrix<double, outDim, K>;
    AMat A;
    if constexpr (K == Eigen::Dynamic) {
        A = AMat::Zero(outDim, dim);
    } else {
        A.setZero();
    }
    for (int i{0}; i < dim; ++i) {
        A(0, i) = i + 1;
    }
    for (int i{0}; i < dim; ++i) {
        A(1, i) = 1.0;
    }

    auto f{[&A]<typename T>(const T& v) -> ADVector<outDim, K> {
        ADVector<outDim, K> r;
        for (int j{0}; j < outDim; ++j) {
            r[j] = A(j, 0) * v[0];
            for (int i{1}; i < v.size(); ++i) {
                r[j] += A(j, i) * v[i];
            }
        }
        return r;
    }};

    auto result{e.template Transform<outDim>(f)};
    auto ref{e.LeftMultiply(A)};

    // Compare values
    for (int j{0}; j < outDim; ++j) {
        CheckClose(result.Value(j), ref.Value(j), fmt::format("40a: Linear value[{}]", j));
    }
    // Compare covariances
    for (int j{0}; j < outDim; ++j) {
        for (int k{0}; k < outDim; ++k) {
            CheckClose(result.Covariance(j, k), ref.Covariance(j, k),
                       fmt::format("40a: Linear cov({},{})", j, k));
        }
    }
}};

// =========================================================================
// Section 40b: Transform with sin/cos map f(x) = [sin(x0), cos(x1)]
// =========================================================================

constexpr auto sec40bSinCos{[]<int K, CovarianceOption C>() {
    constexpr int dim{2};
    constexpr int outDim{2};

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    auto e{MakeEstimate<K, C>(x, cov)};

    auto f{[]<typename T>(const T& v) -> ADVector<outDim, K> {
        ADVector<outDim, K> r;
        r[0] = sin(v[0]);
        r[1] = cos(v[1]);
        return r;
    }};

    auto result{e.template Transform<outDim>(f)};

    // Analytic values
    double v0{std::sin(x(0))};
    double v1{std::cos(x(1))};
    CheckClose(result.Value(0), v0, "40b: SinCos value[0]");
    CheckClose(result.Value(1), v1, "40b: SinCos value[1]");

    // Analytic Jacobian: [[cos(x0), 0], [0, -sin(x1)]]
    double j00{std::cos(x(0))};
    double j11{-std::sin(x(1))};
    for (int i{0}; i < outDim; ++i) {
        for (int j{0}; j < outDim; ++j) {
            double jacI{(i == 0) ? j00 : ((i == 1) ? j11 : 0.0)};
            double jacJ{(j == 0) ? j00 : ((j == 1) ? j11 : 0.0)};
            double expectedCov;
            if constexpr (C == CovarianceOption::Full) {
                expectedCov = jacI * cov(i, j) * jacJ;
            } else {
                expectedCov = (i == j) ? jacI * cov.diagonal()(i) * jacJ : 0.0;
            }
            CheckClose(result.Covariance(i, j), expectedCov,
                       fmt::format("40b: SinCos cov({},{})", i, j));
        }
    }
}};

// =========================================================================
// Section 40c: Composite transform f(x) = [x0*x1, x0/x1, x0^2] (K=2->L=3)
// =========================================================================

constexpr auto sec40cComposite{[]<int K, CovarianceOption C>() {
    constexpr int dim{2};
    constexpr int outDim{3};

    const auto x{MakeTestValue<K, C>(2)};
    const auto cov{MakeTestCov<K, C>(2)};
    auto e{MakeEstimate<K, C>(x, cov)};

    auto f{[]<typename T>(const T& v) -> ADVector<outDim, K> {
        ADVector<outDim, K> r;
        r[0] = v[0] * v[1];
        r[1] = v[0] / v[1];
        r[2] = v[0] * v[0];
        return r;
    }};

    auto result{e.template Transform<outDim>(f)};

    // Analytic values
    double x0{x(0)}, x1{x(1)};
    double val0{x0 * x1};
    double val1{x0 / x1};
    double val2{x0 * x0};
    CheckClose(result.Value(0), val0, "40c: Composite val[0]");
    CheckClose(result.Value(1), val1, "40c: Composite val[1]");
    CheckClose(result.Value(2), val2, "40c: Composite val[2]");

    // Jacobian rows:
    // r0: [x1, x0]
    // r1: [1/x1, -x0/x1^2]
    // r2: [2*x0, 0]
    Eigen::Matrix<double, outDim, 2> jac;
    jac << x1, x0,
        1.0 / x1, -x0 / muc::pow(x1, 2),
        2 * x0, 0.0;

    for (int j{0}; j < outDim; ++j) {
        for (int k{0}; k < outDim; ++k) {
            if constexpr (C == CovarianceOption::Diagonal) {
                if (j != k) {
                    CheckClose(result.Covariance(j, k), 0.0,
                               fmt::format("40c: Composite cov({},{}) (Diagonal => 0)", j, k));
                    continue;
                }
            }
            double analytic{0};
            for (int m{0}; m < dim; ++m) {
                for (int n{0}; n < dim; ++n) {
                    double covMN;
                    if constexpr (C == CovarianceOption::Full) {
                        covMN = cov(m, n);
                    } else {
                        covMN = (m == n) ? cov.diagonal()(m) : 0.0;
                    }
                    analytic += jac(j, m) * covMN * jac(k, n);
                }
            }
            CheckClose(result.Covariance(j, k), analytic,
                       fmt::format("40c: Composite cov({},{})", j, k));
        }
    }
}};

// =========================================================================
// Section 40d: Transform with Diagonal covariance input
// =========================================================================

constexpr auto sec40dDiagonal{[] {
    constexpr int K{3};
    constexpr int L{2};
    constexpr auto diag{CovarianceOption::Diagonal};

    const auto x{MakeTestValue<K, diag>(K)};
    const auto cov{MakeTestCov<K, diag>(K)};
    auto e{MakeEstimate<K, diag>(x, cov)};

    auto f{[]<typename T>(const T& v) -> ADVector<L, K> {
        ADVector<L, K> r;
        r[0] = v[0] + v[1];
        r[1] = v[0] * v[2];
        return r;
    }};

    auto result{e.template Transform<L>(f)};

    // Analytic Jacobian: [[1, 1, 0], [x2, 0, x0]]
    Eigen::Matrix<double, L, K> jac;
    jac << 1.0, 1.0, 0.0,
        x(2), 0.0, x(0);

    // Diagonal: result[i,i] = sum_j jac(i,j)^2 * var(j)
    for (int i{0}; i < L; ++i) {
        double expectedVar{0};
        for (int j{0}; j < K; ++j) {
            expectedVar += jac(i, j) * jac(i, j) * cov.diagonal()(j);
        }
        CheckClose(result.Covariance(i, i), expectedVar, fmt::format("40d: Diagonal cov({},{})", i, i));
        // off-diagonal should be 0 for Diagonal
        for (int k{0}; k < L; ++k) {
            if (i != k) {
                CheckClose(result.Covariance(i, k), 0.0, fmt::format("40d: Diagonal cov({},{})", i, k));
            }
        }
    }
}};

// =========================================================================
// Section 40e: Transform with dynamic input dimension
// =========================================================================

constexpr auto sec40eDynamic{[] {
    constexpr int dyn{Eigen::Dynamic};
    constexpr auto full{CovarianceOption::Full};
    constexpr int dim{3};
    constexpr int outDim{2};

    const auto x{MakeTestValue<dyn, full>(dim)};
    const auto cov{MakeTestCov<dyn, full>(dim)};
    auto e{MakeEstimate<dyn, full>(x, cov)};

    auto f{[]<typename T>(const T& v) -> ADVector<outDim, dyn> {
        ADVector<outDim, dyn> r;
        r[0] = v[0] * v[0] + v[1];
        r[1] = v[0] + v[1] + v[2];
        return r;
    }};

    auto result{e.template Transform<outDim>(f)};

    // Analytic
    double val0{x(0) * x(0) + x(1)};
    double val1{x(0) + x(1) + x(2)};
    CheckClose(result.Value(0), val0, "40e: Dynamic val[0]");
    CheckClose(result.Value(1), val1, "40e: Dynamic val[1]");

    Eigen::Matrix<double, outDim, Eigen::Dynamic> jac(outDim, dim);
    jac << 2 * x(0), 1.0, 0.0,
        1.0, 1.0, 1.0;
    Eigen::Matrix2d analyticCov{jac * cov * jac.transpose()};
    CheckClose(result.Covariance(0, 0), analyticCov(0, 0), "40e: Dynamic cov(0,0)");
    CheckClose(result.Covariance(0, 1), analyticCov(0, 1), "40e: Dynamic cov(0,1)");
    CheckClose(result.Covariance(1, 0), analyticCov(1, 0), "40e: Dynamic cov(1,0)");
    CheckClose(result.Covariance(1, 1), analyticCov(1, 1), "40e: Dynamic cov(1,1)");
}};

// =========================================================================
// Section 40f: Transform with runtime output dimension via Eigen::Dynamic
// =========================================================================

constexpr auto sec40fRuntimeL{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    auto e{MakeEstimate<K, C>(x, cov)};

    auto f{[]<typename T>(const T& v) -> ADVector<Eigen::Dynamic, K> {
        ADVector<Eigen::Dynamic, K> r(3);
        r[0] = v[0] * v[0];
        r[1] = v.sum();
        r[2] = v[0] * v[1];
        return r;
    }};

    auto result{e.template Transform<Eigen::Dynamic>(f)};

    double x0{dim >= 1 ? x(0) : 0.0};
    double x1{dim >= 2 ? x(1) : 0.0};
    double sum{0};
    for (int i{0}; i < dim; ++i) {
        sum += x(i);
    }

    CheckClose(result.Value(0), x0 * x0, "40f: Dynamic L val[0]");
    CheckClose(result.Value(1), sum, "40f: Dynamic L val[1]");
    CheckClose(result.Value(2), x0 * x1, "40f: Dynamic L val[2]");
}};

} // namespace TestEstimateSection

auto TestEstimate40::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 40: Smoke Test ---");
    RunOverStaticDims<TransformStaticDims>(sec40Smoke);
    RunOverDynamic(sec40Smoke);
    PrintLn("  40 smoke passed: compilation (K=2,3,5 Full/Diag + dynamic, compile-time L + runtime L via Eigen::Dynamic)");

    PrintLn("--- Section 40a: Linear transform f(x)=A*x ---");
    RunOverStaticDims<TransformStaticDims>(sec40aLinear);
    RunOverDynamic(sec40aLinear);
    PrintLn("  40a passed: matches LeftMultiply");

    PrintLn("--- Section 40b: Sin/Cos transform ---");
    RunOverStaticDims<TransformStaticDims>(sec40bSinCos);
    PrintLn("  40b passed: sin/cos Jacobian verified");

    PrintLn("--- Section 40c: Composite f(x)=[x0*x1, x0/x1, x0^2] ---");
    RunOverStaticDims<TransformStaticDims>(sec40cComposite);
    PrintLn("  40c passed: composite Jacobian verified");

    PrintLn("--- Section 40d: Diagonal covariance input ---");
    sec40dDiagonal();
    PrintLn("  40d passed: Diagonal covariance propagation");

    PrintLn("--- Section 40e: Dynamic input dimension ---");
    sec40eDynamic();
    PrintLn("  40e passed: dynamic input dimension");

    PrintLn("--- Section 40f: Runtime output dimension via Eigen::Dynamic ---");
    RunOverStaticDims<TransformStaticDims>(sec40fRuntimeL);
    RunOverDynamic(sec40fRuntimeL);
    PrintLn("  40f passed: dynamic L Transform");

    PrintLn("All TestEstimate40 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
