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

TestEstimate20::TestEstimate20() :
    Subprogram{"TestEstimate20", "Test Mustard::Math::Estimate (Section 20: Edge Cases)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Section 17: Edge Cases and Numerical Stress
// =========================================================================

constexpr auto sec17EdgeCases{[]<int K, CovarianceOption C>() {
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
            CheckClose(e.Value(), x(0), "17: Self-assign Value()");
        } else {
            CheckClose(e.Value(0), x(0), "17: Self-assign Value(0)");
            if constexpr (isFull and dim >= 2) {
                CheckClose(e.Covariance(0, 1), cov(0, 1), "17: Self-assign Cov(0,1)");
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
                    Throw<std::runtime_error>("17: RelUnc(0,var>0) should be inf for K=1");
                }
            }
            // x=0, var=0
            {
                Estimate<K, C> eZeroBoth;
                if (not std::isnan(eZeroBoth.RelativeUncertainty())) {
                    Throw<std::runtime_error>("17: RelUnc(0,var=0) should be NaN for K=1");
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
                    Throw<std::runtime_error>("17: RelUnc(0,var>0) should be inf");
                }
                // other components also inf since all x=0
                if constexpr (dim >= 2) {
                    if (not std::isinf(eZero.RelativeUncertainty(1))) {
                        Throw<std::runtime_error>("17: RelUnc(1,var>0) should be inf");
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
                    Throw<std::runtime_error>("17: RelUnc(0,var=0) should be NaN");
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
                    Throw<std::runtime_error>("17: RelUnc should be finite for non-zero value");
                }
            } else {
                if (not std::isfinite(e.RelativeUncertainty(0))) {
                    Throw<std::runtime_error>("17: RelUnc should be finite for non-zero value");
                }
            }
        }
    }
}};

// Exp(Log(x)) roundtrip — Jacobian chain composes to identity
constexpr auto sec17ExpLogRoundtrip{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    auto e{MakeEstimate<K, C>(x, cov)};
    e.LogInPlace();
    e.ExpInPlace();
    if constexpr (K == 1) {
        CheckClose(e.Value(), x(0), "17: Exp(Log(x)) Value");
    } else {
        for (auto i{0}; i < dim; ++i) {
            CheckClose(e.Value(i), x(i), fmt::format("17: Exp(Log(x)) Value({})", i));
        }
    }
}};

// =========================================================================
// Section 17c: Correlation — zero-variance edge case
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

        CheckClose(e.Correlation(0, 0), 1.0, "20c: Corr(0,0)=1 even Var=0");
        if constexpr (dim >= 2) {
            if (not std::isnan(e.Correlation(0, 1))) {
                Throw<std::runtime_error>("20c: Corr(0,1) should be NaN when Var=0");
            }
            if (not std::isnan(e.Correlation(1, 0))) {
                Throw<std::runtime_error>("20c: Corr(1,0) should be NaN when Var=0");
            }
            CheckClose(e.Correlation(1, 1), 1.0, "20c: Corr(1,1)=1");
        }
    }
}};

} // namespace TestEstimateSection

auto TestEstimate20::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 20: Edge Cases and Numerical Stress ---");
    RunOverAllDims<AllStaticDims>(sec17EdgeCases);
    PrintLn("  20 passed: edge cases and numerical stress (K=1,2,3,5,10 Full/Diag + dynamic)");
    RunOverAllDims<AllStaticDims>(sec17ExpLogRoundtrip);
    PrintLn("  20 passed: exponential/logarithmic round-trip (K=1,2,3,5,10 Full/Diag + dynamic)");
    RunOverAllDims<AllStaticDims>(sec19cCorrZeroVar);
    PrintLn("  20 passed: correlation zero-variance edge case (K=2,3,5,10 Full + dynamic)");

    PrintLn("All TestEstimate20 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
