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

TestEstimate17::TestEstimate17() :
    Subprogram{"TestEstimate17", "Test Mustard::Math::Estimate (Section 17: Special Functions)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

constexpr auto sec14Smoke{[]<int K, CovarianceOption C>() {
    using Est = Estimate<K, C>;
    constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    [[maybe_unused]] typename Est::ValueType vec{};
    const auto newEst{[&] { if constexpr (K == 1) return Est{1.0, 0.5}; else return Est{x, cov}; }};
    auto est{newEst()};
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
    est = newEst();
    vec = logistic(est).Value();
    vec = erf(est).Value();
    vec = erfc(est).Value();
    vec = lgamma(est).Value();
    vec = ndtri(est).Value();
}};

constexpr auto sec14Special{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    // --- Logistic: f(x)=1/(1+exp(-x)), J=diag(f*(1-f)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.LogisticInPlace();
        if constexpr (K == 1) {
            auto expected{1.0 / (1.0 + std::exp(-x(0)))};
            CheckClose(e.Value(), expected, "17a: Logistic value");
            auto jac{expected * (1.0 - expected)};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "17a: Logistic var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                auto expected{1.0 / (1.0 + std::exp(-x(i)))};
                CheckClose(e.Value(i), expected, fmt::format("17a: Logistic value({})", i));
            }
            CheckMathFunction(e, cov, "17a: Logistic",
                              [&](int i) { auto y{1.0 / (1.0 + std::exp(-x(i)))}; return y * (1.0 - y); });
        }

        const auto eLogistic{logistic(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(eLogistic.Value(), 1.0 / (1.0 + std::exp(-x(0))), "17b: logistic() free value");
        } else {
            CheckClose(eLogistic.Value(0), 1.0 / (1.0 + std::exp(-x(0))), "17b: logistic() free value");
        }
    }

    // --- Erf: f(x)=erf(x), J=diag(2/sqrt(pi)*exp(-x^2)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.ErfInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::erf(x(0)), "17c: Erf value");
            auto jac{2.0 / std::sqrt(std::numbers::pi) * std::exp(-x(0) * x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "17c: Erf var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::erf(x(i)), fmt::format("17c: Erf value({})", i));
            }
            CheckMathFunction(e, cov, "17c: Erf",
                              [&](int i) { return 2.0 / std::sqrt(std::numbers::pi) * std::exp(-x(i) * x(i)); });
        }

        const auto eErf{erf(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(eErf.Value(), std::erf(x(0)), "17d: erf() free value");
        } else {
            CheckClose(eErf.Value(0), std::erf(x(0)), "17d: erf() free value");
        }
    }

    // --- Erfc: f(x)=erfc(x), J=diag(-2/sqrt(pi)*exp(-x²)) ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.ErfcInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), std::erfc(x(0)), "17e: Erfc value");
            auto jac{-2.0 / std::sqrt(std::numbers::pi) * std::exp(-x(0) * x(0))};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "17e: Erfc var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::erfc(x(i)), fmt::format("17e: Erfc value({})", i));
            }
            CheckMathFunction(e, cov, "17e: Erfc",
                              [&](int i) { return -2.0 / std::sqrt(std::numbers::pi) * std::exp(-x(i) * x(i)); });
        }

        const auto eErfc{erfc(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(eErfc.Value(), std::erfc(x(0)), "17f: erfc() free value");
        } else {
            CheckClose(eErfc.Value(0), std::erfc(x(0)), "17f: erfc() free value");
        }
    }

    // --- Lgamma ---
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};
        e.LgammaInPlace();
        if constexpr (K == 1) {
            CheckClose(e.Value(), 0.0, "17g: Lgamma(1)=0");
        } else {
            CheckClose(e.Value(0), 0.0, "17g: Lgamma(1)=0");
        }

        const auto eLgamma{lgamma(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(eLgamma.Value(), 0.0, "17h: lgamma() free value");
        } else {
            CheckClose(eLgamma.Value(0), 0.0, "17h: lgamma() free value");
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
            CheckClose(e.Value(), 0.0, "17i: Ndtri(0.5)=0");
            auto jac{std::sqrt(2.0 * std::numbers::pi)};
            CheckClose(e.Variance(), jac * jac * cov.diagonal()(0), "17i: Ndtri var");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), 0.0, fmt::format("17i: Ndtri(0.5)=0 at ({})", i));
            }
            CheckMathFunction(e, cov, "17i: Ndtri",
                              [](int) { return std::sqrt(2.0 * std::numbers::pi); });
        }

        const auto e2{ndtri(MakeEstimate<K, C>(x, cov))};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), 0.0, "17j: ndtri() free value");
        } else {
            CheckClose(e2.Value(0), 0.0, "17j: ndtri() free value");
        }
    }
}};

} // namespace TestEstimateSection

auto TestEstimate17::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 17: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec14Smoke);
    PrintLn("  17 smoke passed: special functions compilation smoke");

    PrintLn("--- Section 17: Special Functions ---");
    RunOverAllDims<AllStaticDims>(sec14Special);
    PrintLn("  17 passed: special functions (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestEstimate17 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
