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

TestEstimate12::TestEstimate12() :
    Subprogram{"TestEstimate12", "Test Mustard::Math::Estimate (Section 12: Logarithmic Functions)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Section 9 Smoke: Logarithmic Functions (from sec0Smoke)
// =========================================================================

constexpr auto sec9Smoke{[]<int K, CovarianceOption C>() {
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

    est = newEst();
    est = std::move(est).Log();
    est = newEst();
    est = std::move(est).Log10();
    est = newEst();
    est = std::move(est).Log2();
    est = newEst();
    est = std::move(est).Log1p();

    est = newEst();
    vec = log(est).Value();
    vec = log10(est).Value();
    vec = log2(est).Value();
    vec = log1p(est).Value();
}};

constexpr auto sec9Logarithmic{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    // --- Log: f(x)=ln(x), J=diag(1/x) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.LogInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::log(x(0)), "12a: Log value");
            CheckClose(e.Variance(), 1.0 / (x(0) * x(0)) * cov.diagonal()(0), "12a: Log var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::log(x(i)), fmt::format("12a: Log value({})", i));
            }
            CheckMathFunction(e, cov, "12a: Log",
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
            CheckClose(e.Value(), std::log10(x(0)), "12c: Log10 value");
            auto jac{1.0 / (x(0) * std::log(10.0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "12c: Log10 var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::log10(x(i)), fmt::format("12c: Log10 value({})", i));
            }
            CheckMathFunction(e, cov, "12c: Log10",
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
            CheckClose(e.Value(), std::log2(x(0)), "12e: Log2 value");
            auto jac{1.0 / (x(0) * std::log(2.0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "12e: Log2 var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::log2(x(i)), fmt::format("12e: Log2 value({})", i));
            }
            CheckMathFunction(e, cov, "12e: Log2",
                              [&](int i) { return 1.0 / (x(i) * std::log(2.0)); });
        }

        const auto e2{log2(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), std::log2(x(0)), "12f: log2() free value");
        } else if constexpr (dim >= 2) {
            CheckClose(e2.Value(1), 1.0, "12f: log2() free value");
        }
    }

    // --- Log1p: f(x)=ln(1+x), J=diag(1/(1+x)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.Log1pInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::log1p(x(0)), "12g: Log1p value");
            auto jac{1.0 / (1.0 + x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "12g: Log1p var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::log1p(x(i)), fmt::format("12g: Log1p value({})", i));
            }
            CheckMathFunction(e, cov, "12g: Log1p",
                              [&](int i) { return 1.0 / (1.0 + x(i)); });
        }

        const auto e2{log1p(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), std::log1p(x(0)), "12h: log1p() free value");
        } else {
            CheckClose(e2.Value(0), std::log1p(x(0)), "12h: log1p() free value");
        }
    }
}};

} // namespace TestEstimateSection

auto TestEstimate12::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 12: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec9Smoke);
    PrintLn("  12 smoke passed: logarithmic compilation smoke");

    PrintLn("--- Section 12: Logarithmic Functions ---");
    RunOverAllDims<AllStaticDims>(sec9Logarithmic);
    PrintLn("  12 passed: logarithmic functions (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestEstimate12 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
