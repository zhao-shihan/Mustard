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

TestEstimate11::TestEstimate11() :
    Subprogram{"TestEstimate11", "Test Mustard::Math::Estimate (Section 11: Exponential Functions)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Section 8 Smoke: Exponential Functions (from sec0Smoke)
// =========================================================================

constexpr auto sec8Smoke{[]<int K, CovarianceOption C>() {
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
    est = std::move(est).Exp();
    est = newEst();
    est = std::move(est).Exp2();
    est = newEst();
    est = std::move(est).Expm1();

    // ---- free ----
    est = newEst();
    vec = exp(est).Value();
    vec = exp2(est).Value();
    vec = expm1(est).Value();
}};

constexpr auto sec8Exponential{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    // --- Exp: f(x)=exp(x), J=diag(exp(x)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.ExpInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::exp(x(0)), "11a: Exp value");
            auto jac{std::exp(x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "11a: Exp var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::exp(x(i)), fmt::format("11a: Exp value({})", i));
            }
            CheckMathFunction(e, cov, "11a: Exp",
                              [&](int i) { return std::exp(x(i)); });
        }

        const auto e2{exp(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), std::exp(x(0)), "11b: exp() free value");
        } else {
            CheckClose(e2.Value(0), std::exp(x(0)), "11b: exp() free value");
        }
    }

    // --- Exp2: f(x)=2^x, J=diag(ln2*2^x) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.Exp2InPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::exp2(x(0)), "11c: Exp2 value");
            auto jac{std::log(2.0) * std::exp2(x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "11c: Exp2 var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::exp2(x(i)), fmt::format("11c: Exp2 value({})", i));
            }
            CheckMathFunction(e, cov, "11c: Exp2",
                              [&](int i) { return std::log(2.0) * std::exp2(x(i)); });
        }

        const auto e2{exp2(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), std::exp2(x(0)), "11d: exp2() free value");
        } else if constexpr (dim >= 2) {
            CheckClose(e2.Value(1), std::exp2(x(1)), "11d: exp2() free value");
        }
    }

    // --- Expm1: f(x)=exp(x)-1, J=diag(exp(x)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.Expm1InPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::expm1(x(0)), "11e: Expm1 value");
            auto jac{std::exp(x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "11e: Expm1 var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::expm1(x(i)), fmt::format("11e: Expm1 value({})", i));
            }
            CheckMathFunction(e, cov, "11e: Expm1",
                              [&](int i) { return std::exp(x(i)); });
        }

        const auto e2{expm1(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), std::expm1(x(0)), "11f: expm1() free value");
        } else if constexpr (dim >= 2) {
            CheckClose(e2.Value(1), std::expm1(x(1)), "11f: expm1() free value");
        }
    }
}};

} // namespace TestEstimateSection

auto TestEstimate11::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 11: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec8Smoke);
    PrintLn("  11 smoke passed: exponential functions compilation smoke");

    PrintLn("--- Section 11: Exponential Functions ---");
    RunOverAllDims<AllStaticDims>(sec8Exponential);
    PrintLn("  11 passed: exponential functions (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestEstimate11 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
