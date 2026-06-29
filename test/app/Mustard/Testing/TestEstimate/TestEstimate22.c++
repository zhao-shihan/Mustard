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

TestEstimate22::TestEstimate22() :
    Subprogram{"TestEstimate22", "Test Mustard::Math::Estimate (Section 22: Normalize)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Section 19: Normalize
// =========================================================================

constexpr auto sec19Normalize{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};

        // 21a. Normalized() const& — value has unit norm
        {
            Est e{v, cov};
            const auto result{e.Normalized()};
            CheckClose(result.Value().norm(), 1.0, "22a: Normalized() const& value has unit norm");
        }

        // 21b. Normalized() && — value has unit norm
        {
            Est e{v, cov};
            const auto result{std::move(e).Normalized()};
            CheckClose(result.Value().norm(), 1.0, "22b: Normalized() && value has unit norm");
        }

        // 21c. Normalize() & — in-place yields unit norm
        {
            Est e{v, cov};
            e.Normalize();
            CheckClose(e.Value().norm(), 1.0, "22c: Normalize() & value has unit norm");
        }

        // 21d. const& and && produce equivalent results
        {
            Est e1{v, cov};
            Est e2{v, cov};
            const auto r1{e1.Normalized()};
            const auto r2{std::move(e2).Normalized()};
            CheckClose(r1.Value(), r2.Value(), "22d: const& and && produce same value");
            CheckClose(r1.Covariance(), r2.Covariance(), "22d: const& and && produce same covariance");
        }

        // 21e. Covariance — verify against analytical Jacobian
        {
            Est e{v, cov};
            const auto mu{e.Value()};
            const auto sqNorm{mu.squaredNorm()};
            const auto norm{std::sqrt(sqNorm)};

            // Compute analytical Jacobian jac = (I - mu*mu^T / ||mu||^2) / ||mu||
            if constexpr (K == Eigen::Dynamic) {
                Eigen::MatrixXd jac{Eigen::MatrixXd::Identity(dim, dim) / norm - mu * mu.transpose() / (sqNorm * norm)};
                Eigen::MatrixXd expectedCov{jac * e.Covariance() * jac};
                const auto result{e.Normalized()};
                if constexpr (C == CovarianceOption::Full) {
                    CheckClose(result.Covariance(), expectedCov,
                               "22e: covariance matches analytical Jacobian (Full)");
                } else {
                    CheckClose(result.Variance(), expectedCov.diagonal(),
                               "22e: covariance matches analytical Jacobian (Diag)");
                }
            } else {
                Eigen::Matrix<double, K, K> jac{
                    decltype(jac)::Identity() / norm - mu * mu.transpose() / (sqNorm * norm)};
                Eigen::Matrix<double, K, K> expectedCov{jac * e.Covariance() * jac};
                const auto result{e.Normalized()};
                if constexpr (C == CovarianceOption::Full) {
                    CheckClose(result.Covariance(), expectedCov,
                               "22e: covariance matches analytical Jacobian (Full)");
                } else {
                    CheckClose(result.Variance(), expectedCov.diagonal(),
                               "22e: covariance matches analytical Jacobian (Diag)");
                }
            }
        }

        // 21f. Near-zero vector — Normalize is a no-op
        {
            auto zeroVec{MakeTestValue<K, C>(dim)};
            zeroVec.setZero();
            Est e{zeroVec, cov};
            const auto origVal{e.Value()};
            const auto origCov{e.Covariance()};
            e.Normalize();
            CheckClose(e.Value(), origVal, "22f: near-zero Normalize() leaves value unchanged");
            CheckClose(e.Covariance(), origCov, "22f: near-zero Normalize() leaves covariance unchanged");
        }

        // 21g. Chaining — Normalize().Sum() works
        {
            Est e{v, cov};
            const auto sum{e.Normalized().Sum()};
            // sum value should be close to sum of unit vector components
            const auto expectedSumVal{v.normalized().sum()};
            CheckClose(sum.Value(), expectedSumVal, "22g: Normalized().Sum() value");
        }
    }
}};

} // namespace TestEstimateSection

auto TestEstimate22::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 22: Normalize ---");
    RunOverAllDims<AllStaticDims>(sec19Normalize);
    PrintLn("  22 passed: normalize (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestEstimate22 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
