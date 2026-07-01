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

TestEstimate38::TestEstimate38() :
    Subprogram{"TestEstimate38", "Test Mustard::Math::Estimate (Section 38: User-defined Apply)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Section 38 Smoke: Apply and ApplyInPlace compilation
// =========================================================================

constexpr auto sec38Smoke{[]<int K, CovarianceOption C>() {
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

    // ---- in-place Apply ----
    est = newEst();
    est = std::move(est).Apply([]<typename T>(const T& t) -> T { return t; });

    // ---- const& Apply ----
    constexpr auto f{[]<typename T>(const T& t) -> T { return t * t; }};
    auto r1{est.Apply(f)};      // const&
    auto r2{newEst().Apply(f)}; // && with lvalue lambda
    vec = r1.Value();
    vec = r2.Value();

    // ---- Apply with scalar estimate ----
    if constexpr (K == 1) {
        est = newEst();
        scalar = est.Apply([]<typename T>(const T& t) -> T {
                        using std::exp;
                        return exp(t);
                    })
                     .Value();
    }
}};

// =========================================================================
// Section 38: Apply with x^2 — compare with built-in square
// =========================================================================

constexpr auto sec38ApplySquare{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    auto e{MakeEstimate<K, C>(x, cov)};

    auto eSquare{e.Apply([]<typename T>(const T& t) -> T { return t * t; })};
    auto eBuiltin{square(MakeEstimate<K, C>(x, cov))};

    if constexpr (K == 1) {
        CheckClose(eSquare.Value(), eBuiltin.Value(), "38a: Apply x^2 vs square value");
        CheckClose(eSquare.Variance(), eBuiltin.Variance(), "38a: Apply x^2 vs square var");
    } else {
        for (auto i{0}; i < dim; ++i) {
            CheckClose(eSquare.Value(i), eBuiltin.Value(i), fmt::format("38a: Apply x^2 value({})", i));
            CheckClose(eSquare.Variance(i), eBuiltin.Variance(i), fmt::format("38a: Apply x^2 var({})", i));
            if constexpr (C == CovarianceOption::Full) {
                for (auto j{0}; j < dim; ++j) {
                    CheckClose(eSquare.Covariance(i, j), eBuiltin.Covariance(i, j),
                               fmt::format("38a: Apply x^2 cov({},{})", i, j));
                }
            }
        }
    }
}};

// =========================================================================
// Section 38: Apply with exp(x) — compare with built-in exp
// =========================================================================

constexpr auto sec38ApplyExp{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    auto e{MakeEstimate<K, C>(x, cov)};

    auto eApply{e.Apply([]<typename T>(const T& t) -> T {
        using std::exp;
        return exp(t);
    })};
    auto eBuiltin{exp(MakeEstimate<K, C>(x, cov))};

    if constexpr (K == 1) {
        CheckClose(eApply.Value(), eBuiltin.Value(), "38b: Apply exp vs builtin exp value");
        CheckClose(eApply.Variance(), eBuiltin.Variance(), "38b: Apply exp vs builtin exp var");
    } else {
        for (auto i{0}; i < dim; ++i) {
            CheckClose(eApply.Value(i), eBuiltin.Value(i), fmt::format("38b: Apply exp value({})", i));
            CheckClose(eApply.Variance(i), eBuiltin.Variance(i), fmt::format("38b: Apply exp var({})", i));
        }
    }
}};

// =========================================================================
// Section 38: Apply with composite function f(x) = x^3 + 2*sin(x)
// =========================================================================

constexpr auto sec38ApplyComposite{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    auto e{MakeEstimate<K, C>(x, cov)};

    e.ApplyInPlace([]<typename T>(const T& t) -> T {
        using std::sin;
        return t * t * t + 2 * sin(t);
    });

    if constexpr (K == 1) {
        CheckClose(e.Value(), x(0) * x(0) * x(0) + 2 * std::sin(x(0)),
                   "38c: Composite value");
        auto expectedVar{cov.diagonal()(0) * muc::pow(3 * x(0) * x(0) + 2 * std::cos(x(0)), 2)};
        CheckClose(e.Variance(), expectedVar, "38c: Composite var");
    } else {
        for (auto i{0}; i < dim; ++i) {
            CheckClose(e.Value(i), x(i) * x(i) * x(i) + 2 * std::sin(x(i)),
                       fmt::format("38c: Composite value({})", i));
        }
        CheckMathFunction(e, cov, "38c: Composite",
                          [&](int i) { return 3 * x(i) * x(i) + 2 * std::cos(x(i)); });
    }
}};

// =========================================================================
// Section 38: Apply with constant function f(x) = 3.0
//             (returns plain double, not AutoDiffScalar — Jacobian = 0)
// =========================================================================

constexpr auto sec38ApplyConstant{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    auto e{MakeEstimate<K, C>(x, cov)};

    e.ApplyInPlace([]<typename T>(const T&) -> T { return 3.0; });

    if constexpr (K == 1) {
        CheckClose(e.Value(), 3.0, "38d: Constant value");
        CheckClose(e.Variance(), 0.0, "38d: Constant var");
    } else {
        for (auto i{0}; i < dim; ++i) {
            CheckClose(e.Value(i), 3.0, fmt::format("38d: Constant value({})", i));
        }
        // All covariance entries should be zero
        for (auto i{0}; i < dim; ++i) {
            for (auto j{0}; j < dim; ++j) {
                CheckClose(e.Covariance(i, j), 0.0,
                           fmt::format("38d: Constant cov({},{})", i, j));
            }
        }
    }
}};

// =========================================================================
// Section 38: Apply with linear function f(x) = 2*x + 1
//             Jacobian = diag(2), cov scaled by 4
// =========================================================================

constexpr auto sec38ApplyLinear{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    auto e{MakeEstimate<K, C>(x, cov)};

    e.ApplyInPlace([]<typename T>(const T& t) -> T { return 2 * t + 1; });

    if constexpr (K == 1) {
        CheckClose(e.Value(), 2 * x(0) + 1, "38e: Linear value");
        CheckClose(e.Variance(), 4 * cov.diagonal()(0), "38e: Linear var");
    } else {
        for (auto i{0}; i < dim; ++i) {
            CheckClose(e.Value(i), 2 * x(i) + 1, fmt::format("38e: Linear value({})", i));
        }
        CheckMathFunction(e, cov, "38e: Linear",
                          [](int) { return 2.0; });
    }
}};

// =========================================================================
// Section 38: Apply with dynamic dimension
// =========================================================================

constexpr auto sec38ApplyDynamic{[] {
    constexpr int dyn{Eigen::Dynamic};
    constexpr auto full{CovarianceOption::Full};
    constexpr int dim{4};

    const auto x{MakeTestValue<dyn, full>(dim)};
    const auto cov{MakeTestCov<dyn, full>(dim)};
    auto e{MakeEstimate<dyn, full>(x, cov)};

    e.ApplyInPlace([]<typename T>(const T& t) -> T {
        using std::log;
        return t * t + log(t);
    });

    for (auto i{0}; i < dim; ++i) {
        CheckClose(e.Value(i), x(i) * x(i) + std::log(x(i)),
                   fmt::format("38f: Dynamic Apply value({})", i));
    }
    CheckMathFunction(e, cov, "38f: Dynamic Apply",
                      [&](int i) { return 2 * x(i) + 1.0 / x(i); });
}};

} // namespace TestEstimateSection

auto TestEstimate38::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 38: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec38Smoke);
    PrintLn("  38 smoke passed: compilation (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 38: Apply x^2 vs square ---");
    RunOverAllDims<AllStaticDims>(sec38ApplySquare);
    PrintLn("  38a passed: Apply x^2 matches built-in square");

    PrintLn("--- Section 38: Apply exp vs built-in exp ---");
    RunOverAllDims<AllStaticDims>(sec38ApplyExp);
    PrintLn("  38b passed: Apply exp matches built-in exp");

    PrintLn("--- Section 38: Apply composite f(x)=x^3+2sin(x) ---");
    RunOverAllDims<AllStaticDims>(sec38ApplyComposite);
    PrintLn("  38c passed: composite function covariance");

    PrintLn("--- Section 38: Apply constant f(x)=3.0 ---");
    RunOverAllDims<AllStaticDims>(sec38ApplyConstant);
    PrintLn("  38d passed: constant function zeroes covariance");

    PrintLn("--- Section 38: Apply linear f(x)=2x+1 ---");
    RunOverAllDims<AllStaticDims>(sec38ApplyLinear);
    PrintLn("  38e passed: linear function J=2");

    PrintLn("--- Section 38: Apply dynamic dimension ---");
    sec38ApplyDynamic();
    PrintLn("  38f passed: dynamic dimension");

    PrintLn("All TestEstimate38 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
