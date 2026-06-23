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

TestEstimate14::TestEstimate14() :
    Subprogram{"TestEstimate14", "Test Mustard::Math::Estimate (Section 14: Inverse Trigonometric)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Section 11 Smoke: Inverse Trigonometric Functions (from sec0Smoke)
// =========================================================================

constexpr auto sec11Smoke{[]<int K, CovarianceOption C>() {
    using Est = Estimate<K, C>;
    constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    [[maybe_unused]] typename Est::ValueType vec{};
    const auto newEst{[&] { if constexpr (K == 1) return Est{1.0, 0.5}; else return Est{x, cov}; }};
    auto est{newEst()};
    est = newEst();
    est = std::move(est).Asin();
    est = newEst();
    est = std::move(est).Acos();
    est = newEst();
    est = std::move(est).Atan();
    est = newEst();
    vec = asin(est).Value();
    vec = acos(est).Value();
    vec = atan(est).Value();
}};

constexpr auto sec11InverseTrig{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    // --- Asin: f(x)=asin(x), J=diag(1/sqrt(1-x^2)) ---
    {
        const Eigen::Vector<double, K> x{MakeTestValue<K, C>(dim) * 0.01};
        const auto cov{EvalCov<K, C>(MakeTestCov<K, C>(dim) * muc::pow(0.01, 2))};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.AsinInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::asin(x(0)), "14a: Asin value");
            auto jac{1.0 / std::sqrt(1.0 - x(0) * x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "14a: Asin var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::asin(x(i)), fmt::format("14a: Asin value({})", i));
            }
            CheckMathFunction(e, cov, "14a: Asin",
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
            CheckClose(e.Value(), std::acos(x(0)), "14c: Acos value");
            auto jac{-1.0 / std::sqrt(1.0 - x(0) * x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "14c: Acos var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::acos(x(i)), fmt::format("14c: Acos value({})", i));
            }
            CheckMathFunction(e, cov, "14c: Acos",
                              [&](int i) { return -1.0 / std::sqrt(1.0 - x(i) * x(i)); });
        }

        const auto e2{acos(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), std::acos(x(0)), "14d: acos() free value");
        } else {
            CheckClose(e2.Value(0), std::acos(x(0)), "14d: acos() free value");
        }
    }

    // --- Atan: f(x)=atan(x), J=diag(1/(1+x^2)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.AtanInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::atan(x(0)), "14e: Atan value");
            auto jac{1.0 / (1.0 + x(0) * x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "14e: Atan var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::atan(x(i)), fmt::format("14e: Atan value({})", i));
            }
            CheckMathFunction(e, cov, "14e: Atan",
                              [&](int i) { return 1.0 / (1.0 + x(i) * x(i)); });
        }
    }
}};

} // namespace TestEstimateSection

auto TestEstimate14::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 14: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec11Smoke);
    PrintLn("  14 smoke passed: inverse trigonometric compilation smoke");

    PrintLn("--- Section 14: Inverse Trigonometric Functions ---");
    RunOverAllDims<AllStaticDims>(sec11InverseTrig);
    PrintLn("  14 passed: inverse trigonometric functions (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestEstimate14 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
