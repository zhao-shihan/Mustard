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

#include "Eigen/LU"

#include <cstdlib>

namespace Mustard::Testing {

TestEstimate33::TestEstimate33() :
    Subprogram{"TestEstimate33", "Test Mustard::Math::Estimate (Section 33: Combine)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Section 30: Combine — inverse-covariance-weighted combination (BLUE)
// =========================================================================

constexpr auto sec30Combine{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
    using Est = Estimate<K, C>;

    const auto v1{MakeTestValue<K, C>(dim)};
    const auto cov1{MakeTestCov<K, C>(dim)};
    Est e1{MakeEstimate<K, C>(v1, cov1)};

    if constexpr (K == 1) {
        // 32a. Two identical estimates → value unchanged, variance halved
        {
            Est e2{v1(0), cov1.diagonal()(0)};
            const auto result{e1.Combine(e2)};
            CheckClose(result.Value(), v1(0), "33a: Combine identical scalars — value unchanged");
            CheckClose(result.Variance(), cov1.diagonal()(0) / 2.0, "33a: Combine identical scalars — variance halved");
        }

        // 32b. Two different estimates
        {
            Est e2{v1(0) + 1.0, 0.9};
            const auto result{e1.Combine(e2)};
            const double invVar1{1.0 / cov1.diagonal()(0)};
            const double invVar2{1.0 / 0.9};
            const double expectedVar{1.0 / (invVar1 + invVar2)};
            const double expectedVal{expectedVar * (invVar1 * v1(0) + invVar2 * (v1(0) + 1.0))};
            CheckClose(result.Value(), expectedVal, "33b: Combine different scalars — value");
            CheckClose(result.Variance(), expectedVar, "33b: Combine different scalars — variance");
        }

        // 32c. Self-combine via move (must run before CombineInPlace which mutates e1)
        {
            Est e2{v1(0), cov1.diagonal()(0)};
            const auto result{std::move(e1).Combine(e2)};
            CheckClose(result.Variance(), cov1.diagonal()(0) / 2.0, "33c: Combine(move, const&) — variance halved");
        }

        // 32d. CombineInPlace
        {
            Est e1b{v1(0), cov1.diagonal()(0)};
            Est e2{v1(0) + 1.0, 0.9};
            e1b.CombineInPlace(e2);
            const double invVar1{1.0 / cov1.diagonal()(0)};
            const double invVar2{1.0 / 0.9};
            const double expectedVar{1.0 / (invVar1 + invVar2)};
            const double expectedVal{expectedVar * (invVar1 * v1(0) + invVar2 * (v1(0) + 1.0))};
            CheckClose(e1b.Value(), expectedVal, "33d: CombineInPlace scalars — value");
            CheckClose(e1b.Variance(), expectedVar, "33d: CombineInPlace scalars — variance");
        }
    } else {
        // 32a. Two identical Full/Diagonal estimates → value unchanged, variance halved
        {
            Est e2{MakeEstimate<K, C>(v1, cov1)};
            const auto result{e1.Combine(e2)};
            for (auto i{0}; i < dim; ++i) {
                CheckClose(result.Value(i), v1(i), "33a: Combine identical — value unchanged");
                CheckClose(result.Variance(i), e1.Variance(i) / 2.0, "33a: Combine identical — variance halved");
            }
            if constexpr (C == CovarianceOption::Full) {
                for (auto i{0}; i < dim; ++i) {
                    for (auto j{0}; j < dim; ++j) {
                        CheckClose(result.Covariance(i, j), e1.Covariance(i, j) / 2.0, "33a: Combine identical — covariance halved");
                    }
                }
            }
        }

        // 32b. Two different estimates — verify against BLUE formula
        {
            // Create a second estimate with different values and covariances
            const auto v2{MakeTestValue<K, C>(dim)};
            for (auto i{0}; i < dim; ++i) {
                const_cast<double&>(v2(i)) = v1(i) + 1.0;
            }
            const auto cov2{MakeTestCov<K, C>(dim)};
            Est e2{MakeEstimate<K, C>(v2, cov2)};
            const auto result{e1.Combine(e2)};

            if constexpr (C == CovarianceOption::Full) {
                // Compute expected via full matrix BLUE formula
                Eigen::MatrixXd cov1Full{dim, dim};
                Eigen::MatrixXd cov2Full{dim, dim};
                for (auto i{0}; i < dim; ++i) {
                    for (auto j{0}; j < dim; ++j) {
                        cov1Full(i, j) = e1.Covariance(i, j);
                        cov2Full(i, j) = e2.Covariance(i, j);
                    }
                }
                const auto inv1{cov1Full.inverse()};
                const auto inv2{cov2Full.inverse()};
                const auto expectedCov{(inv1 + inv2).inverse()};
                const Eigen::VectorXd expectedVal{expectedCov * (inv1 * e1.Value() + inv2 * e2.Value())};
                for (auto i{0}; i < dim; ++i) {
                    CheckClose(result.Value(i), expectedVal(i), "33b: Combine different Full — value");
                }
                for (auto i{0}; i < dim; ++i) {
                    for (auto j{0}; j < dim; ++j) {
                        CheckClose(result.Covariance(i, j), expectedCov(i, j), "33b: Combine different Full — covariance");
                    }
                }
            } else {
                // Diagonal: compute expected per-component
                for (auto i{0}; i < dim; ++i) {
                    const double invVar1{1.0 / e1.Variance(i)};
                    const double invVar2{1.0 / e2.Variance(i)};
                    const double expectedVar{1.0 / (invVar1 + invVar2)};
                    const double expectedVal{expectedVar * (invVar1 * v1(i) + invVar2 * v2(i))};
                    CheckClose(result.Value(i), expectedVal, "33b: Combine different Diag — value");
                    CheckClose(result.Variance(i), expectedVar, "33b: Combine different Diag — variance");
                }
            }
        }

        // 32c. CombineInPlace
        {
            const auto v2{MakeTestValue<K, C>(dim)};
            for (auto i{0}; i < dim; ++i) {
                const_cast<double&>(v2(i)) = v1(i) + 1.0;
            }
            const auto cov2{MakeTestCov<K, C>(dim)};
            Est e2{MakeEstimate<K, C>(v2, cov2)};
            Est eCopy{e1};
            eCopy.CombineInPlace(e2);
            const auto expected{e1.Combine(e2)};
            for (auto i{0}; i < dim; ++i) {
                CheckClose(eCopy.Value(i), expected.Value(i), "33c: CombineInPlace — value matches member function");
            }
        }

        // 32d. Self-combine via move
        {
            Est e2{MakeEstimate<K, C>(v1, cov1)};
            const auto result{std::move(e1).Combine(e2)};
            for (auto i{0}; i < dim; ++i) {
                CheckClose(result.Variance(i), e2.Variance(i) / 2.0, "33d: Combine(move, const&) — variance halved");
            }
        }
    }
}};

// =========================================================================
// Section 30b: Combine cross-type tests
// =========================================================================

auto sec32bCombineCrossType() -> void {
    constexpr int dim{3};
    using EstFF = Estimate<dim, CovarianceOption::Full>;
    using EstFD = Estimate<dim, CovarianceOption::Diagonal>;
    using EstDF = Estimate<dim, CovarianceOption::Diagonal>;
    using EstDD = Estimate<dim, CovarianceOption::Diagonal>;

    const auto v{MakeTestValue<dim, CovarianceOption::Full>(dim)};
    const auto covFull{MakeTestCov<dim, CovarianceOption::Full>(dim)};
    const auto covDiag{MakeTestCov<dim, CovarianceOption::Diagonal>(dim)};

    // 32b-a. Combine(Full, Diagonal) → result is Full
    {
        EstFF eF{v, covFull};
        EstFD eD{v, covDiag};
        const auto result{eF.Combine(eD)};
        static_assert(std::same_as<decltype(result), const Estimate<dim, CovarianceOption::Full>>);
        // Verify against the BLUE formula: Σ = (Σ_F⁻¹ + diag(1/σ_D²))⁻¹
        Eigen::Matrix3d invFull;
        for (auto i{0}; i < dim; ++i) {
            for (auto j{0}; j < dim; ++j) {
                invFull(i, j) = eF.Covariance(i, j);
            }
        }
        invFull = invFull.inverse().eval();
        Eigen::Matrix3d invSum{invFull};
        for (auto i{0}; i < dim; ++i) {
            invSum(i, i) += 1.0 / eD.Variance(i);
        }
        const Eigen::Matrix3d expectedCov{invSum.inverse().eval()};
        const Eigen::Vector3d expectedVal{expectedCov * (invFull * eF.Value() + eD.Value().cwiseQuotient(eD.Variance()))};
        for (auto i{0}; i < dim; ++i) {
            CheckClose(result.Value(i), expectedVal(i), "33b-a: Combine(Full,Diag) value");
            CheckClose(result.Variance(i), expectedCov(i, i), "33b-a: Combine(Full,Diag) variance");
        }
    }

    // 32b-b. Combine(Diagonal, Full) → result is Full (via member function)
    {
        EstDF eD{v, covDiag};
        EstFF eF{v, covFull};
        const auto result{eD.Combine(eF)};
        static_assert(std::same_as<decltype(result), const Estimate<dim, CovarianceOption::Full>>);
        // Verify against BLUE: Σ = (diag(1/σ_D²) + Σ_F⁻¹)⁻¹
        Eigen::Matrix3d invFull;
        for (auto i{0}; i < dim; ++i) {
            for (auto j{0}; j < dim; ++j) {
                invFull(i, j) = eF.Covariance(i, j);
            }
        }
        invFull = invFull.inverse().eval();
        Eigen::Matrix3d invSum{invFull};
        for (auto i{0}; i < dim; ++i) {
            invSum(i, i) += 1.0 / eD.Variance(i);
        }
        const Eigen::Matrix3d expectedCov{invSum.inverse().eval()};
        const Eigen::Vector3d expectedVal{expectedCov * (eD.Value().cwiseQuotient(eD.Variance().eval()) + invFull * eF.Value())};
        // Note: eD.Variance().eval() needed for cwiseQuotient
        for (auto i{0}; i < dim; ++i) {
            CheckClose(result.Value(i), expectedVal(i), "33b-b: Combine(Diag,Full) value");
            CheckClose(result.Variance(i), expectedCov(i, i), "33b-b: Combine(Diag,Full) variance");
        }
    }

    // 32b-c. Combine(Diagonal, Diagonal) → result is Diagonal
    {
        EstDD eD1{v, covDiag};
        EstDD eD2{v, covDiag};
        const auto result{eD1.Combine(eD2)};
        static_assert(std::same_as<decltype(result), const Estimate<dim, CovarianceOption::Diagonal>>);
        for (auto i{0}; i < dim; ++i) {
            CheckClose(result.Variance(i), eD1.Variance(i) / 2.0, "33b-c: Combine(Diag,Diag) variance halved");
        }
    }
};

} // namespace TestEstimateSection

auto TestEstimate33::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 33: Combine ---");
    RunOverAllDims<AllStaticDims>(sec30Combine);
    PrintLn("  33 passed: Combine (K=1,2,3,5,10 Full/Diag + dynamic)");
    sec32bCombineCrossType();
    PrintLn("  33b passed: Combine cross-type");

    PrintLn("All TestEstimate33 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
