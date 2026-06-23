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

TestEstimate16::TestEstimate16() :
    Subprogram{"TestEstimate16", "Test Mustard::Math::Estimate (Section 16: Inverse Hyperbolic)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

constexpr auto sec13Smoke{[]<int K, CovarianceOption C>() {
    using Est = Estimate<K, C>;
    constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    [[maybe_unused]] typename Est::ValueType vec{};
    const auto newEst{[&] { if constexpr (K == 1) return Est{1.0, 0.5}; else return Est{x, cov}; }};
    auto est{newEst()};
    est = newEst();
    est = std::move(est).Asinh();
    est = newEst();
    est = std::move(est).Acosh();
    est = newEst();
    est = std::move(est).Atanh();
    est = newEst();
    vec = asinh(est).Value();
    vec = acosh(est).Value();
    vec = atanh(est).Value();
}};

constexpr auto sec13InverseHyperbolic{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    // --- Asinh: f(x)=asinh(x), J=diag(1/sqrt(1+x^2)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.AsinhInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::asinh(x(0)), "16a: Asinh value");
            auto jac{1.0 / std::sqrt(1.0 + x(0) * x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "16a: Asinh var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::asinh(x(i)), fmt::format("16a: Asinh value({})", i));
            }
            CheckMathFunction(e, cov, "16a: Asinh",
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
            CheckClose(e.Value(), std::acosh(x(0)), "16c: Acosh value");
            auto jac{1.0 / std::sqrt(x(0) * x(0) - 1.0)};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "16c: Acosh var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::acosh(x(i)), fmt::format("16c: Acosh value({})", i));
            }
            CheckMathFunction(e, cov, "16c: Acosh",
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
            CheckClose(e.Value(), std::atanh(x(0)), "16e: Atanh value");
            auto jac{1.0 / (1.0 - x(0) * x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "16e: Atanh var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::atanh(x(i)), fmt::format("16e: Atanh value({})", i));
            }
            CheckMathFunction(e, cov, "16e: Atanh",
                              [&](int i) { return 1.0 / (1.0 - x(i) * x(i)); });
        }
    }
}};

} // namespace TestEstimateSection

auto TestEstimate16::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 16: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec13Smoke);
    PrintLn("  16 smoke passed: inverse hyperbolic compilation smoke");

    PrintLn("--- Section 16: Inverse Hyperbolic Functions ---");
    RunOverAllDims<AllStaticDims>(sec13InverseHyperbolic);
    PrintLn("  16 passed: inverse hyperbolic functions (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestEstimate16 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
