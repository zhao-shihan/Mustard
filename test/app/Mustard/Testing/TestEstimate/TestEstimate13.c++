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

TestEstimate13::TestEstimate13() :
    Subprogram{"TestEstimate13", "Test Mustard::Math::Estimate (Section 13: Trigonometric Functions)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Section 10 Smoke: Trigonometric Functions (from sec0Smoke)
// =========================================================================

constexpr auto sec10Smoke{[]<int K, CovarianceOption C>() {
    using Est = Estimate<K, C>;
    constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    [[maybe_unused]] typename Est::ValueType vec{};
    const auto newEst{[&] { if constexpr (K == 1) return Est{1.0, 0.5}; else return Est{x, cov}; }};
    auto est{newEst()};
    est = newEst();
    est = std::move(est).Sin();
    est = newEst();
    est = std::move(est).Cos();
    est = newEst();
    est = std::move(est).Tan();
    est = newEst();
    vec = sin(est).Value();
    vec = cos(est).Value();
    vec = tan(est).Value();
}};

constexpr auto sec10Trigonometric{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    // --- Sin: f(x)=sin(x), J=diag(cos(x)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.SinInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::sin(x(0)), "13a: Sin value");
            auto jac{std::cos(x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "13a: Sin var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::sin(x(i)), fmt::format("13a: Sin value({})", i));
            }
            CheckMathFunction(e, cov, "13a: Sin",
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
            CheckClose(e.Value(), std::cos(x(0)), "13c: Cos value");
            auto jac{-std::sin(x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "13c: Cos var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::cos(x(i)), fmt::format("13c: Cos value({})", i));
            }
            CheckMathFunction(e, cov, "13c: Cos",
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
            CheckClose(e.Value(), std::tan(x(0)), "13e: Tan value");
            auto c{std::cos(x(0))};
            auto jac{1.0 / (c * c)};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "13e: Tan var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::tan(x(i)), fmt::format("13e: Tan value({})", i));
            }
            CheckMathFunction(e, cov, "13e: Tan",
                              [&](int i) { auto c{std::cos(x(i))}; return 1.0 / (c * c); });
        }
    }
}};

} // namespace TestEstimateSection

auto TestEstimate13::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 13: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec10Smoke);
    PrintLn("  13 smoke passed: trigonometric compilation smoke");

    PrintLn("--- Section 13: Trigonometric Functions ---");
    RunOverAllDims<AllStaticDims>(sec10Trigonometric);
    PrintLn("  13 passed: trigonometric functions (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestEstimate13 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
