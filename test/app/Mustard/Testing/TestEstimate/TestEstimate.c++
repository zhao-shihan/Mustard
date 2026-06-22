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
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/IO/Print.h++"
#include "Mustard/Math/Estimate.h++"
#include "Mustard/Testing/TestEstimate/TestEstimate.h++"
#include "Mustard/Utility/Base64.h++"

#include "Eigen/Core"

#include "muc/numeric"

#include "fmt/format.h"
#include "fmt/ostream.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <initializer_list>
#include <limits>
#include <numbers>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace Mustard::Testing {

TestEstimate::TestEstimate() :
    Subprogram{"TestEstimate", "Test Mustard::Math::Estimate."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;

// =========================================================================
// Helper: make an Eigen vector from an initializer list
// =========================================================================
template<int K>
auto MakeVector(std::initializer_list<double> init) -> auto {
    if constexpr (K == Eigen::Dynamic) {
        Eigen::VectorXd v(init.size());
        auto i{0};
        for (auto val : init) {
            v[i++] = val;
        }
        return v;
    } else {
        Eigen::Vector<double, K> v;
        v.setZero();
        auto i{0};
        for (auto val : init) {
            v[i++] = val;
        }
        return v;
    }
}

template<int K>
auto MakeMatrix(std::initializer_list<double> init, int rows, int cols) -> auto {
    if constexpr (K == Eigen::Dynamic) {
        Eigen::MatrixXd m{rows, cols};
        auto i{0};
        for (auto r{0}; r < rows; ++r) {
            for (auto c{0}; c < cols; ++c) {
                m(r, c) = std::data(init)[i++];
            }
        }
        return m;
    } else {
        Eigen::Matrix<double, K, K> m;
        auto i{0};
        for (auto r{0}; r < rows; ++r) {
            for (auto c{0}; c < cols; ++c) {
                m(r, c) = std::data(init)[i++];
            }
        }
        return m;
    }
}

// =========================================================================
// Unified factory functions for test data
// =========================================================================

// MakeTestValue<K, C>(dim) → always [1.0, 2.0, 3.0, ..., dim]
template<int K, CovarianceOption C>
auto MakeTestValue(int dim) -> Eigen::Vector<double, K> {
    if constexpr (K == Eigen::Dynamic) {
        Eigen::VectorXd v(dim);
        for (auto i{0}; i < dim; ++i) {
            v[i] = i + 1;
        }
        return v;
    } else {
        Eigen::Vector<double, K> v;
        for (auto i{0}; i < K; ++i) {
            v[i] = i + 1;
        }
        return v;
    }
}

// MakeTestCov<K, C>(dim) → tri-diagonal covariance, correct type per C
// Full:    primes [4,3,5,7,11,...] on diagonal, 1.0 on adjacent off-diagonals
// Diagonal: same primes, stored as DiagonalMatrix
template<int K, CovarianceOption C>
auto MakeTestCov(int dim) -> auto {
    const double diagVals[]{0.4, 0.3, 0.5, 0.7, 1.1, 1.3, 1.7, 1.9, 2.3, 2.9};
    if constexpr (K == Eigen::Dynamic) {
        Eigen::MatrixXd m(dim, dim);
        for (auto i{0}; i < dim; ++i) {
            for (auto j{0}; j < dim; ++j) {
                if (i == j) {
                    m(i, j) = diagVals[i];
                } else if (i == j - 1 or j == i - 1) {
                    m(i, j) = 0.1;
                } else {
                    m(i, j) = 0.0;
                }
            }
        }
        if constexpr (C == CovarianceOption::Full) {
            return m;
        } else {
            return Eigen::DiagonalMatrix<double, Eigen::Dynamic>{m.diagonal()};
        }
    } else {
        Eigen::Matrix<double, K, K> m;
        for (auto i{0}; i < K; ++i) {
            for (auto j{0}; j < K; ++j) {
                if (i == j and i < dim) {
                    m(i, j) = diagVals[i];
                } else if ((i == j - 1 or j == i - 1) and i < dim and j < dim) {
                    m(i, j) = 0.1;
                } else {
                    m(i, j) = 0.0;
                }
            }
        }
        if constexpr (C == CovarianceOption::Full) {
            return m;
        } else {
            return Eigen::DiagonalMatrix<double, K>{m.diagonal()};
        }
    }
}

// MakeEstimate<K, C>(x, cov) → construct Estimate<K, C>
// For K=1, uses the scalar constructor Estimate{value, variance}.
template<int K, CovarianceOption C>
auto MakeEstimate(const Eigen::Vector<double, K>& x, const auto& cov) -> Estimate<K, C> {
    if constexpr (K == 1) {
        return Estimate<K, C>{x(0), cov.diagonal()(0)};
    } else {
        return Estimate<K, C>{x, cov};
    }
}

// Evaluate an Eigen expression template into a concrete type.
// Only used when the expression captures temporaries (e.g. auto x = mat * scalar).
template<int K, CovarianceOption C, typename Expr>
auto EvalCov(const Expr& expr) -> auto {
    if constexpr (C == CovarianceOption::Full) {
        return Eigen::Matrix<double, K, K>{expr};
    } else {
        return Eigen::DiagonalMatrix<double, K>{expr};
    }
}

// =========================================================================
// Iteration helpers
// =========================================================================

// All static K dimensions to test
using AllStaticDims = std::integer_sequence<int, 1, 2, 3, 5, 10>;

// Run a test function over all static K>=2 with both Full and Diagonal
template<int... Ks, typename Func, typename... Args>
auto RunOverStaticDims(Func&& func, Args&&... args) -> void {
    (std::forward<Func>(func).template operator()<Ks, CovarianceOption::Full>(std::forward<Args>(args)...), ...);
    (std::forward<Func>(func).template operator()<Ks, CovarianceOption::Diagonal>(std::forward<Args>(args)...), ...);
}

// Run a test function over dynamic dimension with both Full and Diagonal
template<typename Func, typename... Args>
auto RunOverDynamic(Func&& func, Args&&... args) -> void {
    std::forward<Func>(func).template operator()<Eigen::Dynamic, CovarianceOption::Full>(std::forward<Args>(args)...);
    std::forward<Func>(func).template operator()<Eigen::Dynamic, CovarianceOption::Diagonal>(std::forward<Args>(args)...);
}

// Run a test function over all K>=2 (static + dynamic) with both Full and Diagonal
template<int... Ks, typename Func, typename... Args>
auto RunOverAllDims(Func&& func, Args&&... args) -> void {
    RunOverStaticDims<Ks...>(std::forward<Func>(func), std::forward<Args>(args)...);
    RunOverDynamic(std::forward<Func>(func), std::forward<Args>(args)...);
}

// Wrappers that accept std::integer_sequence (AllStaticDims) and deduce the int pack
template<typename Seq, typename Func, typename... Args>
auto RunOverStaticDims(Func&& func, Args&&... args) -> void {
    [&]<int... Ks>(std::integer_sequence<int, Ks...>) {
        RunOverStaticDims<Ks...>(std::forward<Func>(func), std::forward<Args>(args)...);
    }(Seq{});
}

template<typename Seq, typename Func, typename... Args>
auto RunOverAllDims(Func&& func, Args&&... args) -> void {
    [&]<int... Ks>(std::integer_sequence<int, Ks...>) {
        RunOverAllDims<Ks...>(std::forward<Func>(func), std::forward<Args>(args)...);
    }(Seq{});
}

// =========================================================================
// Checker functions
// =========================================================================

template<typename T, typename U>
auto CheckClose(const T& actual, const U& expected, const std::string& ctx) -> void {
    if constexpr (std::is_arithmetic_v<T>) {
        if (not muc::isclose(actual, expected)) {
            Throw<std::runtime_error>(fmt::format("\n"
                                                  "{}: expected {}, got {}",
                                                  ctx, expected, actual));
        }
    } else {
        auto isCloseMat{[](const auto& a, const auto& b) {
            for (auto i{0}; i < a.rows(); ++i) {
                for (auto j{0}; j < a.cols(); ++j) {
                    if (not muc::isclose(a.coeff(i, j), b.coeff(i, j))) {
                        return false;
                    }
                }
            }
            return true;
        }};
        if (not isCloseMat(actual, expected)) {
            Throw<std::runtime_error>(fmt::format("\n"
                                                  "{}: expected approx\n"
                                                  "{},\n"
                                                  "got\n"
                                                  "{}",
                                                  ctx, fmt::streamed(expected), fmt::streamed(actual)));
        }
    }
}

auto CheckEq(auto actual, auto expected, const std::string& ctx) -> void {
    if (actual != expected) {
        Throw<std::runtime_error>(fmt::format("\n"
                                              "{}: expected {}, got {}",
                                              ctx, expected, actual));
    }
}

// =========================================================================
// Section 0: Compilation Smoke Test — homogeneous single-type API
// =========================================================================

constexpr auto sec0Smoke{[]<int K, CovarianceOption C>() {
    using Est = Estimate<K, C>;
    constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr bool isFull{C == CovarianceOption::Full};
    constexpr CovarianceOption oppositeC{isFull ? CovarianceOption::Diagonal : CovarianceOption::Full};
    using OppositeEst = Estimate<K, oppositeC>;

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    const auto xOpp{MakeTestValue<K, oppositeC>(dim)};
    const auto covOpp{MakeTestCov<K, oppositeC>(dim)};

    // Three reusable result variables: scalar, vector, matrix
    [[maybe_unused]] double scalar{};
    typename Est::ValueType vec{};
    typename Est::CovarianceType mat{};

    // Fresh-Estimate helpers (K=1 uses scalar ctor; K!=1 uses Eigen ctor)
    const auto newEst{[&] {
        if constexpr (K == 1) {
            return Est{1.0, 0.5};
        } else {
            return Est{x, cov};
        }
    }};
    const auto newOpp{[&] {
        if constexpr (K == 1) {
            return OppositeEst{0.5, 0.25};
        } else {
            return OppositeEst{xOpp, covOpp};
        }
    }};

    // ---- constructors ----
    {
        if constexpr (not isDynamic) {
            Est e0;
            vec = e0.Value();
            typename Est::PODType pod{};
            Est e5{pod};
            vec = e5.Value();
        }
        if constexpr (isDynamic) {
            Est e0d{dim};
            vec = e0d.Value();
        }
        Est e1 = newEst();
        if constexpr (K == 1) {
            Estimate<1, C> e1s{1.0, 0.5};
            scalar = e1s.Value();
        }
        Est e2{std::move(e1)};
        Est e3{e2};
        OppositeEst eOpp = newOpp();
        Est e4{eOpp};
    }

    Est e3 = newEst();

    // ---- per-component accessors (K!=1 only; hidden by no-arg overloads for K=1) ----
    if constexpr (K != 1) {
        scalar = e3.Dimension();
        scalar = e3.Value(0);
        scalar = e3.Covariance(0, 0);
        scalar = e3.Variance(0);
        scalar = e3.StdDev(0);
        scalar = e3.Uncertainty(0);
        scalar = e3.RelativeUncertainty(0);
        scalar = e3.Correlation(0, 0);
        if constexpr (dim >= 2) {
            scalar = e3.Correlation(0, 1);
        }
    }

    // ---- full vector/matrix accessors ----
    {
        scalar = e3.Dimension();
        vec = e3.Value();
        if constexpr (K != 1) {
            mat = e3.Covariance();
            mat = e3.Correlation();
        }
        vec = e3.Variance();
        vec = e3.StdDev();
        vec = e3.Uncertainty();
        vec = e3.RelativeUncertainty();
    }

    // ---- assignment operators ----
    auto est{newEst()};
    {
        est = e3;
        {
            auto e3m{newEst()};
            est = std::move(e3m);
        }
        auto& estRef{est};
        est = estRef;
        est = std::move(estRef);
        OppositeEst eOpp2{newOpp()};
        est = eOpp2;
        est = newEst();
    }

    // ---- compound assignment: +=, -=, *=, /= ----
    {
        est += e3;
        if constexpr (K != 1) {
            est += x;
        }
        est += 1.0;

        est -= e3;
        if constexpr (K != 1) {
            est -= x;
        }
        est -= 1.0;

        est *= e3;
        if constexpr (K != 1) {
            est *= x;
        }
        est *= 2.0;

        est /= e3;
        if constexpr (K != 1) {
            est /= x;
        }
        est /= 2.0;
    }

    // ---- Dot product (K!=1 only) ----
    if constexpr (K != 1) {
        scalar = e3.Dot(est).Value();
        scalar = e3.Dot(x).Value();
        scalar = Dot(e3, est).Value();
        scalar = Dot(e3, x).Value();
        scalar = Dot(x, e3).Value();

        // ---- matrix-vector multiply (K!=1 only) ----
        if constexpr (isDynamic) {
            Eigen::MatrixXd matA{dim, dim};
            matA.setIdentity();
            Eigen::MatrixXd matRect{dim + 1, dim};
            matRect.setOnes();
            auto resultMatA{e3.RightMultiply(matA)};
            e3 = newEst();
            resultMatA = e3 * matA;
            e3 = newEst();
            auto resultMatRect{matRect * e3};
            resultMatRect = e3.LeftMultiply(matRect);
        } else {
            Eigen::Matrix<double, K, K> matA;
            matA.setIdentity();
            Eigen::Matrix<double, K + 1, K> matRect;
            matRect.setOnes();
            auto resultMatA{e3.RightMultiply(matA)};
            e3 = newEst();
            resultMatA = e3 * matA;
            e3 = newEst();
            auto resultMatRect{matRect * e3};
            resultMatRect = e3.LeftMultiply(matRect);
        }

        // ---- reduction operations (K!=1 only) ----
        scalar = est.Sum().Value();
        scalar = est.Mean().Value();
        est = newEst();
        scalar = est.Prod().Value();
        est = newEst();
        scalar = est.SquaredNorm().Value();
        est = newEst();
        scalar = est.Norm().Value();
        est = newEst();
        scalar = est.HarmonicMean().Value();
        est = newEst();
        scalar = est.GeometricMean().Value();
        est = newEst();
        scalar = est.QuadraticMean().Value();
        est = newEst();
        scalar = est.CubicMean().Value();
        est = newEst();
        scalar = est.template LpNorm<1.0>().Value();
        est = newEst();
        scalar = est.template LpNorm<2.0>().Value();
        est = newEst();
        scalar = est.template LpNorm<3.0>().Value();
        est = newEst();
        scalar = est.template LpNorm<4.0>().Value();
        est = newEst();
        scalar = est.template LpNorm<42.0>().Value();
    }

    // ---- in-place unary math functions ----
    {
        est = newEst();
        est = std::move(est).Negate();
        est = newEst();
        est = std::move(est).Square();
        est = newEst();
        est = std::move(est).Cube();
        est = newEst();
        est = std::move(est).Sqrt();
        est = newEst();
        est = std::move(est).Cbrt();
        est = newEst();
        est = std::move(est).Rsqrt();
        est = newEst();
        est = std::move(est).Inverse();
        est = newEst();
        est = std::move(est).Abs();
        est = newEst();
        est = std::move(est).Exp();
        est = newEst();
        est = std::move(est).Exp2();
        est = newEst();
        est = std::move(est).Expm1();
        est = newEst();
        est = std::move(est).Log();
        est = newEst();
        est = std::move(est).Log10();
        est = newEst();
        est = std::move(est).Log2();
        est = newEst();
        est = std::move(est).Log1p();
        est = newEst();
        est = std::move(est).Sin();
        est = newEst();
        est = std::move(est).Cos();
        est = newEst();
        est = std::move(est).Tan();
        est = newEst();
        est = std::move(est).Asin();
        est = newEst();
        est = std::move(est).Acos();
        est = newEst();
        est = std::move(est).Atan();
        est = newEst();
        est = std::move(est).Sinh();
        est = newEst();
        est = std::move(est).Cosh();
        est = newEst();
        est = std::move(est).Tanh();
        est = newEst();
        est = std::move(est).Asinh();
        est = newEst();
        est = std::move(est).Acosh();
        est = newEst();
        est = std::move(est).Atanh();
        est = newEst();
        est = std::move(est).Logistic();
        est = newEst();
        est = std::move(est).Erf();
        est = newEst();
        est = std::move(est).Erfc();
        est = newEst();
        est = std::move(est).Lgamma();
        est = newEst();
        est = std::move(est).Ndtri();
    }

    // ---- in-place binary element-wise transformations ----
    {
        est = newEst();
        est = std::move(est).NegateAdd(e3);
        if constexpr (K != 1) {
            est = std::move(est).NegateAdd(x);
        }
        est = std::move(est).NegateAdd(3.0);

        est = newEst();
        est = std::move(est).Divide(e3);
        if constexpr (K != 1) {
            est = std::move(est).Divide(x);
        }
        est = std::move(est).Divide(4.0);

        est = newEst();
        est = std::move(est).Pow(e3);
        if constexpr (K != 1) {
            est = std::move(est).Pow(x);
        }
        est = std::move(est).Pow(2.0);

        est = newEst();
        est = std::move(est).Exp(e3);
        if constexpr (K != 1) {
            est = std::move(est).Exp(x);
        }
        est = std::move(est).Exp(2.0);
    }

    // ---- free unary operators ----
    {
        vec = operator+(est).Value();
        vec = operator+(std::as_const(est)).Value();
        vec = operator+(newEst()).Value();
        vec = operator+(static_cast<const Est&&>(newEst())).Value();
        vec = operator-(newEst()).Value();
    }

    // ---- free binary operators ----
    {
        // operator+
        vec = (est + e3).Value();
        vec = (newEst() + e3).Value();
        vec = (est + newEst()).Value();
        vec = (newEst() + newEst()).Value();
        if constexpr (K != 1) {
            vec = (est + x).Value();
            vec = (x + est).Value();
        }
        vec = (est + 1.0).Value();
        vec = (1.0 + est).Value();

        // operator-
        vec = (est - e3).Value();
        vec = (newEst() - e3).Value();
        vec = (est - newEst()).Value();
        vec = (newEst() - newEst()).Value();
        if constexpr (K != 1) {
            vec = (est - x).Value();
            vec = (x - est).Value();
        }
        vec = (est - 1.0).Value();
        vec = (1.0 - est).Value();

        // operator*
        vec = (est * e3).Value();
        vec = (newEst() * e3).Value();
        vec = (est * newEst()).Value();
        vec = (newEst() * newEst()).Value();
        if constexpr (K != 1) {
            vec = (est * x).Value();
            vec = (x * est).Value();
        }
        vec = (est * 2.0).Value();
        vec = (2.0 * est).Value();

        // operator/
        vec = (est / e3).Value();
        vec = (newEst() / e3).Value();
        vec = (est / newEst()).Value();
        vec = (newEst() / newEst()).Value();
        if constexpr (K != 1) {
            vec = (est / x).Value();
            vec = (x / est).Value();
        }
        vec = (est / 2.0).Value();
        vec = (2.0 / est).Value();

        // pow
        vec = pow(est, e3).Value();
        vec = pow(newEst(), e3).Value();
        vec = pow(est, newEst()).Value();
        vec = pow(newEst(), newEst()).Value();
        if constexpr (K != 1) {
            vec = pow(est, x).Value();
            vec = pow(x, est).Value();
        }
        vec = pow(est, 2.0).Value();
        vec = pow(2.0, est).Value();
    }

    // ---- free element-wise math functions ----
    {
        est = newEst();
        vec = square(est).Value();
        vec = cube(est).Value();
        vec = sqrt(est).Value();
        vec = cbrt(est).Value();
        vec = rsqrt(est).Value();
        vec = inverse(est).Value();
        vec = abs(est).Value();
        vec = exp(est).Value();
        vec = exp2(est).Value();
        vec = expm1(est).Value();
        vec = log(est).Value();
        vec = log10(est).Value();
        vec = log2(est).Value();
        vec = log1p(est).Value();
        vec = sin(est).Value();
        vec = cos(est).Value();
        vec = tan(est).Value();
        vec = asin(est).Value();
        vec = acos(est).Value();
        vec = atan(est).Value();
        vec = sinh(est).Value();
        vec = cosh(est).Value();
        vec = tanh(est).Value();
        vec = asinh(est).Value();
        vec = acosh(est).Value();
        vec = atanh(est).Value();
        vec = logistic(est).Value();
        vec = erf(est).Value();
        vec = erfc(est).Value();
        vec = lgamma(est).Value();
        vec = ndtri(est).Value();
    }

    // ---- From/ToPOD (static K only) ----
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
// Section 0c: Compilation Smoke Test — cross-type operations
//
// Allowed cross-type combinations:
//   (a) Same K, different CovarianceOption  (Full ↔ Diagonal)
//   (b) Static ↔ Dynamic  (same actual dimension)
//   (c) Dynamic ↔ Dynamic  (same actual dimension)
//   (d) (a)+(b) combined: Static Full ↔ Dynamic Diagonal / Static Diagonal ↔ Dynamic Full
//
// Forbidden: static×static with different K
//
// For each allowed combination we smoke-test:
//   - cross construction (Estimate<K1,C1> from Estimate<K2,C2>)
//   - cross assignment (e1 = e2)
//   - cross binary operators: +, -, *, /, pow, Dot (both argument orders)
// =========================================================================

auto sec0SmokeCrossType() -> void {
    [[maybe_unused]] double scalar{};
    constexpr auto full{CovarianceOption::Full};
    constexpr auto diag{CovarianceOption::Diagonal};
    constexpr int dyn{Eigen::Dynamic};

    // =========================================================================
    // Subsection A: Same K, different CovarianceOption (K=3, Full ↔ Diagonal)
    // =========================================================================
    {
        const auto x3{MakeTestValue<3, full>(3)};
        const auto cov3f{MakeTestCov<3, full>(3)};
        const auto cov3d{MakeTestCov<3, diag>(3)};
        Estimate<3, full> ef3{x3, cov3f};
        Estimate<3, diag> ed3{x3, cov3d};

        // cross construction
        [[maybe_unused]] Estimate<3, full> efFromDiag{ed3};
        [[maybe_unused]] Estimate<3, diag> edFromFull{ef3};

        // cross assignment
        Estimate<3, full> efAssign{x3, cov3f};
        efAssign = ed3;
        scalar = efAssign.Value(0);
        Estimate<3, diag> edAssign{x3, cov3d};
        edAssign = ef3;
        scalar = edAssign.Value(0);

        // cross binary operators — Full op Diag (and swapped)
        scalar = (ef3 + ed3).Value(0);
        scalar = (ed3 + ef3).Value(0);
        scalar = (ef3 - ed3).Value(0);
        scalar = (ed3 - ef3).Value(0);
        scalar = (ef3 * ed3).Value(0);
        scalar = (ed3 * ef3).Value(0);
        scalar = (ef3 / ed3).Value(0);
        scalar = (ed3 / ef3).Value(0);
        scalar = pow(ef3, ed3).Value(0);
        scalar = pow(ed3, ef3).Value(0);
        scalar = ef3.Dot(ed3).Value();
        scalar = Dot(ef3, ed3).Value();

        // matrix-vector multiply cross-type
        {
            Eigen::Matrix3d matA;
            matA.setIdentity();
            Eigen::Matrix<double, 4, 3> matRect;
            matRect.setOnes();
            // RightMultiply
            auto matResultF{ef3.RightMultiply(matA)};
            auto matResultD{ed3.RightMultiply(matA)};
            matResultF = ef3 * matA;
            matResultD = ed3 * matA;
            // LeftMultiply
            auto matResultRectF{matRect * ef3};
            auto matResultRectD{matRect * ed3};
            matResultRectF = ef3.LeftMultiply(matRect);
            matResultRectD = ed3.LeftMultiply(matRect);
        }

        // compound assignment cross-type
        {
            auto efTmp{ef3};
            efTmp += ed3;
            scalar = efTmp.Value(0);
            efTmp -= ed3;
            scalar = efTmp.Value(0);
            efTmp *= ed3;
            scalar = efTmp.Value(0);
            efTmp /= ed3;
            scalar = efTmp.Value(0);
        }
        {
            auto edTmp{ed3};
            edTmp += ef3;
            scalar = edTmp.Value(0);
            edTmp -= ef3;
            scalar = edTmp.Value(0);
            edTmp *= ef3;
            scalar = edTmp.Value(0);
            edTmp /= ef3;
            scalar = edTmp.Value(0);
        }
    }

    // =========================================================================
    // Subsection B: Static ↔ Dynamic, same dimension, same CovarianceOption
    //   K=3 Static Full ↔ Dynamic Full
    //   K=3 Static Diag ↔ Dynamic Diag
    // =========================================================================
    {
        const auto x3{MakeTestValue<3, full>(3)};
        const auto cov3f{MakeTestCov<3, full>(3)};
        const auto cov3d{MakeTestCov<3, diag>(3)};
        const auto covDynF3{MakeTestCov<dyn, full>(3)};
        const auto covDynD3{MakeTestCov<dyn, diag>(3)};
        const auto xDyn3{MakeTestValue<dyn, full>(3)};

        // --- Static Full ↔ Dynamic Full ---
        Estimate<3, full> e3f{x3, cov3f};
        Estimate<dyn, full> edf3{xDyn3, covDynF3};

        // cross construction
        [[maybe_unused]] Estimate<3, full> e3fFromDyn{edf3};
        [[maybe_unused]] Estimate<dyn, full> edfFromStat{e3f};

        // cross assignment
        Estimate<dyn, full> edfAssign{MakeTestValue<dyn, full>(3), MakeTestCov<dyn, full>(3)};
        edfAssign = e3f;
        scalar = edfAssign.Value(0);
        Estimate<3, full> e3fAssign{x3, cov3f};
        e3fAssign = edf3;
        scalar = e3fAssign.Value(0);

        // cross binary operators
        scalar = (e3f + edf3).Value(0);
        scalar = (edf3 + e3f).Value(0);
        scalar = (e3f - edf3).Value(0);
        scalar = (edf3 - e3f).Value(0);
        scalar = (e3f * edf3).Value(0);
        scalar = (edf3 * e3f).Value(0);
        scalar = (e3f / edf3).Value(0);
        scalar = (edf3 / e3f).Value(0);
        scalar = pow(e3f, edf3).Value(0);
        scalar = pow(edf3, e3f).Value(0);
        scalar = e3f.Dot(edf3).Value();
        scalar = Dot(e3f, edf3).Value();

        // compound assignment
        {
            auto eTmp{e3f};
            eTmp += edf3;
            scalar = eTmp.Value(0);
            eTmp -= edf3;
            scalar = eTmp.Value(0);
            eTmp *= edf3;
            scalar = eTmp.Value(0);
            eTmp /= edf3;
            scalar = eTmp.Value(0);
        }
        {
            auto edTmp{edf3};
            edTmp += e3f;
            scalar = edTmp.Value(0);
            edTmp -= e3f;
            scalar = edTmp.Value(0);
            edTmp *= e3f;
            scalar = edTmp.Value(0);
            edTmp /= e3f;
            scalar = edTmp.Value(0);
        }

        // --- Static Diag ↔ Dynamic Diag ---
        Estimate<3, diag> e3d{MakeTestValue<3, diag>(3), cov3d};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};

        // cross construction
        [[maybe_unused]] Estimate<3, diag> e3dFromDyn{edd3};
        [[maybe_unused]] Estimate<dyn, diag> eddFromStat{e3d};

        // cross assignment
        Estimate<dyn, diag> eddAssign{MakeTestValue<dyn, diag>(3), MakeTestCov<dyn, diag>(3)};
        eddAssign = e3d;
        scalar = eddAssign.Variance(0);
        Estimate<3, diag> e3dAssign{MakeTestValue<3, diag>(3), cov3d};
        e3dAssign = edd3;
        scalar = e3dAssign.Variance(0);

        // cross binary operators
        scalar = (e3d + edd3).Value(0);
        scalar = (edd3 + e3d).Value(0);
        scalar = (e3d - edd3).Value(0);
        scalar = (edd3 - e3d).Value(0);
        scalar = (e3d * edd3).Value(0);
        scalar = (edd3 * e3d).Value(0);
        scalar = (e3d / edd3).Value(0);
        scalar = (edd3 / e3d).Value(0);
        scalar = pow(e3d, edd3).Value(0);
        scalar = pow(edd3, e3d).Value(0);
        scalar = e3d.Dot(edd3).Value();
        scalar = Dot(e3d, edd3).Value();

        // compound assignment
        {
            auto eTmp{e3d};
            eTmp += edd3;
            scalar = eTmp.Variance(0);
            eTmp -= edd3;
            scalar = eTmp.Variance(0);
            eTmp *= edd3;
            scalar = eTmp.Variance(0);
            eTmp /= edd3;
            scalar = eTmp.Variance(0);
        }
        {
            auto edTmp{edd3};
            edTmp += e3d;
            scalar = edTmp.Variance(0);
            edTmp -= e3d;
            scalar = edTmp.Variance(0);
            edTmp *= e3d;
            scalar = edTmp.Variance(0);
            edTmp /= e3d;
            scalar = edTmp.Variance(0);
        }
    }

    // =========================================================================
    // Subsection C: Dynamic ↔ Dynamic, same dimension
    //   Dyn Full ↔ Dyn Diag (dim=3)
    //   Dyn Full ↔ Dyn Full (dim=3, same C but different dim=4 is forbidden)
    // =========================================================================
    {
        const auto xDyn3{MakeTestValue<dyn, full>(3)};
        const auto covDynF3{MakeTestCov<dyn, full>(3)};
        const auto covDynD3{MakeTestCov<dyn, diag>(3)};

        // --- Dyn Full ↔ Dyn Diag ---
        Estimate<dyn, full> edf3{xDyn3, covDynF3};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};

        // cross construction
        [[maybe_unused]] Estimate<dyn, full> edfFromDiag{edd3};
        [[maybe_unused]] Estimate<dyn, diag> eddFromFull{edf3};

        // cross assignment
        Estimate<dyn, full> edfAssign{MakeTestValue<dyn, full>(3), MakeTestCov<dyn, full>(3)};
        edfAssign = edd3;
        scalar = edfAssign.Value(0);
        Estimate<dyn, diag> eddAssign{MakeTestValue<dyn, diag>(3), MakeTestCov<dyn, diag>(3)};
        eddAssign = edf3;
        scalar = eddAssign.Variance(0);

        // cross binary operators
        scalar = (edf3 + edd3).Value(0);
        scalar = (edd3 + edf3).Value(0);
        scalar = (edf3 - edd3).Value(0);
        scalar = (edd3 - edf3).Value(0);
        scalar = (edf3 * edd3).Value(0);
        scalar = (edd3 * edf3).Value(0);
        scalar = (edf3 / edd3).Value(0);
        scalar = (edd3 / edf3).Value(0);
        scalar = pow(edf3, edd3).Value(0);
        scalar = pow(edd3, edf3).Value(0);
        scalar = edf3.Dot(edd3).Value();
        scalar = Dot(edf3, edd3).Value();

        // compound assignment
        {
            auto eTmp{edf3};
            eTmp += edd3;
            scalar = eTmp.Value(0);
            eTmp -= edd3;
            scalar = eTmp.Value(0);
            eTmp *= edd3;
            scalar = eTmp.Value(0);
            eTmp /= edd3;
            scalar = eTmp.Value(0);
        }
        {
            auto edTmp{edd3};
            edTmp += edf3;
            scalar = edTmp.Variance(0);
            edTmp -= edf3;
            scalar = edTmp.Variance(0);
            edTmp *= edf3;
            scalar = edTmp.Variance(0);
            edTmp /= edf3;
            scalar = edTmp.Variance(0);
        }
    }

    // =========================================================================
    // Subsection D: Static ↔ Dynamic, different CovarianceOption
    //   K=3 Static Full ↔ Dynamic Diag
    //   K=3 Static Diag ↔ Dynamic Full
    // =========================================================================
    {
        const auto x3{MakeTestValue<3, full>(3)};
        const auto cov3f{MakeTestCov<3, full>(3)};
        const auto cov3d{MakeTestCov<3, diag>(3)};
        const auto xDyn3{MakeTestValue<dyn, full>(3)};
        const auto covDynF3{MakeTestCov<dyn, full>(3)};
        const auto covDynD3{MakeTestCov<dyn, diag>(3)};

        // --- Static Full ↔ Dynamic Diag ---
        Estimate<3, full> e3f{x3, cov3f};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};

        // cross construction
        [[maybe_unused]] Estimate<3, full> e3fFromDD{edd3};
        [[maybe_unused]] Estimate<dyn, diag> eddFromSF{e3f};

        // cross assignment
        Estimate<dyn, diag> eddAssign{MakeTestValue<dyn, diag>(3), MakeTestCov<dyn, diag>(3)};
        eddAssign = e3f;
        scalar = eddAssign.Variance(0);
        Estimate<3, full> e3fAssign{x3, cov3f};
        e3fAssign = edd3;
        scalar = e3fAssign.Value(0);

        // cross binary operators
        scalar = (e3f + edd3).Value(0);
        scalar = (edd3 + e3f).Value(0);
        scalar = (e3f - edd3).Value(0);
        scalar = (edd3 - e3f).Value(0);
        scalar = (e3f * edd3).Value(0);
        scalar = (edd3 * e3f).Value(0);
        scalar = (e3f / edd3).Value(0);
        scalar = (edd3 / e3f).Value(0);
        scalar = pow(e3f, edd3).Value(0);
        scalar = pow(edd3, e3f).Value(0);
        scalar = e3f.Dot(edd3).Value();
        scalar = Dot(e3f, edd3).Value();

        // compound assignment
        {
            auto eTmp{e3f};
            eTmp += edd3;
            scalar = eTmp.Value(0);
            eTmp -= edd3;
            scalar = eTmp.Value(0);
            eTmp *= edd3;
            scalar = eTmp.Value(0);
            eTmp /= edd3;
            scalar = eTmp.Value(0);
        }
        {
            auto edTmp{edd3};
            edTmp += e3f;
            scalar = edTmp.Variance(0);
            edTmp -= e3f;
            scalar = edTmp.Variance(0);
            edTmp *= e3f;
            scalar = edTmp.Variance(0);
            edTmp /= e3f;
            scalar = edTmp.Variance(0);
        }

        // --- Static Diag ↔ Dynamic Full ---
        Estimate<3, diag> e3d{MakeTestValue<3, diag>(3), cov3d};
        Estimate<dyn, full> edf3{xDyn3, covDynF3};

        // cross construction
        [[maybe_unused]] Estimate<3, diag> e3dFromDF{edf3};
        [[maybe_unused]] Estimate<dyn, full> edfFromSD{e3d};

        // cross assignment
        Estimate<dyn, full> edfAssign{MakeTestValue<dyn, full>(3), MakeTestCov<dyn, full>(3)};
        edfAssign = e3d;
        scalar = edfAssign.Value(0);
        Estimate<3, diag> e3dAssign{MakeTestValue<3, diag>(3), cov3d};
        e3dAssign = edf3;
        scalar = e3dAssign.Variance(0);

        // cross binary operators
        scalar = (e3d + edf3).Value(0);
        scalar = (edf3 + e3d).Value(0);
        scalar = (e3d - edf3).Value(0);
        scalar = (edf3 - e3d).Value(0);
        scalar = (e3d * edf3).Value(0);
        scalar = (edf3 * e3d).Value(0);
        scalar = (e3d / edf3).Value(0);
        scalar = (edf3 / e3d).Value(0);
        scalar = pow(e3d, edf3).Value(0);
        scalar = pow(edf3, e3d).Value(0);
        scalar = e3d.Dot(edf3).Value();
        scalar = Dot(e3d, edf3).Value();

        // compound assignment
        {
            auto eTmp{e3d};
            eTmp += edf3;
            scalar = eTmp.Variance(0);
            eTmp -= edf3;
            scalar = eTmp.Variance(0);
            eTmp *= edf3;
            scalar = eTmp.Variance(0);
            eTmp /= edf3;
            scalar = eTmp.Variance(0);
        }
        {
            auto edTmp{edf3};
            edTmp += e3d;
            scalar = edTmp.Value(0);
            edTmp -= e3d;
            scalar = edTmp.Value(0);
            edTmp *= e3d;
            scalar = edTmp.Value(0);
            edTmp /= e3d;
            scalar = edTmp.Value(0);
        }
    }
};

// =========================================================================
// Section 1: Construction and Basic Accessors
// =========================================================================
constexpr auto sec1Construction{[]<int K, CovarianceOption C>() {
    using Est = Estimate<K, C>;
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr bool isFull{C == CovarianceOption::Full};

    // Default construction (static only)
    if constexpr (not isDynamic) {
        Est e;
        CheckEq(e.Dimension(), K, "1: Dimension");
        if constexpr (K == 1) {
            CheckClose(e.Value(), 0.0, "1: Value()");
            CheckClose(e.Variance(), 0.0, "1: Variance()");
            CheckClose(e.StdDev(), 0.0, "1: StdDev()");
            CheckClose(e.Uncertainty(), 0.0, "1: Uncertainty()");
        } else {
            for (auto i{0}; i < K; ++i) {
                CheckClose(e.Value(i), 0.0, fmt::format("1: Value({})", i));
                CheckClose(e.Variance(i), 0.0, fmt::format("1: Variance({})", i));
                CheckClose(e.StdDev(i), 0.0, fmt::format("1: StdDev({})", i));
                CheckClose(e.Uncertainty(i), 0.0, fmt::format("1: Uncertainty({})", i));
            }
        }
    } else {
        {
            Est e{2};
            CheckEq(e.Dimension(), 2, "1: Dimension");
            CheckClose(e.Value(0), 0.0, "1: Value(0)");
            CheckClose(e.Value(1), 0.0, "1: Value(1)");
        }
        {
            auto threwZero{false};
            try {
                Est e{0};
            } catch (const std::invalid_argument&) {
                threwZero = true;
            }
            if (not threwZero) {
                Throw<std::runtime_error>("1: dim=0 should throw");
            }

            auto threwNeg{false};
            try {
                Est e{-1};
            } catch (const std::invalid_argument&) {
                threwNeg = true;
            }
            if (not threwNeg) {
                Throw<std::runtime_error>("1: dim=-1 should throw");
            }
        }
    }

    // Value+cov constructor
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};

        if constexpr (K == 1) {
            CheckClose(e.Value(), 1.0, "1: Value()");
            CheckClose(e.Variance(), 0.4, "1: Variance()");
            CheckClose(e.StdDev(), std::sqrt(0.4), "1: StdDev()");
            CheckClose(e.Uncertainty(), std::sqrt(0.4), "1: Uncertainty()");
            CheckClose(e.RelativeUncertainty(), std::sqrt(0.4), "1: RelUnc()");
        } else {
            CheckClose(e.Value(0), 1.0, "1: Value(0)");
            if constexpr (dim >= 2) {
                CheckClose(e.Value(1), 2.0, "1: Value(1)");
            }
            if constexpr (isFull) {
                CheckClose(e.Covariance(0, 0), 0.4, "1: Cov(0,0)");
                if constexpr (dim >= 2) {
                    CheckClose(e.Covariance(1, 1), 0.3, "1: Cov(1,1)");
                    CheckClose(e.Covariance(0, 1), 0.1, "1: Cov(0,1)");
                    CheckClose(e.Covariance(1, 0), 0.1, "1: Cov(1,0)");
                }
            }
            CheckClose(e.Variance(0), 0.4, "1: Variance(0)");
            if constexpr (dim >= 2) {
                CheckClose(e.Variance(1), 0.3, "1: Variance(1)");
                CheckClose(e.StdDev(1), std::sqrt(0.3), "1: StdDev(1)");
                CheckClose(e.RelativeUncertainty(1), std::sqrt(0.3) / 2.0, "1: RelUnc(1)");
            }
            CheckClose(e.StdDev(0), std::sqrt(0.4), "1: StdDev(0)");
            CheckClose(e.Uncertainty(0), std::sqrt(0.4), "1: Uncertainty(0)");
            CheckClose(e.RelativeUncertainty(0), std::sqrt(0.4), "1: RelUnc(0)");
        }

        if constexpr (K == 1) {
            CheckClose(e.Value(), x(0), "1: Value()");
            CheckClose(e.Variance(), cov.diagonal()(0), "1: Variance()");
            CheckClose(e.StdDev(), std::sqrt(cov.diagonal()(0)), "1: StdDev()");
            CheckClose(e.Uncertainty(), std::sqrt(cov.diagonal()(0)), "1: Uncertainty()");
        } else {
            CheckClose(e.Value(), x, "1: Value()");
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Variance()(i), cov.diagonal()(i), fmt::format("1: Variance()({})", i));
                CheckClose(e.StdDev()(i), std::sqrt(cov.diagonal()(i)), fmt::format("1: StdDev()({})", i));
                CheckClose(e.Uncertainty()(i), std::sqrt(cov.diagonal()(i)), fmt::format("1: Uncertainty()({})", i));
            }
        }
        if constexpr (isFull and K != 1) {
            CheckClose(e.Covariance(), cov, "1: Covariance()");
        }

        // Correlation checks
        if constexpr (isFull and K != 1) {
            CheckClose(e.Correlation(0, 0), 1.0, "1: Corr(0,0)=1");
            if constexpr (dim >= 2) {
                CheckClose(e.Correlation(1, 1), 1.0, "1: Corr(1,1)=1");
                CheckClose(e.Correlation(0, 1), 0.1 / std::sqrt(0.12), "1: Corr(0,1)");
                CheckClose(e.Correlation(1, 0), 0.1 / std::sqrt(0.12), "1: Corr(1,0) sym");
            }
            const auto corrMat{e.Correlation()};
            CheckClose(corrMat(0, 0), 1.0, "1: Correlation()(0,0)");
            if constexpr (dim >= 2) {
                CheckClose(corrMat(0, 1), 0.1 / std::sqrt(0.12), "1: Correlation()(0,1)");
            }
        }
        if constexpr (not isFull and K != 1 and dim >= 2) {
            CheckClose(e.Correlation(0, 0), 1.0, "1: Corr(0,0)=1 diag");
            CheckClose(e.Correlation(0, 1), 0.0, "1: Corr(0,1)=0 diag");
        }

        Est e2{e};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), e.Value(), "1: Copy Value()");
            CheckClose(e2.Variance(), e.Variance(), "1: Copy Variance()");
        } else {
            CheckClose(e2.Value(0), e.Value(0), "1: Copy Value(0)");
            if constexpr (isFull and dim >= 2) {
                CheckClose(e2.Covariance(0, 1), e.Covariance(0, 1), "1: Copy Cov(0,1)");
            }
        }

        Est e3{std::move(e2)};
        if constexpr (K == 1) {
            CheckClose(e3.Value(), 1.0, "1: Move Value()");
            CheckClose(e3.Variance(), 0.4, "1: Move Variance()");
        } else {
            CheckClose(e3.Value(0), 1.0, "1: Move Value(0)");
            if constexpr (isFull and dim >= 2) {
                CheckClose(e3.Covariance(0, 1), 0.1, "1: Move Cov(0,1)");
            }
        }
    }

    // Cross-CovarianceOption copy
    if constexpr (isFull) {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, CovarianceOption::Full>(dim)};
        auto eFull{MakeEstimate<K, CovarianceOption::Full>(x, cov)};
        Estimate<K, CovarianceOption::Diagonal> eDiag{eFull};
        if constexpr (K == 1) {
            CheckClose(eDiag.Value(), 1.0, "1: DiagFromFull Value()");
            CheckClose(eDiag.Variance(), 0.4, "1: DiagFromFull Variance()");
        } else {
            CheckClose(eDiag.Value(0), 1.0, "1: DiagFromFull Value(0)");
            CheckClose(eDiag.Variance(0), 0.4, "1: DiagFromFull Variance(0)");
            if constexpr (dim >= 2) {
                CheckClose(eDiag.Variance(1), 0.3, "1: DiagFromFull Variance(1)");
                CheckClose(eDiag.Covariance(0, 1), 0.0, "1: DiagFromFull Cov(0,1)=0");
            }
        }
    } else {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto diagCov{MakeTestCov<K, CovarianceOption::Diagonal>(dim)};
        auto eDiag{MakeEstimate<K, CovarianceOption::Diagonal>(x, diagCov)};
        Estimate<K, CovarianceOption::Full> eFull{eDiag};
        if constexpr (K == 1) {
            CheckClose(eFull.Value(), 1.0, "1: FullFromDiag Value()");
            CheckClose(eFull.Variance(), 0.4, "1: FullFromDiag Variance()");
        } else {
            CheckClose(eFull.Value(0), 1.0, "1: FullFromDiag Value(0)");
            CheckClose(eFull.Variance(0), 0.4, "1: FullFromDiag Variance(0)");
            if constexpr (dim >= 2) {
                CheckClose(eFull.Covariance(0, 1), 0.0, "1: FullFromDiag Cov(0,1)=0");
            }
        }
    }

    // Dimension mismatch throws (dynamic only)
    if constexpr (isDynamic) {
        Est e1{2};
        auto threw{false};
        try {
            Est eOther{3};
            e1 += eOther;
        } catch (const std::invalid_argument&) { threw = true; }
        if (not threw) {
            Throw<std::runtime_error>("1: dimension mismatch should throw");
        }
    }
}};

// =========================================================================
// Section 1: Cross-Dimension Copy (Static <-> Dynamic)
// =========================================================================
constexpr auto sec1CrossDimCopy{[] {
    // Static from Dynamic
    Eigen::VectorXd xDyn{2};
    xDyn << 5.0, 6.0;
    Eigen::MatrixXd covDyn{2, 2};
    covDyn << 0.9, 0.2, 0.2, 0.4;
    Estimate<Eigen::Dynamic, CovarianceOption::Full> eDyn{xDyn, covDyn};
    Estimate<2, CovarianceOption::Full> eStat{eDyn};
    CheckEq(eStat.Dimension(), 2, "1j: Static from Dynamic Dimension");
    CheckClose(eStat.Value(0), 5.0, "1j: Static from Dynamic Value(0)");
    CheckClose(eStat.Covariance(0, 1), 0.2, "1j: Static from Dynamic Cov(0,1)");

    // Dynamic from Static
    Eigen::Vector2d xStat{7.0, 8.0};
    Eigen::Matrix2d covStat;
    covStat << 1.6, 0.3, 0.3, 2.5;
    Estimate<2, CovarianceOption::Full> eStat2{xStat, covStat};
    Estimate<Eigen::Dynamic, CovarianceOption::Full> eDyn2{eStat2};
    CheckEq(eDyn2.Dimension(), 2, "1j: Dynamic from Static Dimension");
    CheckClose(eDyn2.Value(0), 7.0, "1j: Dynamic from Static Value(0)");
    CheckClose(eDyn2.Covariance(0, 1), 0.3, "1j: Dynamic from Static Cov(0,1)");
}};

// =========================================================================
// Section 3: Assignment Operators
// =========================================================================
constexpr auto sec3Assignment{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    constexpr bool isFull{C == CovarianceOption::Full};

    const auto v{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    auto e1{MakeEstimate<K, C>(v, cov)};
    auto e2{MakeEstimate<K, C>(v, cov)};

    // Move assignment
    auto e3{MakeEstimate<K, C>(v, cov)};
    e3 = std::move(e2);
    if constexpr (K == 1) {
        CheckClose(e3.Value(), 1.0, "3a: MoveAssign Value()");
        CheckClose(e3.Variance(), 0.4, "3a: MoveAssign Variance()");
    } else {
        CheckClose(e3.Value(0), 1.0, "3a: MoveAssign Value(0)");
        if constexpr (dim >= 2) {
            CheckClose(e3.Variance(1), 0.3, "3a: MoveAssign Variance(1)");
        }
    }

    // Copy assignment
    auto e4{MakeEstimate<K, C>(v, cov)};
    e4 = e1;
    if constexpr (K == 1) {
        CheckClose(e4.Value(), 1.0, "3b: CopyAssign Value()");
    } else {
        CheckClose(e4.Value(0), 1.0, "3b: CopyAssign Value(0)");
    }

    // Self-assignment
    auto& e1Ref{e1};
    e1 = e1Ref;
    e1 = std::move(e1Ref);
    if constexpr (K == 1) {
        CheckClose(e1.Value(), 1.0, "3c: SelfAssign Value()");
        CheckClose(e1.Variance(), 0.4, "3c: SelfAssign Variance()");
    } else {
        CheckClose(e1.Value(0), 1.0, "3c: SelfAssign Value(0)");
        if constexpr (isFull and dim >= 2) {
            CheckClose(e1.Covariance(0, 1), 0.1, "3c: SelfAssign Cov(0,1)");
        }
    }

    // Cross-CovarianceOption assignment
    if constexpr (isFull) {
        const auto diagCov{MakeTestCov<K, CovarianceOption::Diagonal>(dim)};
        auto eDiag{MakeEstimate<K, CovarianceOption::Diagonal>(v, diagCov)};
        auto eFull{MakeEstimate<K, C>(v, cov)};
        eFull = eDiag;
        if constexpr (K == 1) {
            CheckClose(eFull.Value(), 1.0, "3d: Full=Diag Value()");
            CheckClose(eFull.Variance(), 0.4, "3d: Full=Diag Variance()");
        } else {
            CheckClose(eFull.Value(0), 1.0, "3d: Full=Diag Value(0)");
            if constexpr (dim >= 2) {
                CheckClose(eFull.Variance(1), 0.3, "3d: Full=Diag Variance(1)");
                CheckClose(eFull.Covariance(0, 1), 0.0, "3d: Full=Diag Cov(0,1)=0");
            }
        }

        auto eDiag2{MakeEstimate<K, CovarianceOption::Diagonal>(v, diagCov)};
        eDiag2 = MakeEstimate<K, C>(v, cov);
        if constexpr (K == 1) {
            CheckClose(eDiag2.Value(), 1.0, "3d: Diag=Full Value()");
            CheckClose(eDiag2.Variance(), 0.4, "3d: Diag=Full Variance()");
        } else {
            CheckClose(eDiag2.Value(0), 1.0, "3d: Diag=Full Value(0)");
            CheckClose(eDiag2.Variance(0), 0.4, "3d: Diag=Full Variance(0)");
            if constexpr (dim >= 2) {
                CheckClose(eDiag2.Covariance(0, 1), 0.0, "3d: Diag=Full Cov(0,1)=0");
            }
        }
    } else {
        const auto fullCov{MakeTestCov<K, CovarianceOption::Full>(dim)};
        auto eFull{MakeEstimate<K, CovarianceOption::Full>(v, fullCov)};
        auto eDiag{MakeEstimate<K, C>(v, cov)};
        eDiag = eFull;
        if constexpr (K == 1) {
            CheckClose(eDiag.Value(), 1.0, "3d: Diag=Full Value()");
            CheckClose(eDiag.Variance(), 0.4, "3d: Diag=Full Variance()");
        } else {
            CheckClose(eDiag.Value(0), 1.0, "3d: Diag=Full Value(0)");
            CheckClose(eDiag.Variance(0), 0.4, "3d: Diag=Full Variance(0)");
            if constexpr (dim >= 2) {
                CheckClose(eDiag.Covariance(0, 1), 0.0, "3d: Diag=Full Cov(0,1)=0");
            }
        }

        auto eFull2{MakeEstimate<K, CovarianceOption::Full>(v, fullCov)};
        eFull2 = MakeEstimate<K, C>(v, cov);
        if constexpr (K == 1) {
            CheckClose(eFull2.Value(), 1.0, "3d: Full=Diag Value()");
            CheckClose(eFull2.Variance(), 0.4, "3d: Full=Diag Variance()");
        } else {
            CheckClose(eFull2.Value(0), 1.0, "3d: Full=Diag Value(0)");
            if constexpr (dim >= 2) {
                CheckClose(eFull2.Variance(1), 0.3, "3d: Full=Diag Variance(1)");
                CheckClose(eFull2.Covariance(0, 1), 0.0, "3d: Full=Diag Cov(0,1)=0");
            }
        }
    }
}};

// =========================================================================
// Section 3: Cross-Dimension Assignment
// =========================================================================
constexpr auto sec3CrossDimAssignment{[] {
    Eigen::VectorXd xDyn{2};
    xDyn << 7.0, 8.0;
    Eigen::MatrixXd covDyn{2, 2};
    covDyn << 4.9, 0.0, 0.0, 6.4;
    Estimate<Eigen::Dynamic, CovarianceOption::Full> eDyn{xDyn, covDyn};

    Eigen::Vector2d xStat{1.0, 2.0};
    Eigen::Matrix2d covStat;
    covStat << 0.1, 0.0, 0.0, 0.4;
    Estimate<2, CovarianceOption::Full> eStat{xStat, covStat};

    eDyn = eStat;
    CheckClose(eDyn.Value(0), 1.0, "3e: Dyn=Stat Value(0)");
    CheckClose(eDyn.Covariance(1, 1), 0.4, "3e: Dyn=Stat Cov(1,1)");
}};

// =========================================================================
// Section 4: Arithmetic In-Place — += and -=
// =========================================================================
constexpr auto sec4AddSubInPlace{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    using Est = Estimate<K, C>;
    constexpr bool isFull{C == CovarianceOption::Full};

    const auto v{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    // e1 += e2
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        auto e2{MakeEstimate<K, C>(v, cov)};
        e1 += e2;
        if constexpr (K == 1) {
            CheckClose(e1.Value(), 2.0, "4a: Value() after +=");
            CheckClose(e1.Variance(), 0.8, "4a: Var() after +=");
        } else {
            for (auto i{0}; i < std::min(dim, 2); ++i) {
                CheckClose(e1.Value(i), 2.0 * v(i), fmt::format("4a: Value({}) after +=", i));
            }
            CheckClose(e1.Variance(0), 0.8, "4a: Var(0) after +=");
            if constexpr (dim >= 2) {
                CheckClose(e1.Variance(1), 0.6, "4a: Var(1) after +=");
            }
        }
    }

    // e1 -= e2
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        auto e2{MakeEstimate<K, C>(v, cov)};
        e1 -= e2;
        if constexpr (K == 1) {
            CheckClose(e1.Value(), 0.0, "4b: Value() after -=");
            CheckClose(e1.Variance(), 0.8, "4b: Var() after -=");
        } else {
            for (auto i{0}; i < std::min(dim, 2); ++i) {
                CheckClose(e1.Value(i), 0.0, fmt::format("4b: Value({}) after -=", i));
            }
            CheckClose(e1.Variance(0), 0.8, "4b: Var(0) after -=");
            if constexpr (dim >= 2) {
                CheckClose(e1.Variance(1), 0.6, "4b: Var(1) after -=");
            }
        }
    }

    // += with vector
    if constexpr (K != 1) {
        auto e1{MakeEstimate<K, C>(v, cov)};
        e1 += v;
        for (auto i{0}; i < std::min(dim, 2); ++i) {
            CheckClose(e1.Value(i), 2.0 * v(i), fmt::format("4d: Value({}) +=vec", i));
        }
        CheckClose(e1.Variance(0), 0.4, "4d: Var(0) +=vec unchanged");
    }

    // += with scalar
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        e1 += 10.0;
        if constexpr (K == 1) {
            CheckClose(e1.Value(), 11.0, "4e: Value() +=scalar");
            CheckClose(e1.Variance(), 0.4, "4e: Var() +=scalar unchanged");
        } else {
            CheckClose(e1.Value(0), 11.0, "4e: Value(0) +=scalar");
            if constexpr (dim >= 2) {
                CheckClose(e1.Value(1), 12.0, "4e: Value(1) +=scalar");
            }
            CheckClose(e1.Variance(0), 0.4, "4e: Var(0) +=scalar unchanged");
        }
    }

    // -= with vector/scalar
    if constexpr (K != 1) {
        auto e1{MakeEstimate<K, C>(v, cov)};
        e1 -= v;
        CheckClose(e1.Value(0), 0.0, "4f: Value(0) -=vec");
        if constexpr (dim >= 2) {
            CheckClose(e1.Value(1), 0.0, "4f: Value(1) -=vec");
        }
    }
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        e1 -= 5.0;
        if constexpr (K == 1) {
            CheckClose(e1.Value(), -4.0, "4f: Value() -=scalar");
        } else {
            CheckClose(e1.Value(0), -4.0, "4f: Value(0) -=scalar");
            if constexpr (dim >= 2) {
                CheckClose(e1.Value(1), -3.0, "4f: Value(1) -=scalar");
            }
        }
    }

    // Self +=
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        e1 += e1;
        if constexpr (K == 1) {
            CheckClose(e1.Value(), 2.0, "4c: Value() self+=");
            CheckClose(e1.Variance(), 0.8, "4c: Var() self+=");
        } else {
            CheckClose(e1.Value(0), 2.0, "4c: Value(0) self+=");
            if constexpr (dim >= 2) {
                CheckClose(e1.Value(1), 4.0, "4c: Value(1) self+=");
                CheckClose(e1.Variance(1), 0.6, "4c: Var(1) self+=");
            }
            CheckClose(e1.Variance(0), 0.8, "4c: Var(0) self+=");
        }
    }

    // Dimension mismatch (dynamic only)
    if constexpr (K == Eigen::Dynamic) {
        Est e1{2};
        e1 += MakeTestValue<K, C>(2);
        auto threw{false};
        try {
            Est e3{3};
            e1 += e3;
        } catch (const std::invalid_argument&) { threw = true; }
        if (not threw) {
            Throw<std::runtime_error>("4j: dimension mismatch should throw");
        }
    }

    if constexpr (isFull and K != 1 and dim >= 2) {
        // Full += Diagonal: off-diagonals from Full preserved
        const auto diagCov{MakeTestCov<K, CovarianceOption::Diagonal>(dim)};
        auto eD{MakeEstimate<K, CovarianceOption::Diagonal>(v, diagCov)};
        auto eF{MakeEstimate<K, C>(v, cov)};
        eF += eD;
        CheckClose(eF.Covariance(0, 1), 0.1, "4h: Cov(0,1) Full+=Diag preserved");
    }

    if constexpr (isFull and K != 1 and dim >= 2) {
        // Full *= Diagonal: Cov = diag(y)·Cov_full·diag(y) + diag contribution on diagonal
        // Off-diagonal: cov_ij = y_i * cov_full(i,j) * y_j
        const auto diagCov{MakeTestCov<K, CovarianceOption::Diagonal>(dim)};
        auto eD{MakeEstimate<K, CovarianceOption::Diagonal>(v, diagCov)};
        auto eF{MakeEstimate<K, C>(v, cov)};
        eF *= eD;
        CheckClose(eF.Value(0), v(0) * v(0), "4i: Value(0) Full*=Diag");
        CheckClose(eF.Value(1), v(1) * v(1), "4i: Value(1) Full*=Diag");
        CheckClose(eF.Covariance(0, 1), cov.coeff(0, 1) * v(0) * v(1), "4i: Cov(0,1) Full*=Diag");
    }

    if constexpr (isFull and K != 1 and dim >= 2) {
        // Full /= Diagonal: Cov = diag(1/y)·Cov_full·diag(1/y) + diag contribution on diagonal
        // Off-diagonal: cov_ij = cov_full(i,j) / (y_i * y_j)
        const auto diagCov{MakeTestCov<K, CovarianceOption::Diagonal>(dim)};
        auto eD{MakeEstimate<K, CovarianceOption::Diagonal>(v, diagCov)};
        auto eF{MakeEstimate<K, C>(v, cov)};
        eF /= eD;
        CheckClose(eF.Value(0), 1.0, "4j: Value(0) Full/=Diag");
        CheckClose(eF.Value(1), 1.0, "4j: Value(1) Full/=Diag");
        CheckClose(eF.Covariance(0, 1), cov.coeff(0, 1) / (v(0) * v(1)), "4j: Cov(0,1) Full/=Diag");
    }
}};

// =========================================================================
// Section 5: Arithmetic Free Functions — +, -, unary -
// =========================================================================
constexpr auto sec5FreeAddSub{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    const auto v{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    auto e1{MakeEstimate<K, C>(v, cov)};
    auto e2{MakeEstimate<K, C>(v, cov)};

    // e1 + e2
    {
        const auto e3{e1 + e2};
        if constexpr (K == 1) {
            CheckClose(e3.Value(), 2.0, "5a: Value() e1+e2");
            CheckClose(e3.Variance(), 0.8, "5a: Var() e1+e2");
        } else {
            CheckClose(e3.Value(0), 2.0, "5a: Value(0) e1+e2");
            if constexpr (dim >= 2) {
                CheckClose(e3.Value(1), 4.0, "5a: Value(1) e1+e2");
                CheckClose(e3.Variance(1), 0.6, "5a: Var(1) e1+e2");
            }
            CheckClose(e3.Variance(0), 0.8, "5a: Var(0) e1+e2");
        }
    }

    // e1 - e2
    {
        const auto e4{e1 - e2};
        if constexpr (K == 1) {
            CheckClose(e4.Value(), 0.0, "5b: Value() e1-e2");
            CheckClose(e4.Variance(), 0.8, "5b: Var() e1-e2");
        } else {
            CheckClose(e4.Value(0), 0.0, "5b: Value(0) e1-e2");
            if constexpr (dim >= 2) {
                CheckClose(e4.Value(1), 0.0, "5b: Value(1) e1-e2");
            }
            CheckClose(e4.Variance(0), 0.8, "5b: Var(0) e1-e2");
        }
    }

    // Unary -
    {
        const auto e5{-e1};
        if constexpr (K == 1) {
            CheckClose(e5.Value(), -1.0, "5f: Value() -e");
            CheckClose(e5.Variance(), e1.Variance(), "5f: Var() -e unchanged");
        } else {
            CheckClose(e5.Value(0), -1.0, "5f: Value(0) -e");
            if constexpr (dim >= 2) {
                CheckClose(e5.Value(1), -2.0, "5f: Value(1) -e");
            }
            CheckClose(e5.Variance(0), e1.Variance(0), "5f: Var(0) -e unchanged");
        }
    }

    // e + vector
    if constexpr (K != 1) {
        const auto e6{e1 + v};
        CheckClose(e6.Value(0), 2.0, "5g: Value(0) e+vec");
        CheckClose(e6.Variance(0), 0.4, "5g: Var(0) e+vec unchanged");
    }

    // vector + e
    if constexpr (K != 1) {
        const auto e7{v + e1};
        CheckClose(e7.Value(0), 2.0, "5g: Value(0) vec+e");
        CheckClose(e7.Variance(0), 0.4, "5g: Var(0) vec+e unchanged");
    }

    // e + scalar
    {
        const auto e8{e1 + 10.0};
        if constexpr (K == 1) {
            CheckClose(e8.Value(), 11.0, "5h: Value() e+scalar");
            CheckClose(e8.Variance(), 0.4, "5h: Var() e+scalar unchanged");
        } else {
            CheckClose(e8.Value(0), 11.0, "5h: Value(0) e+scalar");
            CheckClose(e8.Variance(0), 0.4, "5h: Var(0) e+scalar unchanged");
        }
    }

    // scalar + e
    {
        const auto e9{10.0 + e1};
        if constexpr (K == 1) {
            CheckClose(e9.Value(), 11.0, "5h: Value() scalar+e");
        } else {
            CheckClose(e9.Value(0), 11.0, "5h: Value(0) scalar+e");
        }
    }

    // e - vector
    if constexpr (K != 1) {
        const auto e10{e1 - v};
        CheckClose(e10.Value(0), 0.0, "5i: Value(0) e-vec");
    }

    // vector - e (uses NegateAdd)
    if constexpr (K != 1) {
        const auto e11{v - e1};
        CheckClose(e11.Value(0), 0.0, "5i: Value(0) vec-e");
        CheckClose(e11.Variance(0), 0.4, "5i: Var(0) vec-e unchanged");
    }
}};

// =========================================================================
// Section 6: Arithmetic In-Place — *= and /=
// =========================================================================
constexpr auto sec6MulDivInPlace{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    constexpr bool isFull{C == CovarianceOption::Full};

    const auto v{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    // e1 *= e2: J_x = diag(y), J_y = diag(x)
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        auto e2{MakeEstimate<K, C>(v, cov)};
        e1 *= e2;
        if constexpr (K == 1) {
            CheckClose(e1.Value(), 1.0, "6a: Value() *=e2");
            CheckClose(e1.Variance(), 0.8, "6a: Var() *=e2");
        } else {
            CheckClose(e1.Value(0), 1.0, "6a: Value(0) *=e2");
            if constexpr (dim >= 2) {
                CheckClose(e1.Value(1), 4.0, "6a: Value(1) *=e2");
                CheckClose(e1.Covariance(1, 1), 2.4, "6a: Cov(1,1) *=e2");
                if constexpr (isFull) {
                    CheckClose(e1.Covariance(0, 1), 0.4, "6a: Cov(0,1) *=e2");
                }
            }
            CheckClose(e1.Covariance(0, 0), 0.8, "6a: Cov(0,0) *=e2");
        }
    }

    // e1 *= vector: J = diag(v)
    if constexpr (K != 1) {
        auto e1{MakeEstimate<K, C>(v, cov)};
        e1 *= v;
        CheckClose(e1.Value(0), 1.0, "6b: Value(0) *=vec");
        if constexpr (dim >= 2) {
            CheckClose(e1.Value(1), 4.0, "6b: Value(1) *=vec");
            if constexpr (isFull) {
                CheckClose(e1.Covariance(0, 1), 0.2, "6b: Cov(0,1) *=vec");
            }
        }
        CheckClose(e1.Covariance(0, 0), 0.4, "6b: Cov(0,0) *=vec");
    }

    // e1 *= scalar
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        e1 *= 3.0;
        if constexpr (K == 1) {
            CheckClose(e1.Value(), 3.0, "6c: Value() *=3");
            CheckClose(e1.Variance(), 3.6, "6c: Var() *=3");
        } else {
            CheckClose(e1.Value(0), 3.0, "6c: Value(0) *=3");
            if constexpr (dim >= 2) {
                CheckClose(e1.Value(1), 6.0, "6c: Value(1) *=3");
            }
            CheckClose(e1.Covariance(0, 0), 3.6, "6c: Cov(0,0) *=3");
        }
    }

    // e1 /= e2
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        auto e2{MakeEstimate<K, C>(v, cov)};
        e1 /= e2;
        if constexpr (K == 1) {
            CheckClose(e1.Value(), 1.0, "6d: Value() /=e2");
            CheckClose(e1.Variance(), 0.8, "6d: Var() /=e2");
        } else {
            CheckClose(e1.Value(0), 1.0, "6d: Value(0) /=e2");
            if constexpr (dim >= 2) {
                CheckClose(e1.Value(1), 1.0, "6d: Value(1) /=e2");
                CheckClose(e1.Covariance(1, 1), 0.15, "6d: Cov(1,1) /=e2");
            }
            CheckClose(e1.Covariance(0, 0), 0.8, "6d: Cov(0,0) /=e2");
        }
    }

    // e1 /= vector: J = diag(1/y)
    if constexpr (K != 1) {
        auto e1{MakeEstimate<K, C>(v, cov)};
        e1 /= v;
        CheckClose(e1.Value(0), 1.0, "6e: Value(0) /=vec");
        if constexpr (dim >= 2) {
            CheckClose(e1.Value(1), 1.0, "6e: Value(1) /=vec");
        }
        // Cov = diag(1/y) * Cov * diag(1/y)
        for (auto i{0}; i < dim; ++i) {
            for (auto j{0}; j < dim; ++j) {
                const auto expectedCov{cov.coeff(i, j) / (v(i) * v(j))};
                CheckClose(e1.Covariance(i, j), expectedCov,
                           fmt::format("6e: Cov({},{}) /=vec", i, j));
            }
        }
    }

    // e1 /= scalar
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        e1 /= 2.0;
        if constexpr (K == 1) {
            CheckClose(e1.Value(), 0.5, "6f: Value() /=2");
            CheckClose(e1.Variance(), 0.1, "6f: Var() /=2");
        } else {
            CheckClose(e1.Value(0), 0.5, "6f: Value(0) /=2");
            if constexpr (dim >= 2) {
                CheckClose(e1.Value(1), 1.0, "6f: Value(1) /=2");
            }
            CheckClose(e1.Covariance(0, 0), 0.1, "6f: Cov(0,0) /=2");
        }
    }

    // e1 *= 0
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        e1 *= 0.0;
        if constexpr (K == 1) {
            CheckClose(e1.Value(), 0.0, "6j: Value() *=0");
            CheckClose(e1.Variance(), 0.0, "6j: Var() *=0");
        } else {
            CheckClose(e1.Value(0), 0.0, "6j: Value(0) *=0");
            if constexpr (dim >= 2) {
                CheckClose(e1.Value(1), 0.0, "6j: Value(1) *=0");
                CheckClose(e1.Covariance(1, 1), 0.0, "6j: Cov(1,1) *=0");
            }
            CheckClose(e1.Covariance(0, 0), 0.0, "6j: Cov(0,0) *=0");
        }
    }
}};

// =========================================================================
// Section 7: Free Functions — *, /, pow, NegateAdd, Divide, Pow, Exp
// =========================================================================
constexpr auto sec7FreeMulDivPow{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    const auto v{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    auto e1{MakeEstimate<K, C>(v, cov)};
    auto e2{MakeEstimate<K, C>(v, cov)};

    // e1 * e2 free function
    {
        const auto e3{e1 * e2};
        if constexpr (K == 1) {
            CheckClose(e3.Value(), 1.0, "7a: Value() e1*e2");
        } else {
            CheckClose(e3.Value(0), 1.0, "7a: Value(0) e1*e2");
            if constexpr (dim >= 2) {
                CheckClose(e3.Value(1), 4.0, "7a: Value(1) e1*e2");
            }
        }
        // Cov = diag(y)·Cov1·diag(y) + diag(x)·Cov2·diag(x)
        if constexpr (K != 1) {
            CheckBinaryCov(e3, cov, cov, "7a: e1*e2 cov",
                           [&](int i) { return v(i); },
                           [&](int i) { return v(i); });
        } else {
            CheckClose(e3.Variance(), 0.8, "7a: Var() e1*e2");
        }
    }

    // e1 / e2 free function
    {
        const auto e4{e1 / e2};
        if constexpr (K == 1) {
            CheckClose(e4.Value(), 1.0, "7b: Value() e1/e2");
        } else {
            CheckClose(e4.Value(0), 1.0, "7b: Value(0) e1/e2");
        }
        // Cov = diag(1/y)·Cov1·diag(1/y) + diag(x/y²)·Cov2·diag(x/y²)
        if constexpr (K != 1) {
            CheckBinaryCov(e4, cov, cov, "7b: e1/e2 cov",
                           [&](int i) { return 1.0 / v(i); },
                           [&](int i) { return 1.0 / v(i); });
        } else {
            CheckClose(e4.Variance(), 0.8, "7b: Var() e1/e2");
        }
    }

    // e * vec, e * scalar, scalar * e
    if constexpr (K != 1) {
        const auto e5{e1 * v};
        CheckClose(e5.Value(0), 1.0, "7c: Value(0) e*vec");
    }
    {
        const auto e6{e1 * 3.0};
        if constexpr (K == 1) {
            CheckClose(e6.Value(), 3.0, "7c: Value() e*scalar");
        } else {
            CheckClose(e6.Value(0), 3.0, "7c: Value(0) e*scalar");
        }
        const auto e7{3.0 * e1};
        if constexpr (K == 1) {
            CheckClose(e7.Value(), 3.0, "7c: Value() scalar*e");
        } else {
            CheckClose(e7.Value(0), 3.0, "7c: Value(0) scalar*e");
        }
    }

    // e / scalar, vec / e
    {
        const auto e8{e1 / 2.0};
        if constexpr (K == 1) {
            CheckClose(e8.Value(), 0.5, "7d: Value() e/scalar");
        } else {
            CheckClose(e8.Value(0), 0.5, "7d: Value(0) e/scalar");
        }
    }
    if constexpr (K != 1) {
        const auto e9{v / e1};
        CheckClose(e9.Value(0), 1.0, "7d: Value(0) vec/e");
    }

    // NegateAdd
    {
        auto e10{MakeEstimate<K, C>(v, cov)};
        e10.NegateAddInPlace(e2);
        if constexpr (K == 1) {
            CheckClose(e10.Value(), 0.0, "7e: Value() NegateAdd(e2)");
        } else {
            CheckClose(e10.Value(0), 0.0, "7e: Value(0) NegateAdd(e2)");
        }
        // Cov = Cov_x + Cov_y (f = y - x, J_x = -1, J_y = 1, minus sign squares away)
        if constexpr (K != 1) {
            CheckBinaryCov(e10, cov, cov, "7e: NegateAdd(e2) cov",
                           [](int) { return 1.0; },
                           [](int) { return 1.0; });
        } else {
            CheckClose(e10.Variance(), 0.8, "7e: Var() NegateAdd(e2)");
        }
    }

    // Divide
    {
        auto e11{MakeEstimate<K, C>(v, cov)};
        e11.DivideInPlace(e2);
        if constexpr (K == 1) {
            CheckClose(e11.Value(), 1.0, "7f: Value() Divide(e2)");
        } else {
            CheckClose(e11.Value(0), 1.0, "7f: Value(0) Divide(e2)");
        }
        // Cov = diag(y/x²)·Cov1·diag(y/x²) + diag(1/x)·Cov2·diag(1/x)
        if constexpr (K != 1) {
            CheckBinaryCov(e11, cov, cov, "7f: Divide(e2) cov",
                           [&](int i) { return 1.0 / v(i); },
                           [&](int i) { return 1.0 / v(i); });
        } else {
            CheckClose(e11.Variance(), 0.8, "7f: Var() Divide(e2)");
        }
    }

    // pow(e1, e2)
    {
        const auto e12{pow(MakeEstimate<K, C>(v, cov), e2)};
        if constexpr (K == 1) {
            CheckClose(e12.Value(), 1.0, "7g: Value() pow(e1,e2)");
        } else {
            CheckClose(e12.Value(0), 1.0, "7g: Value(0) pow(e1,e2)");
        }
        // Cov = diag(y·x^(y-1))·Cov1·diag(y·x^(y-1)) + diag(x^y·log(x))·Cov2·diag(x^y·log(x))
        // When x=y=v(i), J_x = v(i)^v(i), J_y = v(i)^v(i)·log(v(i))
        if constexpr (K != 1) {
            CheckBinaryCov(e12, cov, cov, "7g: pow(e1,e2) cov",
                           [&](int i) { return std::pow(v(i), v(i)); },
                           [&](int i) { return std::pow(v(i), v(i)) * std::log(v(i)); });
        } else {
            auto jacX{std::pow(v(0), v(0))};
            auto jacY{std::pow(v(0), v(0)) * std::log(v(0))};
            CheckClose(e12.Variance(), (jacX * jacX + jacY * jacY) * cov.diagonal()(0), "7g: Var() pow(e1,e2)");
        }
    }

    // pow(e, scalar)
    {
        const auto e13{pow(e1, 2.0)};
        if constexpr (K == 1) {
            CheckClose(e13.Value(), 1.0, "7h: Value() pow(e,2)");
        } else {
            CheckClose(e13.Value(0), 1.0, "7h: Value(0) pow(e,2)");
            if constexpr (dim >= 2) {
                CheckClose(e13.Value(1), 4.0, "7h: Value(1) pow(e,2)");
            }
        }
    }

    // Exp(base, exponent)
    {
        auto e14{MakeEstimate<K, C>(v, cov)};
        e14.ExpInPlace(e2);
        if constexpr (K == 1) {
            CheckClose(e14.Value(), 1.0, "7i: Value() Exp(base,exp)");
        } else {
            CheckClose(e14.Value(0), 1.0, "7i: Value(0) Exp(base,exp)");
        }
        // Cov = diag(y^x·log(y))·Cov1·diag(y^x·log(y)) + diag(x/y·y^x)·Cov2·diag(x/y·y^x)
        // When x=y=v(i), J_x = v(i)^v(i)·log(v(i)), J_y = v(i)^v(i)
        if constexpr (K != 1) {
            CheckBinaryCov(e14, cov, cov, "7i: Exp(base,e) cov",
                           [&](int i) { return std::pow(v(i), v(i)) * std::log(v(i)); },
                           [&](int i) { return std::pow(v(i), v(i)); });
        } else {
            auto jacX{std::pow(v(0), v(0)) * std::log(v(0))};
            auto jacY{std::pow(v(0), v(0))};
            CheckClose(e14.Variance(), (jacX * jacX + jacY * jacY) * cov.diagonal()(0), "7i: Var() Exp(base,e)");
        }
    }

    // NegateAdd with vector and scalar
    if constexpr (K != 1) {
        auto e15{MakeEstimate<K, C>(v, cov)};
        e15.NegateAddInPlace(v);
        CheckClose(e15.Value(0), 0.0, "7j: NegateAdd(vec) value(0)");
    }
    {
        auto e16{MakeEstimate<K, C>(v, cov)};
        e16.NegateAddInPlace(10.0);
        if constexpr (K == 1) {
            CheckClose(e16.Value(), 9.0, "7j: NegateAdd(scalar) value()");
        } else {
            CheckClose(e16.Value(0), 9.0, "7j: NegateAdd(scalar) value(0)");
        }
    }

    // Divide with vector
    if constexpr (K != 1) {
        auto e17{MakeEstimate<K, C>(v, cov)};
        e17.DivideInPlace(v);
        CheckClose(e17.Value(0), 1.0, "7k: Divide(vec) value(0)");
    }

    // Divide(scalar): x → c/x, J = diag(-c/x²)
    {
        auto e18{MakeEstimate<K, C>(v, cov)};
        e18.DivideInPlace(10.0);
        const auto jacDiv{[&](int i) { return -10.0 / (v(i) * v(i)); }};
        if constexpr (K == 1) {
            CheckClose(e18.Value(), 10.0, "7k: Divide(scalar) value()");
            CheckClose(e18.Variance(), jacDiv(0) * jacDiv(0) * cov.coeff(0, 0), "7k: Divide(scalar) var");
        } else {
            CheckClose(e18.Value(0), 10.0, "7k: Divide(scalar) value(0)");
            CheckMathFunction(e18, cov, "7k: Divide(scalar)",
                              [&](int i) { return -10.0 / (v(i) * v(i)); });
        }
    }

    // Pow(vec): x → x^y, J = diag(y/x · x^y)
    if constexpr (K != 1) {
        auto eVecPow{MakeEstimate<K, C>(v, cov)};
        eVecPow.PowInPlace(v);
        if constexpr (K == Eigen::Dynamic) {
            CheckClose(eVecPow.Value(0), std::pow(v(0), v(0)), "7l: Pow(vec) value(0)");
        } else {
            CheckClose(eVecPow.Value(0), std::pow(v(0), v(0)), "7l: Pow(vec) value(0)");
        }
        CheckMathFunction(eVecPow, cov, "7l: Pow(vec)",
                          [&](int i) {
                              auto powVal{std::pow(v(i), v(i))};
                              return v(i) / v(i) * powVal;
                          });
    }

    // Pow(scalar): x → x^c, J = diag(c/x · x^c)
    {
        auto eScalarPow{MakeEstimate<K, C>(v, cov)};
        eScalarPow.PowInPlace(3.0);
        constexpr double cPow{3.0};
        if constexpr (K == 1) {
            CheckClose(eScalarPow.Value(), std::pow(v(0), cPow), "7m: Pow(scalar) value()");
            auto jac{cPow / v(0) * std::pow(v(0), cPow)};
            CheckClose(eScalarPow.Variance(), jac * jac * cov.coeff(0, 0), "7m: Pow(scalar) var");
        } else {
            CheckClose(eScalarPow.Value(0), std::pow(v(0), cPow), "7m: Pow(scalar) value(0)");
            CheckMathFunction(eScalarPow, cov, "7m: Pow(scalar)",
                              [&](int i) { return cPow / v(i) * std::pow(v(i), cPow); });
        }
    }

    // Exp(vec): x → y^x = exp(x·log(y)), J = diag(y^x · log(y))
    if constexpr (K != 1) {
        // Use y = [2, 3, ...] to avoid log(1)=0 masking
        Eigen::Vector<double, K> baseVec;
        if constexpr (K == Eigen::Dynamic) {
            baseVec.resize(dim);
        }
        baseVec.setConstant(2.0);
        auto eVecExp{MakeEstimate<K, C>(v, cov)};
        eVecExp.ExpInPlace(baseVec);
        CheckClose(eVecExp.Value(0), std::pow(2.0, v(0)), "7n: Exp(vec) value(0)");
        CheckMathFunction(eVecExp, cov, "7n: Exp(vec)",
                          [&](int i) {
                              return std::pow(2.0, v(i)) * std::log(2.0);
                          });
    }

    // Exp(scalar): x → c^x = exp(x·log(c)), J = diag(c^x · log(c))
    {
        auto eScalarExp{MakeEstimate<K, C>(v, cov)};
        eScalarExp.ExpInPlace(2.0);
        constexpr double cExp{2.0};
        if constexpr (K == 1) {
            CheckClose(eScalarExp.Value(), std::pow(cExp, v(0)), "7o: Exp(scalar) value()");
            auto jac{std::pow(cExp, v(0)) * std::log(cExp)};
            CheckClose(eScalarExp.Variance(), jac * jac * cov.coeff(0, 0), "7o: Exp(scalar) var");
        } else {
            CheckClose(eScalarExp.Value(0), std::pow(cExp, v(0)), "7o: Exp(scalar) value(0)");
            CheckMathFunction(eScalarExp, cov, "7o: Exp(scalar)",
                              [&](int i) { return std::pow(cExp, v(i)) * std::log(cExp); });
        }
    }

    // pow(double, Estimate)
    {
        const auto e19{pow(2.0, MakeEstimate<K, C>(v, cov))};
        if constexpr (K == 1) {
            CheckClose(e19.Value(), 2.0, "7l: pow(scalar,e) value()");
        } else {
            CheckClose(e19.Value(0), 2.0, "7l: pow(scalar,e) value(0)");
        }
    }
}};

// =========================================================================
// Section 7b: Self-Operations — aliasing (same Estimate on both sides)
// =========================================================================
constexpr auto sec7bSelfOps{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    const auto v{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    // e -= e : Cov(-x, +y) adds covariances → Cov_result = 2*Cov
    {
        auto e{MakeEstimate<K, C>(v, cov)};
        e -= e;
        if constexpr (K == 1) {
            CheckClose(e.Value(), 0.0, "7s: Value() self-=");
            CheckClose(e.Variance(), 0.8, "7s: Var() self-=");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), 0.0, fmt::format("7s: Value({}) self-=", i));
            }
            CheckBinaryCov(e, cov, cov, "7s: self-= cov",
                           [](int) { return 1.0; },
                           [](int) { return 1.0; });
        }
    }

    // e *= e : J_x = diag(v), J_y = diag(v) → Cov = 2·diag(v)·Cov·diag(v)
    {
        auto e{MakeEstimate<K, C>(v, cov)};
        e *= e;
        if constexpr (K == 1) {
            CheckClose(e.Value(), 1.0, "7s: Value() self*=");
            CheckClose(e.Variance(), 0.8, "7s: Var() self*=");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), v(i) * v(i), fmt::format("7s: Value({}) self*=", i));
            }
            CheckBinaryCov(e, cov, cov, "7s: self*= cov",
                           [&](int i) { return v(i); },
                           [&](int i) { return v(i); });
        }
    }

    // e /= e : Cov update now before X update → safe
    // Correct: J_x = 1/v, J_y = x/y² = 1/v → Cov = 2·diag(1/v)·Cov·diag(1/v)
    {
        auto e{MakeEstimate<K, C>(v, cov)};
        e /= e;
        if constexpr (K == 1) {
            CheckClose(e.Value(), 1.0, "7s: Value() self/=");
            CheckClose(e.Variance(), 0.8, "7s: Var() self/=");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), 1.0, fmt::format("7s: Value({}) self/=", i));
            }
            CheckBinaryCov(e, cov, cov, "7s: self/= cov",
                           [&](int i) { return 1.0 / v(i); },
                           [&](int i) { return 1.0 / v(i); });
        }
    }

    // e.NegateAdd(e) : f = v - v = 0, Cov = Cov + Cov = 2·Cov
    {
        auto e{MakeEstimate<K, C>(v, cov)};
        e.NegateAddInPlace(e);
        if constexpr (K == 1) {
            CheckClose(e.Value(), 0.0, "7s: Value() self-NegateAdd");
            CheckClose(e.Variance(), 0.8, "7s: Var() self-NegateAdd");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), 0.0, fmt::format("7s: Value({}) self-NegateAdd", i));
            }
            CheckBinaryCov(e, cov, cov, "7s: self-NegateAdd cov",
                           [](int) { return 1.0; },
                           [](int) { return 1.0; });
        }
    }

    // e.Divide(e) : Cov update now before X update → safe
    // f = v/v = 1. J_x = y/x² = 1/v, J_y = 1/x = 1/v
    {
        auto e{MakeEstimate<K, C>(v, cov)};
        e.DivideInPlace(e);
        if constexpr (K == 1) {
            CheckClose(e.Value(), 1.0, "7s: Value() self-Divide");
            CheckClose(e.Variance(), 0.8, "7s: Var() self-Divide");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), 1.0, fmt::format("7s: Value({}) self-Divide", i));
            }
            CheckBinaryCov(e, cov, cov, "7s: self-Divide cov",
                           [&](int i) { return 1.0 / v(i); },
                           [&](int i) { return 1.0 / v(i); });
        }
    }

    // e.Pow(e) : f = v^v, J_x = v^v, J_y = v^v·log(v)
    // Temporaries (logX, xPowY) computed before ArrX update → safe
    {
        auto e{MakeEstimate<K, C>(v, cov)};
        e.PowInPlace(e);
        if constexpr (K == 1) {
            CheckClose(e.Value(), 1.0, "7s: Value() self-Pow");
            auto jacX{std::pow(v(0), v(0))};
            auto jacY{std::pow(v(0), v(0)) * std::log(v(0))};
            CheckClose(e.Variance(), (jacX * jacX + jacY * jacY) * cov.diagonal()(0), "7s: Var() self-Pow");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::pow(v(i), v(i)), fmt::format("7s: Value({}) self-Pow", i));
            }
            CheckBinaryCov(e, cov, cov, "7s: self-Pow cov",
                           [&](int i) { return std::pow(v(i), v(i)); },
                           [&](int i) { return std::pow(v(i), v(i)) * std::log(v(i)); });
        }
    }

    // e.Exp(e) : f = v^v, J_x = v^v·log(v), J_y = v^v
    // Temporaries (logY, yPowX) computed before ArrX update → safe
    {
        auto e{MakeEstimate<K, C>(v, cov)};
        e.ExpInPlace(e);
        if constexpr (K == 1) {
            CheckClose(e.Value(), 1.0, "7s: Value() self-Exp");
            auto jacX{std::pow(v(0), v(0)) * std::log(v(0))};
            auto jacY{std::pow(v(0), v(0))};
            CheckClose(e.Variance(), (jacX * jacX + jacY * jacY) * cov.diagonal()(0), "7s: Var() self-Exp");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::pow(v(i), v(i)), fmt::format("7s: Value({}) self-Exp", i));
            }
            CheckBinaryCov(e, cov, cov, "7s: self-Exp cov",
                           [&](int i) { return std::pow(v(i), v(i)) * std::log(v(i)); },
                           [&](int i) { return std::pow(v(i), v(i)); });
        }
    }
}};

// =========================================================================
// Section 8: Dot Product
// =========================================================================
constexpr auto sec8DotProduct{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};

        Est e1{v, cov};
        Est e2{v, cov};

        // e1.Dot(e2)
        {
            const auto dotResult{e1.Dot(e2)};
            auto expectedValue{0.0};
            for (auto i{0}; i < dim; ++i) {
                expectedValue += v(i) * v(i);
            }
            CheckClose(dotResult.Value(), expectedValue, "8a: Dot value");
            auto expectedVar{0.0};
            for (auto i{0}; i < dim; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    expectedVar += v(i) * e1.Covariance(i, j) * v(j);
                }
            }
            expectedVar *= 2.0; // both sides contribute equally
            CheckClose(dotResult.Variance(), expectedVar, "8a: Dot variance");
        }

        // e.Dot(vector)
        {
            const auto dotResult{e1.Dot(v)};
            auto expectedValue{0.0};
            for (auto i{0}; i < dim; ++i) {
                expectedValue += v(i) * v(i);
            }
            CheckClose(dotResult.Value(), expectedValue, "8c: Dot with vector value");
            auto expectedVar{0.0};
            for (auto i{0}; i < dim; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    expectedVar += v(i) * e1.Covariance(i, j) * v(j);
                }
            }
            CheckClose(dotResult.Variance(), expectedVar, "8c: Dot with vector variance");
        }

        // Free function Dot
        {
            const auto dotResult{Dot(e1, e2)};
            auto expectedValue{0.0};
            for (auto i{0}; i < dim; ++i) {
                expectedValue += v(i) * v(i);
            }
            CheckClose(dotResult.Value(), expectedValue, "8d: Free Dot value");

            const auto dotResult2{Dot(e1, v)};
            CheckClose(dotResult2.Value(), expectedValue, "8e: Free Dot(e,v) value");

            const auto dotResult3{Dot(v, e2)};
            CheckClose(dotResult3.Value(), expectedValue, "8e: Free Dot(v,e) value");
        }

        // Result type is Estimate<1, C>
        static_assert(std::is_same_v<decltype(e1.Dot(e2)), Estimate<1, C>>);
    }
}};

// =========================================================================
// Section 8h: Matrix-vector RightMultiply — square and non-square
// =========================================================================

constexpr auto sec8hRightMultiply{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};

        // ---- RightMultiply with square matrix ----
        {
            Eigen::Matrix<double, K, K> matA;
            if constexpr (K == Eigen::Dynamic) { matA.resize(dim, dim); }
            matA.setIdentity();

            Est e{v, cov};
            auto result{e.RightMultiply(matA)};
            // result should be Estimate<K, C> (same dimension)
            for (auto i{0}; i < dim; ++i) {
                CheckClose(result.Value(i), v(i),
                           fmt::format("8h: RightMultiply sq Value({})", i));
            }
        }

        // ---- RightMultiply with non-square matrix (K × (K+1)) ----
        {
            constexpr int M{K == Eigen::Dynamic ? Eigen::Dynamic : K + 1};
            constexpr int actualM{(K == Eigen::Dynamic) ? dim + 1 : K + 1};

            Eigen::Matrix<double, K, M> matRect;
            if constexpr (K == Eigen::Dynamic or M == Eigen::Dynamic) {
                matRect.resize(dim, actualM);
            }
            for (auto i{0}; i < dim; ++i) {
                for (auto j{0}; j < actualM; ++j) {
                    matRect(i, j) = (i + 1) * 10 + (j + 1);
                }
            }

            Est e{v, cov};
            auto result{e.RightMultiply(matRect)};
            CheckEq(result.Dimension(), actualM, "8h: RightMultiply non-sq dim");

            // expected value = A^T * v
            for (auto j{0}; j < actualM; ++j) {
                auto expectedVal{0.0};
                for (auto i{0}; i < dim; ++i) {
                    expectedVal += matRect(i, j) * v(i);
                }
                CheckClose(result.Value(j), expectedVal,
                           fmt::format("8h: RightMultiply non-sq Value({})", j));
            }

            // expected covariance = A^T * Cov * A
            for (auto j1{0}; j1 < actualM; ++j1) {
                for (auto j2{0}; j2 < actualM; ++j2) {
                    auto expectedCov{0.0};
                    for (auto p{0}; p < dim; ++p) {
                        for (auto q{0}; q < dim; ++q) {
                            auto cov_pq{[&] {
                                if constexpr (C == CovarianceOption::Full) {
                                    return cov(p, q);
                                } else {
                                    return (p == q) ? cov.diagonal()(p) : 0.0;
                                }
                            }()};
                            expectedCov += matRect(p, j1) * cov_pq * matRect(q, j2);
                        }
                    }
                    if constexpr (C == CovarianceOption::Full) {
                        CheckClose(result.Covariance(j1, j2), expectedCov,
                                   fmt::format("8h: RightMultiply non-sq Cov({},{})", j1, j2));
                    } else {
                        if (j1 == j2) {
                            CheckClose(result.Covariance(j1, j2), expectedCov,
                                       fmt::format("8h: RightMultiply non-sq Var({})", j1));
                        }
                    }
                }
            }
        }
    }
}};

// =========================================================================
// Section 8i: Matrix-vector operator*
// =========================================================================

constexpr auto sec8iMatrixMultiplyOperator{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};

        // ---- operator*(est, matrix) ----
        {
            Eigen::Matrix<double, K, K> matA;
            if constexpr (K == Eigen::Dynamic) { matA.resize(dim, dim); }
            matA.setIdentity();

            Est e{v, cov};
            auto r1{e * matA};
            auto r2{e.RightMultiply(matA)};
            for (auto i{0}; i < dim; ++i) {
                CheckClose(r1.Value(i), r2.Value(i),
                           fmt::format("8i: operator*(est, mat) Value({})", i));
            }
        }

        // ---- operator*(matrix, est) (A is (K+1)×K) ----
        {
            constexpr int M{K == Eigen::Dynamic ? Eigen::Dynamic : K + 1};
            constexpr int actualM{(K == Eigen::Dynamic) ? dim + 1 : K + 1};

            Eigen::Matrix<double, M, K> matRect;
            if constexpr (K == Eigen::Dynamic or M == Eigen::Dynamic) {
                matRect.resize(actualM, dim);
            }
            for (auto i{0}; i < actualM; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    matRect(i, j) = (i + 1) * 10 + (j + 1);
                }
            }

            Est e{v, cov};
            auto r1{matRect * e};
            auto r2{e.LeftMultiply(matRect)};
            for (auto j{0}; j < actualM; ++j) {
                CheckClose(r1.Value(j), r2.Value(j),
                           fmt::format("8i: operator*(mat, est) Value({})", j));
            }
        }
    }
}};

// =========================================================================
// Section 8j: Matrix-vector multiply — dimension mismatch throws
// =========================================================================

constexpr auto sec8jRightMultiplyException{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{v, cov};

        constexpr bool isDynamic{K == Eigen::Dynamic};

        // Row mismatch on RightMultiply
        if constexpr (isDynamic) {
            auto threw{false};
            try {
                Eigen::MatrixXd bad{2, 2};
                bad.setIdentity();
                e.RightMultiply(bad);
            } catch (const std::invalid_argument&) { threw = true; }
            if (not threw) {
                Throw<std::runtime_error>("8j: RightMultiply row mismatch should throw");
            }

            threw = false;
            try {
                Eigen::MatrixXd bad{3, 2};
                bad.setIdentity();
                e.LeftMultiply(bad);
            } catch (const std::invalid_argument&) { threw = true; }
            if (not threw) {
                Throw<std::runtime_error>("8j: LeftMultiply col mismatch should throw");
            }
        }
    }
}};

// =========================================================================
// Section 8k: Matrix-vector LeftMultiply — square and non-square
// =========================================================================

constexpr auto sec8kLeftMultiply{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};

        // ---- LeftMultiply with square matrix ----
        {
            // construct anti-diagonal matrix A: A(i,j) = (i == dim-1-j) ? 2.0 : 0.0
            Eigen::Matrix<double, K, K> matA;
            if constexpr (K == Eigen::Dynamic) {
                matA.resize(dim, dim);
            }
            matA.setZero();
            for (auto i{0}; i < dim; ++i) {
                matA(i, dim - 1 - i) = 2.0;
            }

            // expected value = A * v
            Eigen::Vector<double, K> expectedValue;
            if constexpr (K == Eigen::Dynamic) { expectedValue.resize(dim); }
            expectedValue.setZero();
            for (auto i{0}; i < dim; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    expectedValue(i) += matA(i, j) * v(j);
                }
            }

            Est e{v, cov};
            auto result{e.LeftMultiply(matA)};

            for (auto i{0}; i < dim; ++i) {
                CheckClose(result.Value(i), expectedValue(i),
                           fmt::format("8k: LeftMultiply sq Value({})", i));
            }

            // expected covariance = A * Cov * A^T
            for (auto i{0}; i < dim; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    auto expectedCov{0.0};
                    for (auto p{0}; p < dim; ++p) {
                        for (auto q{0}; q < dim; ++q) {
                            auto cov_pq{[&] {
                                if constexpr (C == CovarianceOption::Full) {
                                    return cov(p, q);
                                } else {
                                    return (p == q) ? cov.diagonal()(p) : 0.0;
                                }
                            }()};
                            expectedCov += matA(i, p) * cov_pq * matA(j, q);
                        }
                    }
                    if constexpr (C == CovarianceOption::Full) {
                        CheckClose(result.Covariance(i, j), expectedCov,
                                   fmt::format("8k: LeftMultiply sq Cov({},{})", i, j));
                    } else {
                        if (i == j) {
                            CheckClose(result.Covariance(i, j), expectedCov,
                                       fmt::format("8k: LeftMultiply sq Var({})", i));
                        }
                    }
                }
            }
        }

        // ---- LeftMultiply with non-square matrix ((K+1) × K) ----
        {
            constexpr int M{K == Eigen::Dynamic ? Eigen::Dynamic : K + 1};
            constexpr int actualM{(K == Eigen::Dynamic) ? dim + 1 : K + 1};

            Eigen::Matrix<double, M, K> matRect;
            if constexpr (K == Eigen::Dynamic or M == Eigen::Dynamic) {
                matRect.resize(actualM, dim);
            }
            for (auto i{0}; i < actualM; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    matRect(i, j) = (i + 1) * 10 + (j + 1);
                }
            }

            Est e{v, cov};
            auto result{e.LeftMultiply(matRect)};
            CheckEq(result.Dimension(), actualM, "8k: LeftMultiply non-sq dim");

            // expected value = A * v
            for (auto i{0}; i < actualM; ++i) {
                auto expectedVal{0.0};
                for (auto j{0}; j < dim; ++j) {
                    expectedVal += matRect(i, j) * v(j);
                }
                CheckClose(result.Value(i), expectedVal,
                           fmt::format("8k: LeftMultiply non-sq Value({})", i));
            }

            // expected covariance = A * Cov * A^T
            for (auto i1{0}; i1 < actualM; ++i1) {
                for (auto i2{0}; i2 < actualM; ++i2) {
                    auto expectedCov{0.0};
                    for (auto p{0}; p < dim; ++p) {
                        for (auto q{0}; q < dim; ++q) {
                            auto cov_pq{[&] {
                                if constexpr (C == CovarianceOption::Full) {
                                    return cov(p, q);
                                } else {
                                    return (p == q) ? cov.diagonal()(p) : 0.0;
                                }
                            }()};
                            expectedCov += matRect(i1, p) * cov_pq * matRect(i2, q);
                        }
                    }
                    if constexpr (C == CovarianceOption::Full) {
                        CheckClose(result.Covariance(i1, i2), expectedCov,
                                   fmt::format("8k: LeftMultiply non-sq Cov({},{})", i1, i2));
                    } else {
                        if (i1 == i2) {
                            CheckClose(result.Covariance(i1, i2), expectedCov,
                                       fmt::format("8k: LeftMultiply non-sq Var({})", i1));
                        }
                    }
                }
            }
        }
    }
}};

// =========================================================================
// Section 9: Algebraic/Power Math Functions
// =========================================================================

// Helper: check element-wise Jacobian-based covariance for a math function
template<int K, CovarianceOption C, typename Func>
auto CheckMathFunction(
    const Estimate<K, C>& e,
    const auto& cov,
    const std::string& name,
    Func jacobian) -> void {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    constexpr bool isFull{C == CovarianceOption::Full};
    for (auto i{0}; i < dim; ++i) {
        for (auto j{0}; j < dim; ++j) {
            double cov_ij;
            if constexpr (isFull) {
                cov_ij = cov(i, j);
            } else {
                cov_ij = (i == j) ? cov.diagonal()(i) : 0.0;
            }
            auto expectedCov{jacobian(i) * cov_ij * jacobian(j)};
            CheckClose(e.Covariance(i, j), expectedCov,
                       fmt::format("{}: Cov({},{})", name, i, j));
        }
    }
}

// Helper: check element-wise binary Jacobian-based covariance
// Cov_result(i,j) = jacX(i)*covX(i,j)*jacX(j) + jacY(i)*covY(i,j)*jacY(j)
template<int K, CovarianceOption C, typename JacXFunc, typename JacYFunc>
auto CheckBinaryCov(
    const Estimate<K, C>& e,
    const auto& covX,
    const auto& covY,
    const std::string& name,
    JacXFunc jacX,
    JacYFunc jacY) -> void {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    constexpr bool isFull{C == CovarianceOption::Full};
    for (auto i{0}; i < dim; ++i) {
        for (auto j{0}; j < dim; ++j) {
            double covX_ij;
            double covY_ij;
            if constexpr (isFull) {
                covX_ij = covX(i, j);
                covY_ij = covY(i, j);
            } else {
                covX_ij = (i == j) ? covX.diagonal()(i) : 0.0;
                covY_ij = (i == j) ? covY.diagonal()(i) : 0.0;
            }
            auto expectedCov{jacX(i) * covX_ij * jacX(j) + jacY(i) * covY_ij * jacY(j)};
            CheckClose(e.Covariance(i, j), expectedCov,
                       fmt::format("{}: Cov({},{})", name, i, j));
        }
    }
}

constexpr auto sec9Algebraic{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    // --- Square: f(x)=x^2, J=diag(2x) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.SquareInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), x(0) * x(0), "9a: Square value");
            CheckClose(e.Variance(), 4.0 * x(0) * x(0) * cov.diagonal()(0), "9a: Square var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), x(i) * x(i), fmt::format("9a: Square value({})", i));
            }
            CheckMathFunction(e, cov, "9a: Square",
                              [&](int i) { return 2.0 * x(i); });
        }

        const auto e2{square(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), x(0) * x(0), "9b: square() free value");
        } else {
            CheckClose(e2.Value(0), x(0) * x(0), "9b: square() free value");
        }
    }

    // --- Cube: f(x)=x^3, J=diag(3x^2) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.CubeInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), x(0) * x(0) * x(0), "9c: Cube value");
            CheckClose(e.Variance(), 9.0 * x(0) * x(0) * x(0) * x(0) * cov.diagonal()(0), "9c: Cube var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), x(i) * x(i) * x(i), fmt::format("9c: Cube value({})", i));
            }
            CheckMathFunction(e, cov, "9c: Cube",
                              [&](int i) { return 3.0 * x(i) * x(i); });
        }

        const auto e2{cube(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), x(0) * x(0) * x(0), "9d: cube() free value");
        } else {
            CheckClose(e2.Value(0), x(0) * x(0) * x(0), "9d: cube() free value");
        }
    }

    // --- Sqrt: f(x)=sqrt(x), J=diag(1/(2*sqrt(x))) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.SqrtInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::sqrt(x(0)), "9e: Sqrt value");
            CheckClose(e.Variance(), 0.25 / x(0) * cov.diagonal()(0), "9e: Sqrt var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::sqrt(x(i)), fmt::format("9e: Sqrt value({})", i));
            }
            CheckMathFunction(e, cov, "9e: Sqrt",
                              [&](int i) { return 0.5 / std::sqrt(x(i)); });
        }

        const auto e2{sqrt(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), std::sqrt(x(0)), "9f: sqrt() free value");
        } else {
            CheckClose(e2.Value(0), std::sqrt(x(0)), "9f: sqrt() free value");
        }
    }

    // --- Cbrt: f(x)=cbrt(x), J=diag(1/(3*cbrt(x)^2)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.CbrtInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::cbrt(x(0)), "9g: Cbrt value");
            auto jac{1.0 / (3.0 * std::pow(std::cbrt(x(0)), 2))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "9g: Cbrt var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::cbrt(x(i)), fmt::format("9g: Cbrt value({})", i));
            }
            CheckMathFunction(e, cov, "9g: Cbrt",
                              [&](int i) { return 1.0 / (3.0 * std::pow(std::cbrt(x(i)), 2)); });
        }

        const auto e2{cbrt(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), std::cbrt(x(0)), "9h: cbrt() free value");
        } else {
            CheckClose(e2.Value(0), std::cbrt(x(0)), "9h: cbrt() free value");
        }
    }

    // --- Rsqrt: f(x)=1/sqrt(x), J=diag(-1/(2*x^(3/2))) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.RsqrtInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), 1.0 / std::sqrt(x(0)), "9i: Rsqrt value");
            auto jac{-0.5 / (x(0) * std::sqrt(x(0)))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "9i: Rsqrt var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), 1.0 / std::sqrt(x(i)), fmt::format("9i: Rsqrt value({})", i));
            }
            CheckMathFunction(e, cov, "9i: Rsqrt",
                              [&](int i) { return -0.5 / (x(i) * std::sqrt(x(i))); });
        }

        const auto e2{rsqrt(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), 1.0 / std::sqrt(x(0)), "9j: rsqrt() free value");
        } else {
            CheckClose(e2.Value(0), 1.0 / std::sqrt(x(0)), "9j: rsqrt() free value");
        }
    }

    // --- Inverse: f(x)=1/x, J=diag(-1/x^2) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.InverseInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), 1.0 / x(0), "9k: Inverse value");
            CheckClose(e.Variance(), 1.0 / (x(0) * x(0) * x(0) * x(0)) * cov.diagonal()(0), "9k: Inverse var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), 1.0 / x(i), fmt::format("9k: Inverse value({})", i));
            }
            CheckMathFunction(e, cov, "9k: Inverse",
                              [&](int i) { return -1.0 / (x(i) * x(i)); });
        }

        const auto e2{inverse(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), 1.0 / x(0), "9l: inverse() free value");
        } else {
            CheckClose(e2.Value(0), 1.0 / x(0), "9l: inverse() free value");
        }
    }
}};

// =========================================================================
// Section 9b: Abs (element-wise absolute value)
// =========================================================================
constexpr auto sec9bAbs{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    // --- Abs: f(x)=|x|, J=diag(sign(x)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.AbsInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::abs(x(0)), "9b_a: Abs value");
            CheckClose(e.Variance(), cov.diagonal()(0), "9b_a: Abs var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::abs(x(i)), fmt::format("9b_a: Abs value({})", i));
            }
            CheckMathFunction(e, cov, "9b_a: Abs",
                              [&](int i) { return x(i) > 0 ? 1.0 : (x(i) < 0 ? -1.0 : 0.0); });
        }

        const auto e2{abs(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), std::abs(x(0)), "9b_b: abs() free value");
        } else {
            CheckClose(e2.Value(0), std::abs(x(0)), "9b_b: abs() free value");
        }
    }
}};

// =========================================================================
// Section 10: Exponential Functions
// =========================================================================
constexpr auto sec10Exponential{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    // --- Exp: f(x)=exp(x), J=diag(exp(x)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.ExpInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::exp(x(0)), "10a: Exp value");
            auto jac{std::exp(x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "10a: Exp var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::exp(x(i)), fmt::format("10a: Exp value({})", i));
            }
            CheckMathFunction(e, cov, "10a: Exp",
                              [&](int i) { return std::exp(x(i)); });
        }

        const auto e2{exp(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), std::exp(x(0)), "10b: exp() free value");
        } else {
            CheckClose(e2.Value(0), std::exp(x(0)), "10b: exp() free value");
        }
    }

    // --- Exp2: f(x)=2^x, J=diag(ln2*2^x) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.Exp2InPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::exp2(x(0)), "10c: Exp2 value");
            auto jac{std::log(2.0) * std::exp2(x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "10c: Exp2 var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::exp2(x(i)), fmt::format("10c: Exp2 value({})", i));
            }
            CheckMathFunction(e, cov, "10c: Exp2",
                              [&](int i) { return std::log(2.0) * std::exp2(x(i)); });
        }

        const auto e2{exp2(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), std::exp2(x(0)), "10d: exp2() free value");
        } else if constexpr (dim >= 2) {
            CheckClose(e2.Value(1), std::exp2(x(1)), "10d: exp2() free value");
        }
    }

    // --- Expm1: f(x)=exp(x)-1, J=diag(exp(x)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.Expm1InPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::expm1(x(0)), "10e: Expm1 value");
            auto jac{std::exp(x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "10e: Expm1 var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::expm1(x(i)), fmt::format("10e: Expm1 value({})", i));
            }
            CheckMathFunction(e, cov, "10e: Expm1",
                              [&](int i) { return std::exp(x(i)); });
        }

        const auto e2{expm1(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), std::expm1(x(0)), "10f: expm1() free value");
        } else if constexpr (dim >= 2) {
            CheckClose(e2.Value(1), std::expm1(x(1)), "10f: expm1() free value");
        }
    }
}};

// =========================================================================
// Section 11: Logarithmic Functions
// =========================================================================
constexpr auto sec11Logarithmic{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    // --- Log: f(x)=ln(x), J=diag(1/x) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.LogInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::log(x(0)), "11a: Log value");
            CheckClose(e.Variance(), 1.0 / (x(0) * x(0)) * cov.diagonal()(0), "11a: Log var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::log(x(i)), fmt::format("11a: Log value({})", i));
            }
            CheckMathFunction(e, cov, "11a: Log",
                              [&](int i) { return 1.0 / x(i); });
        }
    }

    // --- Log10: f(x)=log10(x), J=diag(1/(x*ln10)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.Log10InPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::log10(x(0)), "11c: Log10 value");
            auto jac{1.0 / (x(0) * std::log(10.0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "11c: Log10 var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::log10(x(i)), fmt::format("11c: Log10 value({})", i));
            }
            CheckMathFunction(e, cov, "11c: Log10",
                              [&](int i) { return 1.0 / (x(i) * std::log(10.0)); });
        }
    }

    // --- Log2: f(x)=log2(x), J=diag(1/(x*ln2)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.Log2InPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::log2(x(0)), "11e: Log2 value");
            auto jac{1.0 / (x(0) * std::log(2.0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "11e: Log2 var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::log2(x(i)), fmt::format("11e: Log2 value({})", i));
            }
            CheckMathFunction(e, cov, "11e: Log2",
                              [&](int i) { return 1.0 / (x(i) * std::log(2.0)); });
        }

        const auto e2{log2(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), std::log2(x(0)), "11f: log2() free value");
        } else if constexpr (dim >= 2) {
            CheckClose(e2.Value(1), 1.0, "11f: log2() free value");
        }
    }

    // --- Log1p: f(x)=ln(1+x), J=diag(1/(1+x)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.Log1pInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::log1p(x(0)), "11g: Log1p value");
            auto jac{1.0 / (1.0 + x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "11g: Log1p var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::log1p(x(i)), fmt::format("11g: Log1p value({})", i));
            }
            CheckMathFunction(e, cov, "11g: Log1p",
                              [&](int i) { return 1.0 / (1.0 + x(i)); });
        }

        const auto e2{log1p(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), std::log1p(x(0)), "11h: log1p() free value");
        } else {
            CheckClose(e2.Value(0), std::log1p(x(0)), "11h: log1p() free value");
        }
    }
}};

// =========================================================================
// Section 12: Trigonometric Functions
// =========================================================================
constexpr auto sec12Trigonometric{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    // --- Sin: f(x)=sin(x), J=diag(cos(x)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.SinInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::sin(x(0)), "12a: Sin value");
            auto jac{std::cos(x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "12a: Sin var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::sin(x(i)), fmt::format("12a: Sin value({})", i));
            }
            CheckMathFunction(e, cov, "12a: Sin",
                              [&](int i) { return std::cos(x(i)); });
        }
    }

    // --- Cos: f(x)=cos(x), J=diag(-sin(x)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.CosInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::cos(x(0)), "12c: Cos value");
            auto jac{-std::sin(x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "12c: Cos var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::cos(x(i)), fmt::format("12c: Cos value({})", i));
            }
            CheckMathFunction(e, cov, "12c: Cos",
                              [&](int i) { return -std::sin(x(i)); });
        }
    }

    // --- Tan: f(x)=tan(x), J=diag(1/cos^2(x)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.TanInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::tan(x(0)), "12e: Tan value");
            auto c{std::cos(x(0))};
            auto jac{1.0 / (c * c)};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "12e: Tan var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::tan(x(i)), fmt::format("12e: Tan value({})", i));
            }
            CheckMathFunction(e, cov, "12e: Tan",
                              [&](int i) { auto c{std::cos(x(i))}; return 1.0 / (c * c); });
        }
    }
}};

// =========================================================================
// Section 13: Inverse Trigonometric Functions
// =========================================================================
constexpr auto sec13InverseTrig{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    // --- Asin: f(x)=asin(x), J=diag(1/sqrt(1-x^2)) ---
    {
        const Eigen::Vector<double, K> x{MakeTestValue<K, C>(dim) * 0.01};
        const auto cov{EvalCov<K, C>(MakeTestCov<K, C>(dim) * muc::pow(0.01, 2))};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.AsinInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::asin(x(0)), "13a: Asin value");
            auto jac{1.0 / std::sqrt(1.0 - x(0) * x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "13a: Asin var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::asin(x(i)), fmt::format("13a: Asin value({})", i));
            }
            CheckMathFunction(e, cov, "13a: Asin",
                              [&](int i) { return 1.0 / std::sqrt(1.0 - x(i) * x(i)); });
        }
    }

    // --- Acos: f(x)=acos(x), J=diag(-1/sqrt(1-x^2)) ---
    {
        const Eigen::Vector<double, K> x{MakeTestValue<K, C>(dim) * 0.01};
        const auto cov{EvalCov<K, C>(MakeTestCov<K, C>(dim) * muc::pow(0.01, 2))};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.AcosInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::acos(x(0)), "13c: Acos value");
            auto jac{-1.0 / std::sqrt(1.0 - x(0) * x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "13c: Acos var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::acos(x(i)), fmt::format("13c: Acos value({})", i));
            }
            CheckMathFunction(e, cov, "13c: Acos",
                              [&](int i) { return -1.0 / std::sqrt(1.0 - x(i) * x(i)); });
        }

        const auto e2{acos(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), std::acos(x(0)), "13d: acos() free value");
        } else {
            CheckClose(e2.Value(0), std::acos(x(0)), "13d: acos() free value");
        }
    }

    // --- Atan: f(x)=atan(x), J=diag(1/(1+x^2)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.AtanInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::atan(x(0)), "13e: Atan value");
            auto jac{1.0 / (1.0 + x(0) * x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "13e: Atan var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::atan(x(i)), fmt::format("13e: Atan value({})", i));
            }
            CheckMathFunction(e, cov, "13e: Atan",
                              [&](int i) { return 1.0 / (1.0 + x(i) * x(i)); });
        }
    }
}};

// =========================================================================
// Section 14: Hyperbolic Functions
// =========================================================================
constexpr auto sec14Hyperbolic{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    // --- Sinh: f(x)=sinh(x), J=diag(cosh(x)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.SinhInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::sinh(x(0)), "14a: Sinh value");
            auto jac{std::cosh(x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "14a: Sinh var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::sinh(x(i)), fmt::format("14a: Sinh value({})", i));
            }
            CheckMathFunction(e, cov, "14a: Sinh",
                              [&](int i) { return std::cosh(x(i)); });
        }
    }

    // --- Cosh: f(x)=cosh(x), J=diag(sinh(x)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.CoshInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::cosh(x(0)), "14c: Cosh value");
            auto jac{std::sinh(x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "14c: Cosh var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::cosh(x(i)), fmt::format("14c: Cosh value({})", i));
            }
            CheckMathFunction(e, cov, "14c: Cosh",
                              [&](int i) { return std::sinh(x(i)); });
        }
    }

    // --- Tanh: f(x)=tanh(x), J=diag(1/cosh^2(x)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.TanhInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::tanh(x(0)), "14e: Tanh value");
            auto c{std::cosh(x(0))};
            auto jac{1.0 / (c * c)};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "14e: Tanh var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::tanh(x(i)), fmt::format("14e: Tanh value({})", i));
            }
            CheckMathFunction(e, cov, "14e: Tanh",
                              [&](int i) { auto c{std::cosh(x(i))}; return 1.0 / (c * c); });
        }

        const auto e2{tanh(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), std::tanh(x(0)), "14f: tanh() free value");
        } else {
            CheckClose(e2.Value(0), std::tanh(x(0)), "14f: tanh() free value");
        }
    }
}};

// =========================================================================
// Section 15: Inverse Hyperbolic Functions
// =========================================================================
constexpr auto sec15InverseHyperbolic{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    // --- Asinh: f(x)=asinh(x), J=diag(1/sqrt(1+x^2)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.AsinhInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::asinh(x(0)), "15a: Asinh value");
            auto jac{1.0 / std::sqrt(1.0 + x(0) * x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "15a: Asinh var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::asinh(x(i)), fmt::format("15a: Asinh value({})", i));
            }
            CheckMathFunction(e, cov, "15a: Asinh",
                              [&](int i) { return 1.0 / std::sqrt(1.0 + x(i) * x(i)); });
        }
    }

    // --- Acosh: f(x)=acosh(x), J=diag(1/sqrt(x^2-1)), requires x>1 ---
    {
        const Eigen::Vector<double, K> x{MakeTestValue<K, C>(dim) * 1.1};
        const auto cov{EvalCov<K, C>(MakeTestCov<K, C>(dim) * muc::pow(1.1, 2))};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.AcoshInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::acosh(x(0)), "15c: Acosh value");
            auto jac{1.0 / std::sqrt(x(0) * x(0) - 1.0)};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "15c: Acosh var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::acosh(x(i)), fmt::format("15c: Acosh value({})", i));
            }
            CheckMathFunction(e, cov, "15c: Acosh",
                              [&](int i) { return 1.0 / std::sqrt(x(i) * x(i) - 1.0); });
        }
    }

    // --- Atanh: f(x)=atanh(x), J=diag(1/(1-x^2)), requires |x|<1 ---
    {
        const Eigen::Vector<double, K> x{MakeTestValue<K, C>(dim) * 0.01};
        const auto cov{EvalCov<K, C>(MakeTestCov<K, C>(dim) * muc::pow(0.01, 2))};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.AtanhInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::atanh(x(0)), "15e: Atanh value");
            auto jac{1.0 / (1.0 - x(0) * x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "15e: Atanh var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::atanh(x(i)), fmt::format("15e: Atanh value({})", i));
            }
            CheckMathFunction(e, cov, "15e: Atanh",
                              [&](int i) { return 1.0 / (1.0 - x(i) * x(i)); });
        }
    }
}};

// =========================================================================
// Section 16: Special Functions
// =========================================================================
constexpr auto sec16Special{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    // --- Logistic: f(x)=1/(1+exp(-x)), J=diag(f*(1-f)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.LogisticInPlace();
        if constexpr (K == 1) {
            auto expected{1.0 / (1.0 + std::exp(-x(0)))};
            CheckClose(e.Value(), expected, "16a: Logistic value");
            auto jac{expected * (1.0 - expected)};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "16a: Logistic var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                auto expected{1.0 / (1.0 + std::exp(-x(i)))};
                CheckClose(e.Value(i), expected, fmt::format("16a: Logistic value({})", i));
            }
            CheckMathFunction(e, cov, "16a: Logistic",
                              [&](int i) { auto y{1.0 / (1.0 + std::exp(-x(i)))}; return y * (1.0 - y); });
        }

        const auto eLogistic{logistic(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(eLogistic.Value(), 1.0 / (1.0 + std::exp(-x(0))), "16b: logistic() free value");
        } else {
            CheckClose(eLogistic.Value(0), 1.0 / (1.0 + std::exp(-x(0))), "16b: logistic() free value");
        }
    }

    // --- Erf: f(x)=erf(x), J=diag(2/sqrt(pi)*exp(-x^2)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.ErfInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::erf(x(0)), "16c: Erf value");
            auto jac{2.0 / std::sqrt(std::numbers::pi) * std::exp(-x(0) * x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "16c: Erf var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::erf(x(i)), fmt::format("16c: Erf value({})", i));
            }
            CheckMathFunction(e, cov, "16c: Erf",
                              [&](int i) { return 2.0 / std::sqrt(std::numbers::pi) * std::exp(-x(i) * x(i)); });
        }

        const auto eErf{erf(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(eErf.Value(), std::erf(x(0)), "16d: erf() free value");
        } else {
            CheckClose(eErf.Value(0), std::erf(x(0)), "16d: erf() free value");
        }
    }

    // --- Erfc: f(x)=erfc(x), J=diag(-2/sqrt(pi)*exp(-x²)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.ErfcInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::erfc(x(0)), "16e: Erfc value");
            auto jac{-2.0 / std::sqrt(std::numbers::pi) * std::exp(-x(0) * x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "16e: Erfc var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::erfc(x(i)), fmt::format("16e: Erfc value({})", i));
            }
            CheckMathFunction(e, cov, "16e: Erfc",
                              [&](int i) { return -2.0 / std::sqrt(std::numbers::pi) * std::exp(-x(i) * x(i)); });
        }

        const auto eErfc{erfc(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(eErfc.Value(), std::erfc(x(0)), "16f: erfc() free value");
        } else {
            CheckClose(eErfc.Value(0), std::erfc(x(0)), "16f: erfc() free value");
        }
    }

    // --- Lgamma ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.LgammaInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), 0.0, "16g: Lgamma(1)=0");
        } else {
            CheckClose(e.Value(0), 0.0, "16g: Lgamma(1)=0");
        }

        const auto eLgamma{lgamma(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(eLgamma.Value(), 0.0, "16h: lgamma() free value");
        } else {
            CheckClose(eLgamma.Value(0), 0.0, "16h: lgamma() free value");
        }
    }

    // --- Ndtri: f(x)=Phi^{-1}(x), J=diag(sqrt(2*pi)*exp(Phi^{-1}(x)^2/2)) ---
    {
        decltype(MakeTestValue<K, C>(dim)) x;
        if constexpr (K == Eigen::Dynamic) {
            x = Eigen::VectorXd::Constant(dim, 0.5);
        } else {
            x = Eigen::Vector<double, K>::Constant(0.5);
        }
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.NdtriInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), 0.0, "16i: Ndtri(0.5)=0");
            auto jac{std::sqrt(2.0 * std::numbers::pi)};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "16i: Ndtri var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), 0.0, fmt::format("16i: Ndtri(0.5)=0 at ({})", i));
            }
            CheckMathFunction(e, cov, "16i: Ndtri",
                              [](int) { return std::sqrt(2.0 * std::numbers::pi); });
        }

        const auto e2{ndtri(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), 0.0, "16j: ndtri() free value");
        } else {
            CheckClose(e2.Value(0), 0.0, "16j: ndtri() free value");
        }
    }
}};

// =========================================================================
// Section 17: Negate and Chaining
// =========================================================================
constexpr auto sec17NegateChaining{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    constexpr bool isFull{C == CovarianceOption::Full};

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    // Negate() in-place
    {
        auto e{MakeEstimate<K, C>(x, cov)};
        e.NegateInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), -x(0), "17a: Negate value");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), -x(i), fmt::format("17a: Negate value({})", i));
            }
        }
        if constexpr (isFull and K != 1) {
            CheckClose(e.Covariance(), cov, "17a: Negate cov unchanged");
        }
    }

    // Unary operator-
    {
        auto e{MakeEstimate<K, C>(x, cov)};
        const auto e2{-e};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), -x(0), "17b: unary- value");
            CheckClose(e.Value(), x(0), "17b: original unchanged");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e2.Value(i), -x(i), fmt::format("17b: unary- value({})", i));
            }
            CheckClose(e.Value(0), x(0), "17b: original unchanged");
        }
    }

    // Chaining: Square().Sqrt() roundtrip
    {
        auto e{MakeEstimate<K, C>(x, cov)};
        e.SquareInPlace().SqrtInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::abs(x(0)), "17c: Square().Sqrt() value");
        } else {
            for (auto i{0}; i < std::min(dim, 2); ++i) {
                CheckClose(e.Value(i), std::abs(x(i)), fmt::format("17c: Square().Sqrt() value({})", i));
            }
        }
    }

    // Exp().Log() roundtrip
    {
        auto e{MakeEstimate<K, C>(x, cov)};
        e.ExpInPlace().LogInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), x(0), "17d: Exp().Log() value");
        } else {
            for (auto i{0}; i < std::min(dim, 2); ++i) {
                CheckClose(e.Value(i), x(i), fmt::format("17d: Exp().Log() value({})", i));
            }
        }
    }

    // Move semantics
    {
        auto e{MakeEstimate<K, C>(x, cov)};
        const auto e2{std::move(e).Square()};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), x(0) * x(0), "17e: move+Square value");
        } else {
            CheckClose(e2.Value(0), x(0) * x(0), "17e: move+Square value(0)");
            if constexpr (dim >= 2) {
                CheckClose(e2.Value(1), x(1) * x(1), "17e: move+Square value(1)");
            }
        }
    }
}};

// =========================================================================
// Section 18: ToPOD / FromPOD / Base64
// Static K only — ToPOD requires compile-time dimension
// =========================================================================
constexpr auto sec18POD{[]<int K, CovarianceOption C>() {
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
        CheckClose(e2.Value(), e1.Value(), "18: POD roundtrip Value");
        if constexpr (K != 1) {
            CheckClose(e2.Covariance(), e1.Covariance(), "18: POD roundtrip Cov");
        }
        if constexpr (K == 1) {
            CheckClose(e2.StdDev(), e1.StdDev(), "18: POD roundtrip StdDev");
            CheckClose(e2.Variance(), e1.Variance(), "18: POD roundtrip Variance");
        } else {
            CheckClose(e2.StdDev(0), e1.StdDev(0), "18: POD roundtrip StdDev");
            if constexpr (dim >= 2) {
                CheckClose(e2.Variance(1), e1.Variance(1), "18: POD roundtrip Variance");
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
            CheckClose(e2.Value(), x(0), "18: FromPOD into existing Value()");
        } else {
            CheckClose(e2.Value(0), x(0), "18: FromPOD into existing Value(0)");
            if constexpr (dim >= 2) {
                CheckClose(e2.Covariance(1, 1), e1.Covariance(1, 1), "18: FromPOD into existing Cov(1,1)");
            }
        }
    }

    // Base64 encode/decode roundtrip
    {
        auto e1{MakeEstimate<K, C>(x, cov)};
        const auto pod{e1.ToPOD()};
        const auto b64{ToBase64(pod)};
        const auto decoded{FromBase64<EstimatePOD<K, C>>(b64)};
        CheckClose(decoded.x[0], pod.x[0], "18: Base64 x[0] roundtrip");
        if constexpr (dim >= 2) {
            CheckClose(decoded.x[1], pod.x[1], "18: Base64 x[1] roundtrip");
        }

        Est e2{decoded};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), e1.Value(), "18: Base64 Estimate Value()");
        } else {
            CheckClose(e2.Value(0), e1.Value(0), "18: Base64 Estimate Value(0)");
            if constexpr (dim >= 2) {
                CheckClose(e2.Covariance(0, 1), e1.Covariance(0, 1), "18: Base64 Estimate Cov(0,1)");
            }
        }
    }

    // DecodeBase64 method
    {
        auto e1{MakeEstimate<K, C>(x, cov)};
        const auto pod{e1.ToPOD()};
        const auto b64{ToBase64(pod)};
        const auto decoded{FromBase64<EstimatePOD<K, C>>(b64)};
        CheckClose(decoded.x[0], x(0), "18: DecodeBase64 x[0]");
    }
}};

// =========================================================================
// Section 18: Invalid Base64 Throws
// =========================================================================
constexpr auto sec18InvalidBase64{[] {
    auto threw{false};
    try {
        FromBase64<EstimatePOD<2, CovarianceOption::Full>>("!!!not_valid_base64!!!");
    } catch (const std::runtime_error&) {
        threw = true;
    }
    if (not threw) {
        Throw<std::runtime_error>("18f: invalid Base64 should throw");
    }
}};

// =========================================================================
// Section 19: Edge Cases and Numerical Stress
// =========================================================================
constexpr auto sec19EdgeCases{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    constexpr bool isFull{C == CovarianceOption::Full};

    // Self-assignment safety
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        auto& eRef{e};
        e = eRef;
        e = std::move(eRef);
        if constexpr (K == 1) {
            CheckClose(e.Value(), x(0), "19: Self-assign Value()");
        } else {
            CheckClose(e.Value(0), x(0), "19: Self-assign Value(0)");
            if constexpr (isFull and dim >= 2) {
                CheckClose(e.Covariance(0, 1), cov(0, 1), "19: Self-assign Cov(0,1)");
            }
        }
    }

    // Zero-value edge case: RelativeUncertainty
    //    x=0, var>0 → RelUnc = StdDev/|0| = inf
    //    x=0, var=0 → RelUnc = 0/0 = NaN
    {
        if constexpr (K == 1) {
            // x=0, var>0
            {
                Estimate<K, C> eZero{0.0, 4.0};
                if (not std::isinf(eZero.RelativeUncertainty())) {
                    Throw<std::runtime_error>("19: RelUnc(0,var>0) should be inf for K=1");
                }
            }
            // x=0, var=0
            {
                Estimate<K, C> eZeroBoth;
                if (not std::isnan(eZeroBoth.RelativeUncertainty())) {
                    Throw<std::runtime_error>("19: RelUnc(0,var=0) should be NaN for K=1");
                }
            }
        } else {
            Eigen::Vector<double, K> xZero;
            if constexpr (K == Eigen::Dynamic) {
                xZero.resize(dim);
            }
            xZero.setZero();
            // x=0, var>0: use normal cov
            {
                const auto cov{MakeTestCov<K, C>(dim)};
                auto eZero{MakeEstimate<K, C>(xZero, cov)};
                if (not std::isinf(eZero.RelativeUncertainty(0))) {
                    Throw<std::runtime_error>("19: RelUnc(0,var>0) should be inf");
                }
                // other components also inf since all x=0
                if constexpr (dim >= 2) {
                    if (not std::isinf(eZero.RelativeUncertainty(1))) {
                        Throw<std::runtime_error>("19: RelUnc(1,var>0) should be inf");
                    }
                }
            }
            // x=0, var=0: zero cov
            {
                decltype(MakeTestCov<K, C>(dim)) covZero;
                if constexpr (K == Eigen::Dynamic) {
                    if constexpr (isFull) {
                        covZero.resize(dim, dim);
                    } else {
                        covZero.resize(dim);
                    }
                }
                covZero.setZero();
                auto eZeroBoth{MakeEstimate<K, C>(xZero, covZero)};
                if (not std::isnan(eZeroBoth.RelativeUncertainty(0))) {
                    Throw<std::runtime_error>("19: RelUnc(0,var=0) should be NaN");
                }
            }
        }

        // Non-zero value: RelUnc is finite (sanity check)
        {
            const auto x{MakeTestValue<K, C>(dim)};
            const auto cov{MakeTestCov<K, C>(dim)};
            auto e{MakeEstimate<K, C>(x, cov)};
            if constexpr (K == 1) {
                if (not std::isfinite(e.RelativeUncertainty())) {
                    Throw<std::runtime_error>("19: RelUnc should be finite for non-zero value");
                }
            } else {
                if (not std::isfinite(e.RelativeUncertainty(0))) {
                    Throw<std::runtime_error>("19: RelUnc should be finite for non-zero value");
                }
            }
        }
    }
}};

// Exp(Log(x)) roundtrip — Jacobian chain composes to identity
constexpr auto sec19ExpLogRoundtrip{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    auto e{MakeEstimate<K, C>(x, cov)};
    e.LogInPlace();
    e.ExpInPlace();
    if constexpr (K == 1) {
        CheckClose(e.Value(), x(0), "19: Exp(Log(x)) Value");
    } else {
        for (auto i{0}; i < dim; ++i) {
            CheckClose(e.Value(i), x(i), fmt::format("19: Exp(Log(x)) Value({})", i));
        }
    }
}};

// =========================================================================
// Section 19c: Correlation — zero-variance edge case
// =========================================================================
constexpr auto sec19cCorrZeroVar{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    constexpr bool isFull{C == CovarianceOption::Full};

    if constexpr (isFull and K != 1) {
        auto v{MakeTestValue<K, C>(dim)};
        auto cov{MakeTestCov<K, C>(dim)};
        cov.diagonal()(0) = 0.0;
        if constexpr (dim >= 2) {
            cov(0, 1) = 0.0;
            cov(1, 0) = 0.0;
        }
        auto e{MakeEstimate<K, C>(v, cov)};

        CheckClose(e.Correlation(0, 0), 1.0, "19c: Corr(0,0)=1 even Var=0");
        if constexpr (dim >= 2) {
            if (not std::isnan(e.Correlation(0, 1))) {
                Throw<std::runtime_error>("19c: Corr(0,1) should be NaN when Var=0");
            }
            if (not std::isnan(e.Correlation(1, 0))) {
                Throw<std::runtime_error>("19c: Corr(1,0) should be NaN when Var=0");
            }
            CheckClose(e.Correlation(1, 1), 1.0, "19c: Corr(1,1)=1");
        }
    }
}};

// =========================================================================
// Section 20: Cross-Verification
// These are inherently dimension-comparison tests — keep as-is
// =========================================================================
constexpr auto sec20CrossVerification{[] {
    // 20b. Addition result matches manual computation
    {
        Eigen::Vector2d x1{1.0, 2.0};
        Eigen::Vector2d x2{3.0, 4.0};
        Eigen::Matrix2d cov1;
        cov1 << 0.4, 0.1, 0.1, 0.3;
        Eigen::Matrix2d cov2;
        cov2 << 0.9, 0.0, 0.0, 1.6;

        Estimate<2, CovarianceOption::Full> e1{x1, cov1};
        Estimate<2, CovarianceOption::Full> e2{x2, cov2};
        const auto e3{e1 + e2};

        CheckClose(e3.Value(), Eigen::Vector2d{4.0, 6.0}, "20b: manual add value");
        Eigen::Matrix2d expectedCov;
        expectedCov << 1.3, 0.1, 0.1, 1.9;
        CheckClose(e3.Covariance(), expectedCov, "20b: manual add cov");
    }

    // 20c. Dynamic matches static for same K
    {
        Eigen::Vector2d x{1.0, 2.0};
        Eigen::Matrix2d cov;
        cov << 0.4, 0.1, 0.1, 0.3;
        Estimate<2, CovarianceOption::Full> eStat{x, cov};

        Eigen::VectorXd xDyn{2};
        xDyn << 1.0, 2.0;
        Eigen::MatrixXd covDyn{2, 2};
        covDyn << 0.4, 0.1, 0.1, 0.3;
        Estimate<Eigen::Dynamic, CovarianceOption::Full> eDyn{xDyn, covDyn};

        eStat.SquareInPlace();
        eDyn.SquareInPlace();
        CheckClose(eStat.Value(0), eDyn.Value(0), "20c: Static vs Dynamic Square value");
        CheckClose(eStat.Covariance(0, 1), eDyn.Covariance(0, 1), "20c: Static vs Dynamic Square cov");
    }

    // 20d. Diagonal = Full with zero off-diagonals
    {
        Eigen::Vector2d x{2.0, 3.0};
        Eigen::Matrix2d covDiagZero;
        covDiagZero << 0.4, 0.0, 0.0, 0.9;
        Estimate<2, CovarianceOption::Full> eFull{x, covDiagZero};

        Eigen::Vector2d diag{0.4, 0.9};
        Estimate<2, CovarianceOption::Diagonal> eDiag{x, diag.asDiagonal()};

        eFull.SquareInPlace();
        eDiag.SquareInPlace();
        CheckClose(eFull.Value(0), eDiag.Value(0), "20d: Full vs Diag Square value");
        CheckClose(eFull.Variance(0), eDiag.Variance(0), "20d: Full vs Diag Square variance");
        CheckClose(eFull.Covariance(0, 1), 0.0, "20d: Full Cov(0,1)=0");
        CheckClose(eDiag.Covariance(0, 1), 0.0, "20d: Diag Cov(0,1)=0");
    }

    // 20e. Cross-CovarianceOption result type is Full when either is Full
    {
        Eigen::Vector2d x{1.0, 2.0};
        Eigen::Matrix2d covFull;
        covFull << 0.1, 0.0, 0.0, 0.1;
        Estimate<2, CovarianceOption::Full> eFull{x, covFull};

        Eigen::Vector2d diag{0.4, 0.9};
        Estimate<2, CovarianceOption::Diagonal> eDiag{x, diag.asDiagonal()};

        const auto eResult1{eFull + eDiag};
        static_assert(std::is_same_v<std::remove_const_t<decltype(eResult1)>, Estimate<2, CovarianceOption::Full>>);

        const auto eResult2{eDiag + eFull};
        static_assert(std::is_same_v<std::remove_const_t<decltype(eResult2)>, Estimate<2, CovarianceOption::Full>>);
    }
}};

// =========================================================================
// Section 21: Normalize
// =========================================================================
constexpr auto sec21Normalize{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};

        // 21a. Normalized() const& — value has unit norm
        {
            Est e{v, cov};
            const auto result{e.Normalized()};
            CheckClose(result.Value().norm(), 1.0, "21a: Normalized() const& value has unit norm");
        }

        // 21b. Normalized() && — value has unit norm
        {
            Est e{v, cov};
            const auto result{std::move(e).Normalized()};
            CheckClose(result.Value().norm(), 1.0, "21b: Normalized() && value has unit norm");
        }

        // 21c. Normalize() & — in-place yields unit norm
        {
            Est e{v, cov};
            e.Normalize();
            CheckClose(e.Value().norm(), 1.0, "21c: Normalize() & value has unit norm");
        }

        // 21d. const& and && produce equivalent results
        {
            Est e1{v, cov};
            Est e2{v, cov};
            const auto r1{e1.Normalized()};
            const auto r2{std::move(e2).Normalized()};
            CheckClose(r1.Value(), r2.Value(), "21d: const& and && produce same value");
            CheckClose(r1.Covariance(), r2.Covariance(), "21d: const& and && produce same covariance");
        }

        // 21e. Covariance — verify against analytical Jacobian
        {
            Est e{v, cov};
            const auto mu{e.Value()};
            const auto sqNorm{mu.squaredNorm()};
            const auto norm{std::sqrt(sqNorm)};

            // Compute analytical Jacobian jac = (I - mu*mu^T / ||mu||^2) / ||mu||
            if constexpr (K == Eigen::Dynamic) {
                Eigen::MatrixXd jac{Eigen::MatrixXd::Identity(dim, dim) / norm
                                    - mu * mu.transpose() / (sqNorm * norm)};
                Eigen::MatrixXd expectedCov{jac * e.Covariance() * jac};
                const auto result{e.Normalized()};
                if constexpr (C == CovarianceOption::Full) {
                    CheckClose(result.Covariance(), expectedCov,
                               "21e: covariance matches analytical Jacobian (Full)");
                } else {
                    CheckClose(result.Variance(), expectedCov.diagonal(),
                               "21e: covariance matches analytical Jacobian (Diag)");
                }
            } else {
                Eigen::Matrix<double, K, K> jac{
                    decltype(jac)::Identity() / norm - mu * mu.transpose() / (sqNorm * norm)};
                Eigen::Matrix<double, K, K> expectedCov{jac * e.Covariance() * jac};
                const auto result{e.Normalized()};
                if constexpr (C == CovarianceOption::Full) {
                    CheckClose(result.Covariance(), expectedCov,
                               "21e: covariance matches analytical Jacobian (Full)");
                } else {
                    CheckClose(result.Variance(), expectedCov.diagonal(),
                               "21e: covariance matches analytical Jacobian (Diag)");
                }
            }
        }

        // 21f. Near-zero vector — Normalize is a no-op
        {
            auto zeroVec{MakeTestValue<K, C>(dim)};
            zeroVec.setZero();
            Est e{zeroVec, cov};
            const auto origVal{e.Value()};
            const auto origCov{e.Covariance()};
            e.Normalize();
            CheckClose(e.Value(), origVal, "21f: near-zero Normalize() leaves value unchanged");
            CheckClose(e.Covariance(), origCov, "21f: near-zero Normalize() leaves covariance unchanged");
        }

        // 21g. Chaining — Normalize().Sum() works
        {
            Est e{v, cov};
            const auto sum{e.Normalized().Sum()};
            // sum value should be close to sum of unit vector components
            const auto expectedSumVal{v.normalized().sum()};
            CheckClose(sum.Value(), expectedSumVal, "21g: Normalized().Sum() value");
        }
    }
}};

// =========================================================================
// Section 23: Prod
// =========================================================================
constexpr auto sec23Prod{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};

        double expectedValue{1.0};
        for (auto i{0}; i < dim; ++i) { expectedValue *= v(i); }

        // 23a. Prod() const (via lvalue)
        {
            Est e{v, cov};
            const auto result{e.Prod()};
            CheckClose(result.Value(), expectedValue, "23a: Prod() const value");
        }

        // 23b. Prod() and (via rvalue)
        {
            Est e{v, cov};
            const auto result{std::move(e).Prod()};
            CheckClose(result.Value(), expectedValue, "23b: Prod() and value");
        }
    }
}};

// =========================================================================
// Section 24: SquaredNorm
// =========================================================================
constexpr auto sec24SquaredNorm{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};

        double expectedValue{0.0};
        for (auto i{0}; i < dim; ++i) { expectedValue += v(i) * v(i); }

        // 24a. SquaredNorm() const&
        {
            Est e{v, cov};
            const auto result{e.SquaredNorm()};
            CheckClose(result.Value(), expectedValue, "24a: SquaredNorm() const& value");
        }

        // 24b. SquaredNorm() &&
        {
            Est e{v, cov};
            const auto result{std::move(e).SquaredNorm()};
            CheckClose(result.Value(), expectedValue, "24b: SquaredNorm() and value");
        }
    }
}};

// =========================================================================
// Section 25: Norm
// =========================================================================
constexpr auto sec25Norm{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};

        double expectedValue{0.0};
        for (auto i{0}; i < dim; ++i) { expectedValue += v(i) * v(i); }
        expectedValue = std::sqrt(expectedValue);

        // 25a. Norm() const&
        {
            Est e{v, cov};
            const auto result{e.Norm()};
            CheckClose(result.Value(), expectedValue, "25a: Norm() const& value");
        }

        // 25b. Norm() &&
        {
            Est e{v, cov};
            const auto result{std::move(e).Norm()};
            CheckClose(result.Value(), expectedValue, "25b: Norm() and value");
        }

        // 25c. Norm() == SquaredNorm().Sqrt()
        {
            Est e1{v, cov};
            Est e2{v, cov};
            const auto normResult{e1.Norm()};
            auto sqNormResult{std::move(e2).SquaredNorm()};
            sqNormResult.SqrtInPlace();
            CheckClose(normResult.Value(), sqNormResult.Value(), "25c: Norm == SquaredNorm().Sqrt() value");
        }
    }
}};

// =========================================================================
// Section 26: Mean
// =========================================================================
constexpr auto sec26Mean{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{v, cov};

        double expectedValue{0.0};
        for (auto i{0}; i < dim; ++i) { expectedValue += v(i); }
        expectedValue /= dim;

        // 26a. Mean value
        const auto result{e.Mean()};
        CheckClose(result.Value(), expectedValue, "26a: Mean value");

        // 26b. Mean variance == Sum variance / dim^2
        const auto sumResult{e.Sum()};
        CheckClose(result.Variance(), sumResult.Variance() / (dim * dim),
                   "26b: Mean variance = Sum variance / dim^2");
    }
}};

// =========================================================================
// Section 27: HarmonicMean
// =========================================================================
constexpr auto sec27HarmonicMean{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};

        double expectedValue{0.0};
        for (auto i{0}; i < dim; ++i) { expectedValue += 1.0 / v(i); }
        expectedValue = dim / expectedValue;

        // 27a. HarmonicMean() const&
        {
            Est e{v, cov};
            const auto result{e.HarmonicMean()};
            CheckClose(result.Value(), expectedValue, "27a: HarmonicMean() const& value");
        }

        // 27b. HarmonicMean() &&
        {
            Est e{v, cov};
            const auto result{std::move(e).HarmonicMean()};
            CheckClose(result.Value(), expectedValue, "27b: HarmonicMean() and value");
        }
    }
}};

// =========================================================================
// Section 28: GeometricMean
// =========================================================================
constexpr auto sec28GeometricMean{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};

        double expectedValue{1.0};
        for (auto i{0}; i < dim; ++i) { expectedValue *= v(i); }
        expectedValue = std::pow(expectedValue, 1.0 / dim);

        // 28a. GeometricMean() const&
        {
            Est e{v, cov};
            const auto result{e.GeometricMean()};
            CheckClose(result.Value(), expectedValue, "28a: GeometricMean() const& value");
        }

        // 28b. GeometricMean() &&
        {
            Est e{v, cov};
            const auto result{std::move(e).GeometricMean()};
            CheckClose(result.Value(), expectedValue, "28b: GeometricMean() and value");
        }
    }
}};

// =========================================================================
// Section 29: QuadraticMean
// =========================================================================
constexpr auto sec29QuadraticMean{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};

        double expectedValue{0.0};
        for (auto i{0}; i < dim; ++i) { expectedValue += v(i) * v(i); }
        expectedValue = std::sqrt(expectedValue / dim);

        // 29a. QuadraticMean() const&
        {
            Est e{v, cov};
            const auto result{e.QuadraticMean()};
            CheckClose(result.Value(), expectedValue, "29a: QuadraticMean() const& value");
        }

        // 29b. QuadraticMean() &&
        {
            Est e{v, cov};
            const auto result{std::move(e).QuadraticMean()};
            CheckClose(result.Value(), expectedValue, "29b: QuadraticMean() and value");
        }
    }
}};

// =========================================================================
// Section 30: CubicMean
// =========================================================================
constexpr auto sec30CubicMean{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};

        double expectedValue{0.0};
        for (auto i{0}; i < dim; ++i) { expectedValue += v(i) * v(i) * v(i); }
        expectedValue = std::cbrt(expectedValue / dim);

        // 30a. CubicMean() const&
        {
            Est e{v, cov};
            const auto result{e.CubicMean()};
            CheckClose(result.Value(), expectedValue, "30a: CubicMean() const& value");
        }

        // 30b. CubicMean() &&
        {
            Est e{v, cov};
            const auto result{std::move(e).CubicMean()};
            CheckClose(result.Value(), expectedValue, "30b: CubicMean() and value");
        }
    }
}};

// =========================================================================
// Section 31: LpNorm
// =========================================================================
constexpr auto sec31LpNorm{[]<int K, CovarianceOption C>() {
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
            for (auto i{0}; i < dim; ++i) { expected += std::abs(v(i)); }
            CheckClose(result.Value(), expected, "31a: LpNorm<1.0> value");
        }

        // 31b. LpNorm<2.0> — L2 norm = sqrt(sum(x_i^2)), matches Norm()
        {
            Est e1{v, cov};
            Est e2{v, cov};
            const auto lpResult{std::move(e1).template LpNorm<2.0>()};
            const auto normResult{std::move(e2).Norm()};
            CheckClose(lpResult.Value(), normResult.Value(), "31b: LpNorm<2.0> == Norm value");
            CheckClose(lpResult.Variance(), normResult.Variance(), "31b: LpNorm<2.0> == Norm variance");
        }

        // 31c. LpNorm<3.0> — L3 norm
        {
            Est e{v, cov};
            const auto result{std::move(e).template LpNorm<3.0>()};
            double expected{0.0};
            for (auto i{0}; i < dim; ++i) { expected += std::pow(std::abs(v(i)), 3.0); }
            expected = std::cbrt(expected);
            CheckClose(result.Value(), expected, "31c: LpNorm<3.0> value");
        }

        // 31d. LpNorm<4.0> — general case
        {
            Est e{v, cov};
            const auto result{std::move(e).template LpNorm<4.0>()};
            double expected{0.0};
            for (auto i{0}; i < dim; ++i) { expected += std::pow(std::abs(v(i)), 4.0); }
            expected = std::pow(expected, 0.25);
            CheckClose(result.Value(), expected, "31d: LpNorm<4.0> value");
        }

        // 31e. LpNorm const& overload
        {
            Est e{v, cov};
            const auto result{e.template LpNorm<1.5>()};
            double expected{0.0};
            for (auto i{0}; i < dim; ++i) { expected += std::pow(std::abs(v(i)), 1.5); }
            expected = std::pow(expected, 1.0 / 1.5);
            CheckClose(result.Value(), expected, "31e: LpNorm<1.5> const& value");
        }
    }
}};

// =========================================================================
// Section 22: Sum
// =========================================================================
constexpr auto sec22Sum{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{v, cov};

        // 22a. Sum value
        const auto result{e.Sum()};
        double expectedValue{0.0};
        for (auto i{0}; i < dim; ++i) { expectedValue += v(i); }
        CheckClose(result.Value(), expectedValue, "22a: Sum value");

        // 22b. Sum variance
        double expectedVar{0.0};
        if constexpr (C == CovarianceOption::Full) {
            for (auto i{0}; i < dim; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    expectedVar += e.Covariance(i, j);
                }
            }
        } else {
            for (auto i{0}; i < dim; ++i) { expectedVar += e.Variance(i); }
        }
        CheckClose(result.Variance(), expectedVar, "22b: Sum variance");
    }
}};

} // namespace TestEstimateSection

// =========================================================================
// Main — orchestrates all test sections
// =========================================================================
auto TestEstimate::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    // =====================================================================
    // Section 0: Compilation Smoke Test
    // =====================================================================
    PrintLn("--- Section 0: Compilation Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec0Smoke);
    sec0SmokeCrossType();
    PrintLn("  0 passed: compilation smoke test (K=1,2,3,5,10 Full/Diag + dynamic + cross-type)");

    // =====================================================================
    // Section 1: Construction and Basic Accessors
    // =====================================================================
    PrintLn("--- Section 1: Construction and Basic Accessors ---");
    RunOverAllDims<AllStaticDims>(sec1Construction);
    PrintLn("  1 passed: construction and basic accessors (K=1,2,3,5,10 Full/Diag + dynamic)");

    // Cross-dimension copy (Static <-> Dynamic)
    sec1CrossDimCopy();
    PrintLn("  1j passed: cross-dimension copy");

    // =====================================================================
    // Section 3: Assignment Operators
    // =====================================================================
    PrintLn("--- Section 3: Assignment Operators ---");
    RunOverAllDims<AllStaticDims>(sec3Assignment);
    PrintLn("  3 passed: assignment operators (K=1,2,3,5,10 Full/Diag + dynamic)");

    // Cross-dimension assignment
    sec3CrossDimAssignment();
    PrintLn("  3e passed: cross-dimension assignment");

    // =====================================================================
    // Section 4: Arithmetic In-Place — += and -=
    // =====================================================================
    PrintLn("--- Section 4: Arithmetic In-Place (+= and -=) ---");
    RunOverAllDims<AllStaticDims>(sec4AddSubInPlace);
    PrintLn("  4 passed: += and -= (K=1,2,3,5,10 Full/Diag + dynamic)");

    // =====================================================================
    // Section 5: Arithmetic Free Functions — +, -, unary -
    // =====================================================================
    PrintLn("--- Section 5: Arithmetic Free Functions (+ , -, unary -) ---");
    RunOverAllDims<AllStaticDims>(sec5FreeAddSub);
    PrintLn("  5 passed: free functions +, -, unary - (K=1,2,3,5,10 Full/Diag + dynamic)");

    // =====================================================================
    // Section 6: Arithmetic In-Place — *= and /=
    // =====================================================================
    PrintLn("--- Section 6: Arithmetic In-Place (*= and /=) ---");
    RunOverAllDims<AllStaticDims>(sec6MulDivInPlace);
    PrintLn("  6 passed: *= and /= (K=1,2,3,5,10 Full/Diag + dynamic)");

    // =====================================================================
    // Section 7: Free Functions — *, /, pow, NegateAdd, Divide, Pow, Exp
    // =====================================================================
    PrintLn("--- Section 7: Free Functions (*, /, pow, NegateAdd, Divide, Pow, Exp) ---");
    RunOverAllDims<AllStaticDims>(sec7FreeMulDivPow);
    PrintLn("  7 passed: free functions *, /, pow, etc. (K=1,2,3,5,10 Full/Diag + dynamic)");

    // =====================================================================
    // Section 7b: Self-Operations (aliasing)
    // =====================================================================
    PrintLn("--- Section 7b: Self-Operations (aliasing) ---");
    RunOverAllDims<AllStaticDims>(sec7bSelfOps);
    PrintLn("  7b passed: self operations (K=1,2,3,5,10 Full/Diag + dynamic)");

    // =====================================================================
    // Section 8: Dot Product
    // =====================================================================
    PrintLn("--- Section 8: Dot Product ---");
    RunOverAllDims<AllStaticDims>(sec8DotProduct);
    PrintLn("  8 passed: dot product (K=2,3,5,10 Full/Diag + dynamic)");

    // Section 8h: Matrix-vector RightMultiply
    PrintLn("--- Section 8h: Matrix-vector RightMultiply ---");
    RunOverAllDims<AllStaticDims>(sec8hRightMultiply);
    PrintLn("  8h passed: RightMultiply (K=2,3,5,10 Full/Diag + dynamic)");

    // Section 8i: Matrix-vector operator*
    PrintLn("--- Section 8i: Matrix-vector operator* ---");
    RunOverAllDims<AllStaticDims>(sec8iMatrixMultiplyOperator);
    PrintLn("  8i passed: operator* (K=2,3,5,10 Full/Diag + dynamic)");

    // Section 8j: Matrix-vector multiply — dimension mismatch
    PrintLn("--- Section 8j: Matrix-vector multiply — exception ---");
    RunOverAllDims<AllStaticDims>(sec8jRightMultiplyException);
    PrintLn("  8j passed: RightMultiply/LeftMultiply exception (K=2,3,5,10 Full/Diag + dynamic)");

    // Section 8k: Matrix-vector LeftMultiply
    PrintLn("--- Section 8k: Matrix-vector LeftMultiply ---");
    RunOverAllDims<AllStaticDims>(sec8kLeftMultiply);
    PrintLn("  8k passed: LeftMultiply (K=2,3,5,10 Full/Diag + dynamic)");

    // =====================================================================
    // Sections 9-16: Math Functions
    // For each math function, covariance is propagated via Jacobian:
    // Cov_result = J * Cov_input * J^T,  J = diag(df_i/dx_i)
    // =====================================================================

    // Section 9: Algebraic/Power Math Functions
    PrintLn("--- Section 9: Algebraic/Power Math Functions ---");
    RunOverAllDims<AllStaticDims>(sec9Algebraic);
    PrintLn("  9 passed: algebraic math functions (K=1,2,3,5,10 Full/Diag + dynamic)");

    // Section 9b: Abs
    PrintLn("--- Section 9b: Abs ---");
    RunOverAllDims<AllStaticDims>(sec9bAbs);
    PrintLn("  9b passed: abs (K=1,2,3,5,10 Full/Diag + dynamic)");

    // Section 10: Exponential Functions
    PrintLn("--- Section 10: Exponential Functions ---");
    RunOverAllDims<AllStaticDims>(sec10Exponential);
    PrintLn("  10 passed: exponential functions (K=1,2,3,5,10 Full/Diag + dynamic)");

    // Section 11: Logarithmic Functions
    PrintLn("--- Section 11: Logarithmic Functions ---");
    RunOverAllDims<AllStaticDims>(sec11Logarithmic);
    PrintLn("  11 passed: logarithmic functions (K=1,2,3,5,10 Full/Diag + dynamic)");

    // Section 12: Trigonometric Functions
    PrintLn("--- Section 12: Trigonometric Functions ---");
    RunOverAllDims<AllStaticDims>(sec12Trigonometric);
    PrintLn("  12 passed: trigonometric functions (K=1,2,3,5,10 Full/Diag + dynamic)");

    // Section 13: Inverse Trigonometric Functions
    PrintLn("--- Section 13: Inverse Trigonometric Functions ---");
    RunOverAllDims<AllStaticDims>(sec13InverseTrig);
    PrintLn("  13 passed: inverse trigonometric functions (K=1,2,3,5,10 Full/Diag + dynamic)");

    // Section 14: Hyperbolic Functions
    PrintLn("--- Section 14: Hyperbolic Functions ---");
    RunOverAllDims<AllStaticDims>(sec14Hyperbolic);
    PrintLn("  14 passed: hyperbolic functions (K=1,2,3,5,10 Full/Diag + dynamic)");

    // Section 15: Inverse Hyperbolic Functions
    PrintLn("--- Section 15: Inverse Hyperbolic Functions ---");
    RunOverAllDims<AllStaticDims>(sec15InverseHyperbolic);
    PrintLn("  15 passed: inverse hyperbolic functions (K=1,2,3,5,10 Full/Diag + dynamic)");

    // Section 16: Special Functions
    PrintLn("--- Section 16: Special Functions ---");
    RunOverAllDims<AllStaticDims>(sec16Special);
    PrintLn("  16 passed: special functions (K=1,2,3,5,10 Full/Diag + dynamic)");

    // =====================================================================
    // Section 17: Negate and Chaining
    // =====================================================================
    PrintLn("--- Section 17: Negate and Chaining ---");
    RunOverAllDims<AllStaticDims>(sec17NegateChaining);
    PrintLn("  17 passed: negate and chaining (K=1,2,3,5,10 Full/Diag + dynamic)");

    // =====================================================================
    // Section 18: ToPOD / FromPOD / Base64
    // Static K only
    // =====================================================================
    PrintLn("--- Section 18: POD ---");
    RunOverStaticDims<AllStaticDims>(sec18POD);
    PrintLn("  18 passed: ToPOD/FromPOD/Base64 Full/Diag (K=1,2,3,5,10)");

    // Invalid Base64 throws
    sec18InvalidBase64();
    PrintLn("  18f passed: invalid Base64 throws");

    // =====================================================================
    // Section 19: Edge Cases and Numerical Stress
    // =====================================================================
    PrintLn("--- Section 19: Edge Cases and Numerical Stress ---");
    RunOverAllDims<AllStaticDims>(sec19EdgeCases);
    PrintLn("  19 passed: edge cases and numerical stress (K=1,2,3,5,10 Full/Diag + dynamic)");
    RunOverAllDims<AllStaticDims>(sec19ExpLogRoundtrip);
    PrintLn("  19 passed: exponential/logarithmic round-trip (K=1,2,3,5,10 Full/Diag + dynamic)");
    RunOverAllDims<AllStaticDims>(sec19cCorrZeroVar);
    PrintLn("  19 passed: correlation zero-variance edge case (K=2,3,5,10 Full + dynamic)");

    // =====================================================================
    // Section 20: Cross-Verification
    // =====================================================================
    PrintLn("--- Section 20: Cross-Verification ---");
    sec20CrossVerification();
    PrintLn("  20 passed: cross-verification");

    // =====================================================================
    // Section 21: Normalize
    // =====================================================================
    PrintLn("--- Section 21: Normalize ---");
    RunOverAllDims<AllStaticDims>(sec21Normalize);
    PrintLn("  21 passed: normalize (K=2,3,5,10 Full/Diag + dynamic)");

    // =====================================================================
    // Section 22: Sum
    // =====================================================================
    PrintLn("--- Section 22: Sum ---");
    RunOverAllDims<AllStaticDims>(sec22Sum);
    PrintLn("  22 passed: sum (K=2,3,5,10 Full/Diag + dynamic)");

    // =====================================================================
    // Section 23: Prod
    // =====================================================================
    PrintLn("--- Section 23: Prod ---");
    RunOverAllDims<AllStaticDims>(sec23Prod);
    PrintLn("  23 passed: prod (K=2,3,5,10 Full/Diag + dynamic)");

    // =====================================================================
    // Section 24: SquaredNorm
    // =====================================================================
    PrintLn("--- Section 24: SquaredNorm ---");
    RunOverAllDims<AllStaticDims>(sec24SquaredNorm);
    PrintLn("  24 passed: squared norm (K=2,3,5,10 Full/Diag + dynamic)");

    // =====================================================================
    // Section 25: Norm
    // =====================================================================
    PrintLn("--- Section 25: Norm ---");
    RunOverAllDims<AllStaticDims>(sec25Norm);
    PrintLn("  25 passed: norm (K=2,3,5,10 Full/Diag + dynamic)");

    // =====================================================================
    // Section 26: Mean
    // =====================================================================
    PrintLn("--- Section 26: Mean ---");
    RunOverAllDims<AllStaticDims>(sec26Mean);
    PrintLn("  26 passed: mean (K=2,3,5,10 Full/Diag + dynamic)");

    // =====================================================================
    // Section 27: HarmonicMean
    // =====================================================================
    PrintLn("--- Section 27: HarmonicMean ---");
    RunOverAllDims<AllStaticDims>(sec27HarmonicMean);
    PrintLn("  27 passed: harmonic mean (K=2,3,5,10 Full/Diag + dynamic)");

    // =====================================================================
    // Section 28: GeometricMean
    // =====================================================================
    PrintLn("--- Section 28: GeometricMean ---");
    RunOverAllDims<AllStaticDims>(sec28GeometricMean);
    PrintLn("  28 passed: geometric mean (K=2,3,5,10 Full/Diag + dynamic)");

    // =====================================================================
    // Section 29: QuadraticMean
    // =====================================================================
    PrintLn("--- Section 29: QuadraticMean ---");
    RunOverAllDims<AllStaticDims>(sec29QuadraticMean);
    PrintLn("  29 passed: quadratic mean (K=2,3,5,10 Full/Diag + dynamic)");

    // =====================================================================
    // Section 30: CubicMean
    // =====================================================================
    PrintLn("--- Section 30: CubicMean ---");
    RunOverAllDims<AllStaticDims>(sec30CubicMean);
    PrintLn("  30 passed: cubic mean (K=2,3,5,10 Full/Diag + dynamic)");

    // =====================================================================
    // Section 31: LpNorm
    // =====================================================================
    PrintLn("--- Section 31: LpNorm ---");
    RunOverAllDims<AllStaticDims>(sec31LpNorm);
    PrintLn("  31 passed: LpNorm (K=2,3,5,10 Full/Diag + dynamic)");

    // =====================================================================
    // All tests passed
    // =====================================================================

    PrintLn("All TestEstimate tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
