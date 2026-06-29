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

TestEstimate21::TestEstimate21() :
    Subprogram{"TestEstimate21", "Test Mustard::Math::Estimate (Section 21: Cross-Verification)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Section 18: Cross-Verification
// These are inherently dimension-comparison tests — keep as-is
// =========================================================================

constexpr auto sec18CrossVerification{[] {
    // 20b. Addition result matches manual computation
    {
        Eigen::Vector2d x1{1.0, 2.0};
        Eigen::Vector2d x2{3.0, 4.0};
        Eigen::Matrix2d cov1;
        cov1 << 0.4, 0.1, 0.1, 0.3;
        Eigen::Matrix2d cov2;
        cov2 << 0.9, 0.0, 0.0, 1.6;

        Estimate<2, CovarianceOption::Full> e1{x1, cov1};
        Estimate<2, CovarianceOption::Full> e2{x2, cov2};
        const auto e3{e1 + e2};

        CheckClose(e3.Value(), Eigen::Vector2d{4.0, 6.0}, "21b: manual add value");
        Eigen::Matrix2d expectedCov;
        expectedCov << 1.3, 0.1, 0.1, 1.9;
        CheckClose(e3.Covariance(), expectedCov, "21b: manual add cov");
    }

    // 20c. Dynamic matches static for same K
    {
        Eigen::Vector2d x{1.0, 2.0};
        Eigen::Matrix2d cov;
        cov << 0.4, 0.1, 0.1, 0.3;
        Estimate<2, CovarianceOption::Full> eStat{x, cov};

        Eigen::VectorXd xDyn{2};
        xDyn << 1.0, 2.0;
        Eigen::MatrixXd covDyn{2, 2};
        covDyn << 0.4, 0.1, 0.1, 0.3;
        Estimate<Eigen::Dynamic, CovarianceOption::Full> eDyn{xDyn, covDyn};

        eStat.SquareInPlace();
        eDyn.SquareInPlace();
        CheckClose(eStat.Value(0), eDyn.Value(0), "21c: Static vs Dynamic Square value");
        CheckClose(eStat.Covariance(0, 1), eDyn.Covariance(0, 1), "21c: Static vs Dynamic Square cov");
    }

    // 20d. Diagonal = Full with zero off-diagonals
    {
        Eigen::Vector2d x{2.0, 3.0};
        Eigen::Matrix2d covDiagZero;
        covDiagZero << 0.4, 0.0, 0.0, 0.9;
        Estimate<2, CovarianceOption::Full> eFull{x, covDiagZero};

        Eigen::Vector2d diag{0.4, 0.9};
        Estimate<2, CovarianceOption::Diagonal> eDiag{x, diag.asDiagonal()};

        eFull.SquareInPlace();
        eDiag.SquareInPlace();
        CheckClose(eFull.Value(0), eDiag.Value(0), "21d: Full vs Diag Square value");
        CheckClose(eFull.Variance(0), eDiag.Variance(0), "21d: Full vs Diag Square variance");
        CheckClose(eFull.Covariance(0, 1), 0.0, "21d: Full Cov(0,1)=0");
        CheckClose(eDiag.Covariance(0, 1), 0.0, "21d: Diag Cov(0,1)=0");
    }

    // 20e. Cross-CovarianceOption result type is Full when either is Full
    {
        Eigen::Vector2d x{1.0, 2.0};
        Eigen::Matrix2d covFull;
        covFull << 0.1, 0.0, 0.0, 0.1;
        Estimate<2, CovarianceOption::Full> eFull{x, covFull};

        Eigen::Vector2d diag{0.4, 0.9};
        Estimate<2, CovarianceOption::Diagonal> eDiag{x, diag.asDiagonal()};

        const auto eResult1{eFull + eDiag};
        static_assert(std::is_same_v<std::remove_const_t<decltype(eResult1)>, Estimate<2, CovarianceOption::Full>>);

        const auto eResult2{eDiag + eFull};
        static_assert(std::is_same_v<std::remove_const_t<decltype(eResult2)>, Estimate<2, CovarianceOption::Full>>);
    }
}};

} // namespace TestEstimateSection

auto TestEstimate21::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 21: Cross-Verification ---");
    sec18CrossVerification();
    PrintLn("  21 passed: cross-verification");

    PrintLn("All TestEstimate21 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
