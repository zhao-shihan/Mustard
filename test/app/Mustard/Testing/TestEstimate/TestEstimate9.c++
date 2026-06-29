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

TestEstimate9::TestEstimate9() :
    Subprogram{"TestEstimate9", "Test Mustard::Math::Estimate (Section 9: Dot and Matrix Ops)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Section 6 Smoke: Dot Product and Matrix-Vector Multiply (from sec0Smoke)
// =========================================================================

constexpr auto sec6Smoke{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        using Est = Estimate<K, C>;
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        constexpr bool isDynamic{K == Eigen::Dynamic};

        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};

        [[maybe_unused]] double scalar{};

        const auto newEst{[&] {
            if constexpr (K == 1) {
                return Est{1.0, 0.5};
            } else {
                return Est{x, cov};
            }
        }};

        Est e3 = newEst();
        auto est{newEst()};

        // ---- Dot product ----
        scalar = e3.Dot(est).Value();
        scalar = e3.Dot(x).Value();
        scalar = Dot(e3, est).Value();
        scalar = Dot(e3, x).Value();
        scalar = Dot(x, e3).Value();

        // ---- matrix-vector multiply ----
        if constexpr (isDynamic) {
            Eigen::MatrixXd matA{dim, dim};
            matA.setIdentity();
            Eigen::MatrixXd matRect{dim + 1, dim};
            matRect.setOnes();
            auto resultMatA{e3.RightMultiply(matA)};
            e3 = newEst();
            resultMatA = e3 * matA;
            e3 = newEst();
            auto resultMatRect{matRect * e3};
            resultMatRect = e3.LeftMultiply(matRect);
        } else {
            Eigen::Matrix<double, K, K> matA;
            matA.setIdentity();
            Eigen::Matrix<double, K + 1, K> matRect;
            matRect.setOnes();
            auto resultMatA{e3.RightMultiply(matA)};
            e3 = newEst();
            resultMatA = e3 * matA;
            e3 = newEst();
            auto resultMatRect{matRect * e3};
            resultMatRect = e3.LeftMultiply(matRect);
        }
    }
}};

// =========================================================================
// Section 6 Smoke CrossType: Cross Dot and Matrix-Vector Multiply (from sec0Smoke)
// =========================================================================

constexpr auto sec6SmokeCrossType{[] {
    [[maybe_unused]] double scalar{};
    constexpr auto full{CovarianceOption::Full};
    constexpr auto diag{CovarianceOption::Diagonal};
    constexpr int dyn{Eigen::Dynamic};

    // A: Same K, different C (K=3, Full ↔ Diag)
    {
        const auto x3{MakeTestValue<3, full>(3)};
        const auto cov3f{MakeTestCov<3, full>(3)};
        const auto cov3d{MakeTestCov<3, diag>(3)};
        Estimate<3, full> ef3{x3, cov3f};
        Estimate<3, diag> ed3{x3, cov3d};
        scalar = ef3.Dot(ed3).Value();
        scalar = Dot(ef3, ed3).Value();
        Eigen::Matrix3d matA;
        matA.setIdentity();
        Eigen::Matrix<double, 4, 3> matRect;
        matRect.setOnes();
        auto matResultF{ef3.RightMultiply(matA)};
        auto matResultD{ed3.RightMultiply(matA)};
        matResultF = ef3 * matA;
        matResultD = ed3 * matA;
        auto mrF{matRect * ef3};
        mrF = ef3.LeftMultiply(matRect);
        auto mrD{matRect * ed3};
        mrD = ed3.LeftMultiply(matRect);
    }

    // B: Static ↔ Dynamic, same C
    {
        const auto x3{MakeTestValue<3, full>(3)};
        const auto cov3f{MakeTestCov<3, full>(3)};
        const auto cov3d{MakeTestCov<3, diag>(3)};
        const auto covDynF3{MakeTestCov<dyn, full>(3)};
        const auto covDynD3{MakeTestCov<dyn, diag>(3)};
        const auto xDyn3{MakeTestValue<dyn, full>(3)};
        Estimate<3, full> e3f{x3, cov3f};
        Estimate<dyn, full> edf3{xDyn3, covDynF3};
        scalar = e3f.Dot(edf3).Value();
        scalar = Dot(e3f, edf3).Value();
        Estimate<3, diag> e3d{MakeTestValue<3, diag>(3), cov3d};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};
        scalar = e3d.Dot(edd3).Value();
        scalar = Dot(e3d, edd3).Value();
    }

    // C: Dynamic ↔ Dynamic
    {
        const auto xDyn3{MakeTestValue<dyn, full>(3)};
        const auto covDynF3{MakeTestCov<dyn, full>(3)};
        const auto covDynD3{MakeTestCov<dyn, diag>(3)};
        Estimate<dyn, full> edf3{xDyn3, covDynF3};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};
        scalar = edf3.Dot(edd3).Value();
        scalar = Dot(edf3, edd3).Value();
    }

    // D: Static ↔ Dynamic, different C
    {
        const auto x3{MakeTestValue<3, full>(3)};
        const auto cov3f{MakeTestCov<3, full>(3)};
        const auto cov3d{MakeTestCov<3, diag>(3)};
        const auto xDyn3{MakeTestValue<dyn, full>(3)};
        const auto covDynF3{MakeTestCov<dyn, full>(3)};
        const auto covDynD3{MakeTestCov<dyn, diag>(3)};
        Estimate<3, full> e3f{x3, cov3f};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};
        scalar = e3f.Dot(edd3).Value();
        scalar = Dot(e3f, edd3).Value();
        Estimate<3, diag> e3d{MakeTestValue<3, diag>(3), cov3d};
        Estimate<dyn, full> edf3{xDyn3, covDynF3};
        scalar = e3d.Dot(edf3).Value();
        scalar = Dot(e3d, edf3).Value();
    }
}};

// =========================================================================
// Section 6: Dot Product
// =========================================================================

constexpr auto sec6DotProduct{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};

        Est e1{v, cov};
        Est e2{v, cov};

        // e1.Dot(e2)
        {
            const auto dotResult{e1.Dot(e2)};
            auto expectedValue{0.0};
            for (auto i{0}; i < dim; ++i) {
                expectedValue += v(i) * v(i);
            }
            CheckClose(dotResult.Value(), expectedValue, "9a: Dot value");
            auto expectedVar{0.0};
            for (auto i{0}; i < dim; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    expectedVar += v(i) * e1.Covariance(i, j) * v(j);
                }
            }
            expectedVar *= 2.0; // both sides contribute equally
            CheckClose(dotResult.Variance(), expectedVar, "9a: Dot variance");
        }

        // e.Dot(vector)
        {
            const auto dotResult{e1.Dot(v)};
            auto expectedValue{0.0};
            for (auto i{0}; i < dim; ++i) {
                expectedValue += v(i) * v(i);
            }
            CheckClose(dotResult.Value(), expectedValue, "9c: Dot with vector value");
            auto expectedVar{0.0};
            for (auto i{0}; i < dim; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    expectedVar += v(i) * e1.Covariance(i, j) * v(j);
                }
            }
            CheckClose(dotResult.Variance(), expectedVar, "9c: Dot with vector variance");
        }

        // Free function Dot
        {
            const auto dotResult{Dot(e1, e2)};
            auto expectedValue{0.0};
            for (auto i{0}; i < dim; ++i) {
                expectedValue += v(i) * v(i);
            }
            CheckClose(dotResult.Value(), expectedValue, "9d: Free Dot value");

            const auto dotResult2{Dot(e1, v)};
            CheckClose(dotResult2.Value(), expectedValue, "9e: Free Dot(e,v) value");

            const auto dotResult3{Dot(v, e2)};
            CheckClose(dotResult3.Value(), expectedValue, "9e: Free Dot(v,e) value");
        }

        // Result type is Estimate<1, C>
        static_assert(std::is_same_v<decltype(e1.Dot(e2)), Estimate<1, C>>);
    }
}};

// =========================================================================
// Section 6h: Matrix-vector RightMultiply — square and non-square
// =========================================================================

constexpr auto sec8hRightMultiply{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};

        // ---- RightMultiply with square matrix ----
        {
            Eigen::Matrix<double, K, K> matA;
            if constexpr (K == Eigen::Dynamic) {
                matA.resize(dim, dim);
            }
            matA.setIdentity();

            Est e{v, cov};
            auto result{e.RightMultiply(matA)};
            // result should be Estimate<K, C> (same dimension)
            for (auto i{0}; i < dim; ++i) {
                CheckClose(result.Value(i), v(i),
                           fmt::format("9h: RightMultiply sq Value({})", i));
            }
        }

        // ---- RightMultiply with non-square matrix (K × (K+1)) ----
        {
            constexpr int M{K == Eigen::Dynamic ? Eigen::Dynamic : K + 1};
            constexpr int actualM{(K == Eigen::Dynamic) ? dim + 1 : K + 1};

            Eigen::Matrix<double, K, M> matRect;
            if constexpr (K == Eigen::Dynamic or M == Eigen::Dynamic) {
                matRect.resize(dim, actualM);
            }
            for (auto i{0}; i < dim; ++i) {
                for (auto j{0}; j < actualM; ++j) {
                    matRect(i, j) = (i + 1) * 10 + (j + 1);
                }
            }

            Est e{v, cov};
            auto result{e.RightMultiply(matRect)};
            CheckEq(result.Dimension(), actualM, "9h: RightMultiply non-sq dim");

            // expected value = A^T * v
            for (auto j{0}; j < actualM; ++j) {
                auto expectedVal{0.0};
                for (auto i{0}; i < dim; ++i) {
                    expectedVal += matRect(i, j) * v(i);
                }
                CheckClose(result.Value(j), expectedVal,
                           fmt::format("9h: RightMultiply non-sq Value({})", j));
            }

            // expected covariance = A^T * Cov * A
            for (auto j1{0}; j1 < actualM; ++j1) {
                for (auto j2{0}; j2 < actualM; ++j2) {
                    auto expectedCov{0.0};
                    for (auto p{0}; p < dim; ++p) {
                        for (auto q{0}; q < dim; ++q) {
                            auto cov_pq{[&] {
                                if constexpr (C == CovarianceOption::Full) {
                                    return cov(p, q);
                                } else {
                                    return (p == q) ? cov.diagonal()(p) : 0.0;
                                }
                            }()};
                            expectedCov += matRect(p, j1) * cov_pq * matRect(q, j2);
                        }
                    }
                    if constexpr (C == CovarianceOption::Full) {
                        CheckClose(result.Covariance(j1, j2), expectedCov,
                                   fmt::format("9h: RightMultiply non-sq Cov({},{})", j1, j2));
                    } else {
                        if (j1 == j2) {
                            CheckClose(result.Covariance(j1, j2), expectedCov,
                                       fmt::format("9h: RightMultiply non-sq Var({})", j1));
                        }
                    }
                }
            }
        }
    }
}};

// =========================================================================
// Section 6i: Matrix-vector operator*
// =========================================================================

constexpr auto sec8iMatrixMultiplyOperator{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};

        // ---- operator*(est, matrix) ----
        {
            Eigen::Matrix<double, K, K> matA;
            if constexpr (K == Eigen::Dynamic) {
                matA.resize(dim, dim);
            }
            matA.setIdentity();

            Est e{v, cov};
            auto r1{e * matA};
            auto r2{e.RightMultiply(matA)};
            for (auto i{0}; i < dim; ++i) {
                CheckClose(r1.Value(i), r2.Value(i),
                           fmt::format("9i: operator*(est, mat) Value({})", i));
            }
        }

        // ---- operator*(matrix, est) (A is (K+1)×K) ----
        {
            constexpr int M{K == Eigen::Dynamic ? Eigen::Dynamic : K + 1};
            constexpr int actualM{(K == Eigen::Dynamic) ? dim + 1 : K + 1};

            Eigen::Matrix<double, M, K> matRect;
            if constexpr (K == Eigen::Dynamic or M == Eigen::Dynamic) {
                matRect.resize(actualM, dim);
            }
            for (auto i{0}; i < actualM; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    matRect(i, j) = (i + 1) * 10 + (j + 1);
                }
            }

            Est e{v, cov};
            auto r1{matRect * e};
            auto r2{e.LeftMultiply(matRect)};
            for (auto j{0}; j < actualM; ++j) {
                CheckClose(r1.Value(j), r2.Value(j),
                           fmt::format("9i: operator*(mat, est) Value({})", j));
            }
        }
    }
}};

// =========================================================================
// Section 6j: Matrix-vector multiply — dimension mismatch throws
// =========================================================================

constexpr auto sec8jRightMultiplyException{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{v, cov};

        constexpr bool isDynamic{K == Eigen::Dynamic};

        // Row mismatch on RightMultiply
        if constexpr (isDynamic) {
            auto threw{false};
            try {
                Eigen::MatrixXd bad{2, 2};
                bad.setIdentity();
                e.RightMultiply(bad);
            } catch (const std::invalid_argument&) { threw = true; }
            if (not threw) {
                Throw<std::runtime_error>("9j: RightMultiply row mismatch should throw");
            }

            threw = false;
            try {
                Eigen::MatrixXd bad{3, 2};
                bad.setIdentity();
                e.LeftMultiply(bad);
            } catch (const std::invalid_argument&) { threw = true; }
            if (not threw) {
                Throw<std::runtime_error>("9j: LeftMultiply col mismatch should throw");
            }
        }
    }
}};

// =========================================================================
// Section 6k: Matrix-vector LeftMultiply — square and non-square
// =========================================================================

constexpr auto sec8kLeftMultiply{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};

        // ---- LeftMultiply with square matrix ----
        {
            // construct anti-diagonal matrix A: A(i,j) = (i == dim-1-j) ? 2.0 : 0.0
            Eigen::Matrix<double, K, K> matA;
            if constexpr (K == Eigen::Dynamic) {
                matA.resize(dim, dim);
            }
            matA.setZero();
            for (auto i{0}; i < dim; ++i) {
                matA(i, dim - 1 - i) = 2.0;
            }

            // expected value = A * v
            Eigen::Vector<double, K> expectedValue;
            if constexpr (K == Eigen::Dynamic) {
                expectedValue.resize(dim);
            }
            expectedValue.setZero();
            for (auto i{0}; i < dim; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    expectedValue(i) += matA(i, j) * v(j);
                }
            }

            Est e{v, cov};
            auto result{e.LeftMultiply(matA)};

            for (auto i{0}; i < dim; ++i) {
                CheckClose(result.Value(i), expectedValue(i),
                           fmt::format("9k: LeftMultiply sq Value({})", i));
            }

            // expected covariance = A * Cov * A^T
            for (auto i{0}; i < dim; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    auto expectedCov{0.0};
                    for (auto p{0}; p < dim; ++p) {
                        for (auto q{0}; q < dim; ++q) {
                            auto cov_pq{[&] {
                                if constexpr (C == CovarianceOption::Full) {
                                    return cov(p, q);
                                } else {
                                    return (p == q) ? cov.diagonal()(p) : 0.0;
                                }
                            }()};
                            expectedCov += matA(i, p) * cov_pq * matA(j, q);
                        }
                    }
                    if constexpr (C == CovarianceOption::Full) {
                        CheckClose(result.Covariance(i, j), expectedCov,
                                   fmt::format("9k: LeftMultiply sq Cov({},{})", i, j));
                    } else {
                        if (i == j) {
                            CheckClose(result.Covariance(i, j), expectedCov,
                                       fmt::format("9k: LeftMultiply sq Var({})", i));
                        }
                    }
                }
            }
        }

        // ---- LeftMultiply with non-square matrix ((K+1) × K) ----
        {
            constexpr int M{K == Eigen::Dynamic ? Eigen::Dynamic : K + 1};
            constexpr int actualM{(K == Eigen::Dynamic) ? dim + 1 : K + 1};

            Eigen::Matrix<double, M, K> matRect;
            if constexpr (K == Eigen::Dynamic or M == Eigen::Dynamic) {
                matRect.resize(actualM, dim);
            }
            for (auto i{0}; i < actualM; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    matRect(i, j) = (i + 1) * 10 + (j + 1);
                }
            }

            Est e{v, cov};
            auto result{e.LeftMultiply(matRect)};
            CheckEq(result.Dimension(), actualM, "9k: LeftMultiply non-sq dim");

            // expected value = A * v
            for (auto i{0}; i < actualM; ++i) {
                auto expectedVal{0.0};
                for (auto j{0}; j < dim; ++j) {
                    expectedVal += matRect(i, j) * v(j);
                }
                CheckClose(result.Value(i), expectedVal,
                           fmt::format("9k: LeftMultiply non-sq Value({})", i));
            }

            // expected covariance = A * Cov * A^T
            for (auto i1{0}; i1 < actualM; ++i1) {
                for (auto i2{0}; i2 < actualM; ++i2) {
                    auto expectedCov{0.0};
                    for (auto p{0}; p < dim; ++p) {
                        for (auto q{0}; q < dim; ++q) {
                            auto cov_pq{[&] {
                                if constexpr (C == CovarianceOption::Full) {
                                    return cov(p, q);
                                } else {
                                    return (p == q) ? cov.diagonal()(p) : 0.0;
                                }
                            }()};
                            expectedCov += matRect(i1, p) * cov_pq * matRect(i2, q);
                        }
                    }
                    if constexpr (C == CovarianceOption::Full) {
                        CheckClose(result.Covariance(i1, i2), expectedCov,
                                   fmt::format("9k: LeftMultiply non-sq Cov({},{})", i1, i2));
                    } else {
                        if (i1 == i2) {
                            CheckClose(result.Covariance(i1, i2), expectedCov,
                                       fmt::format("9k: LeftMultiply non-sq Var({})", i1));
                        }
                    }
                }
            }
        }
    }
}};

} // namespace TestEstimateSection

auto TestEstimate9::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 9: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec6Smoke);
    PrintLn("  9 smoke passed: Dot and matrix-vector multiply compilation smoke");
    sec6SmokeCrossType();
    PrintLn("  9 smoke cross-type passed: cross Dot and matrix compilation smoke");

    PrintLn("--- Section 9: Dot Product ---");
    RunOverAllDims<AllStaticDims>(sec6DotProduct);
    PrintLn("  9 passed: dot product (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 6h: Matrix-vector RightMultiply ---");
    RunOverAllDims<AllStaticDims>(sec8hRightMultiply);
    PrintLn("  9h passed: RightMultiply (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 6i: Matrix-vector operator* ---");
    RunOverAllDims<AllStaticDims>(sec8iMatrixMultiplyOperator);
    PrintLn("  9i passed: operator* (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 6j: Matrix-vector multiply — exception ---");
    RunOverAllDims<AllStaticDims>(sec8jRightMultiplyException);
    PrintLn("  9j passed: RightMultiply/LeftMultiply exception (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 6k: Matrix-vector LeftMultiply ---");
    RunOverAllDims<AllStaticDims>(sec8kLeftMultiply);
    PrintLn("  9k passed: LeftMultiply (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestEstimate9 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
