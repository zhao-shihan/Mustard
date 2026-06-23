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

TestEstimate15::TestEstimate15() :
    Subprogram{"TestEstimate15", "Test Mustard::Math::Estimate (Section 15: Hyperbolic Functions)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

constexpr auto sec12Smoke{[]<int K, CovarianceOption C>() {
    using Est = Estimate<K, C>;
    constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    [[maybe_unused]] typename Est::ValueType vec{};
    const auto newEst{[&] { if constexpr (K == 1) return Est{1.0, 0.5}; else return Est{x, cov}; }};
    auto est{newEst()};
    est = newEst();
    est = std::move(est).Sinh();
    est = newEst();
    est = std::move(est).Cosh();
    est = newEst();
    est = std::move(est).Tanh();
    est = newEst();
    vec = sinh(est).Value();
    vec = cosh(est).Value();
    vec = tanh(est).Value();
}};

constexpr auto sec12Hyperbolic{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    // --- Sinh: f(x)=sinh(x), J=diag(cosh(x)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.SinhInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::sinh(x(0)), "15a: Sinh value");
            auto jac{std::cosh(x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "15a: Sinh var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::sinh(x(i)), fmt::format("15a: Sinh value({})", i));
            }
            CheckMathFunction(e, cov, "15a: Sinh",
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
            CheckClose(e.Value(), std::cosh(x(0)), "15c: Cosh value");
            auto jac{std::sinh(x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "15c: Cosh var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::cosh(x(i)), fmt::format("15c: Cosh value({})", i));
            }
            CheckMathFunction(e, cov, "15c: Cosh",
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
            CheckClose(e.Value(), std::tanh(x(0)), "15e: Tanh value");
            auto c{std::cosh(x(0))};
            auto jac{1.0 / (c * c)};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "15e: Tanh var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::tanh(x(i)), fmt::format("15e: Tanh value({})", i));
            }
            CheckMathFunction(e, cov, "15e: Tanh",
                              [&](int i) { auto c{std::cosh(x(i))}; return 1.0 / (c * c); });
        }

        const auto e2{tanh(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), std::tanh(x(0)), "15f: tanh() free value");
        } else {
            CheckClose(e2.Value(0), std::tanh(x(0)), "15f: tanh() free value");
        }
    }
}};

} // namespace TestEstimateSection

auto TestEstimate15::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 15: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec12Smoke);
    PrintLn("  15 smoke passed: hyperbolic compilation smoke");

    PrintLn("--- Section 15: Hyperbolic Functions ---");
    RunOverAllDims<AllStaticDims>(sec12Hyperbolic);
    PrintLn("  15 passed: hyperbolic functions (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestEstimate15 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
