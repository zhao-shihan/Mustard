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

TestEstimate10::TestEstimate10() :
    Subprogram{"TestEstimate10", "Test Mustard::Math::Estimate (Section 10: Algebraic Functions)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Section 7 Smoke: Algebraic Functions and Abs (from sec0Smoke)
// =========================================================================

constexpr auto sec7Smoke{[]<int K, CovarianceOption C>() {
    using Est = Estimate<K, C>;
    constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    [[maybe_unused]] double scalar{};
    [[maybe_unused]] typename Est::ValueType vec{};

    const auto newEst{[&] {
        if constexpr (K == 1) {
            return Est{1.0, 0.5};
        } else {
            return Est{x, cov};
        }
    }};

    auto est{newEst()};

    // ---- in-place ----
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

    // ---- free ----
    est = newEst();
    vec = square(est).Value();
    vec = cube(est).Value();
    vec = sqrt(est).Value();
    vec = cbrt(est).Value();
    vec = rsqrt(est).Value();
    vec = inverse(est).Value();
    vec = abs(est).Value();
}};

constexpr auto sec7Algebraic{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    // --- Square: f(x)=x^2, J=diag(2x) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.SquareInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), x(0) * x(0), "10a: Square value");
            CheckClose(e.Variance(), 4.0 * x(0) * x(0) * cov.diagonal()(0), "10a: Square var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), x(i) * x(i), fmt::format("10a: Square value({})", i));
            }
            CheckMathFunction(e, cov, "10a: Square",
                              [&](int i) { return 2.0 * x(i); });
        }

        const auto e2{square(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), x(0) * x(0), "10b: square() free value");
        } else {
            CheckClose(e2.Value(0), x(0) * x(0), "10b: square() free value");
        }
    }

    // --- Cube: f(x)=x^3, J=diag(3x^2) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.CubeInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), x(0) * x(0) * x(0), "10c: Cube value");
            CheckClose(e.Variance(), 9.0 * x(0) * x(0) * x(0) * x(0) * cov.diagonal()(0), "10c: Cube var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), x(i) * x(i) * x(i), fmt::format("10c: Cube value({})", i));
            }
            CheckMathFunction(e, cov, "10c: Cube",
                              [&](int i) { return 3.0 * x(i) * x(i); });
        }

        const auto e2{cube(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), x(0) * x(0) * x(0), "10d: cube() free value");
        } else {
            CheckClose(e2.Value(0), x(0) * x(0) * x(0), "10d: cube() free value");
        }
    }

    // --- Sqrt: f(x)=sqrt(x), J=diag(1/(2*sqrt(x))) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.SqrtInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::sqrt(x(0)), "10e: Sqrt value");
            CheckClose(e.Variance(), 0.25 / x(0) * cov.diagonal()(0), "10e: Sqrt var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::sqrt(x(i)), fmt::format("10e: Sqrt value({})", i));
            }
            CheckMathFunction(e, cov, "10e: Sqrt",
                              [&](int i) { return 0.5 / std::sqrt(x(i)); });
        }

        const auto e2{sqrt(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), std::sqrt(x(0)), "10f: sqrt() free value");
        } else {
            CheckClose(e2.Value(0), std::sqrt(x(0)), "10f: sqrt() free value");
        }
    }

    // --- Cbrt: f(x)=cbrt(x), J=diag(1/(3*cbrt(x)^2)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.CbrtInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::cbrt(x(0)), "10g: Cbrt value");
            auto jac{1.0 / (3.0 * std::pow(std::cbrt(x(0)), 2))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "10g: Cbrt var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::cbrt(x(i)), fmt::format("10g: Cbrt value({})", i));
            }
            CheckMathFunction(e, cov, "10g: Cbrt",
                              [&](int i) { return 1.0 / (3.0 * std::pow(std::cbrt(x(i)), 2)); });
        }

        const auto e2{cbrt(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), std::cbrt(x(0)), "10h: cbrt() free value");
        } else {
            CheckClose(e2.Value(0), std::cbrt(x(0)), "10h: cbrt() free value");
        }
    }

    // --- Rsqrt: f(x)=1/sqrt(x), J=diag(-1/(2*x^(3/2))) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.RsqrtInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), 1.0 / std::sqrt(x(0)), "10i: Rsqrt value");
            auto jac{-0.5 / (x(0) * std::sqrt(x(0)))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "10i: Rsqrt var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), 1.0 / std::sqrt(x(i)), fmt::format("10i: Rsqrt value({})", i));
            }
            CheckMathFunction(e, cov, "10i: Rsqrt",
                              [&](int i) { return -0.5 / (x(i) * std::sqrt(x(i))); });
        }

        const auto e2{rsqrt(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), 1.0 / std::sqrt(x(0)), "10j: rsqrt() free value");
        } else {
            CheckClose(e2.Value(0), 1.0 / std::sqrt(x(0)), "10j: rsqrt() free value");
        }
    }

    // --- Inverse: f(x)=1/x, J=diag(-1/x^2) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.InverseInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), 1.0 / x(0), "10k: Inverse value");
            CheckClose(e.Variance(), 1.0 / (x(0) * x(0) * x(0) * x(0)) * cov.diagonal()(0), "10k: Inverse var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), 1.0 / x(i), fmt::format("10k: Inverse value({})", i));
            }
            CheckMathFunction(e, cov, "10k: Inverse",
                              [&](int i) { return -1.0 / (x(i) * x(i)); });
        }

        const auto e2{inverse(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), 1.0 / x(0), "10l: inverse() free value");
        } else {
            CheckClose(e2.Value(0), 1.0 / x(0), "10l: inverse() free value");
        }
    }
}};

// =========================================================================
// Section 7b: Abs (element-wise absolute value)
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
            CheckClose(e.Value(), std::abs(x(0)), "10b_a: Abs value");
            CheckClose(e.Variance(), cov.diagonal()(0), "10b_a: Abs var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::abs(x(i)), fmt::format("10b_a: Abs value({})", i));
            }
            CheckMathFunction(e, cov, "10b_a: Abs",
                              [&](int i) { return x(i) > 0 ? 1.0 : (x(i) < 0 ? -1.0 : 0.0); });
        }

        const auto e2{abs(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), std::abs(x(0)), "10b_b: abs() free value");
        } else {
            CheckClose(e2.Value(0), std::abs(x(0)), "10b_b: abs() free value");
        }
    }
}};

} // namespace TestEstimateSection

auto TestEstimate10::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 10: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec7Smoke);
    PrintLn("  10 smoke passed: algebraic and abs compilation smoke");

    PrintLn("--- Section 10: Algebraic/Power Math Functions ---");
    RunOverAllDims<AllStaticDims>(sec7Algebraic);
    PrintLn("  10 passed: algebraic math functions (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 7b: Abs ---");
    RunOverAllDims<AllStaticDims>(sec9bAbs);
    PrintLn("  10b passed: abs (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestEstimate10 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
