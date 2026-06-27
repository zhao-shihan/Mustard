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

#include <cmath>
#include <cstdlib>

namespace Mustard::Testing {

TestEstimate36::TestEstimate36() :
    Subprogram{"TestEstimate36", "Test Mustard::Math::Estimate (Section 36: ProjTo, ProjFrom, Project)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Helper: manual covariance formula for ProjTo (RankUpdate×2)
// =========================================================================

constexpr auto manualProjToCov{[](const auto& x, const auto& y,
                              const auto& covX, const auto& covY) -> Eigen::MatrixXd {
    const auto yNormSq{y.squaredNorm()};
    const auto c{x.dot(y) / yNormSq};
    const auto invYSq{1. / yNormSq};
    Eigen::VectorXd v{(x - 2. * c * y) * invYSq};
    Eigen::VectorXd w{covY * v};
    const double beta{(y.transpose() * covX * y)(0, 0) / (yNormSq * yNormSq) + v.dot(w)};
    Eigen::MatrixXd cov{c * c * covY};
    cov.noalias() += beta * y * y.transpose();
    cov.noalias() += c * (y * w.transpose() + w * y.transpose());
    return cov;
}};

// Helper: build full cov matrix from Estimate accessors
constexpr auto toFullCov{[]<int K, CovarianceOption C>(const Estimate<K, C>& est, int dim) {
    Eigen::MatrixXd m(dim, dim);
    for (auto i{0}; i < dim; ++i) {
        for (auto j{0}; j < dim; ++j) {
            m(i, j) = est.Covariance(i, j);
        }
    }
    return m;
}};

// =========================================================================
// Section 36: Smoke Test — all call combinations
// =========================================================================

constexpr auto sec36Smoke{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;
        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{MakeEstimate<K, C>(v, cov)};

        // --- Same-type Estimate ---
        {
            Est e2{MakeEstimate<K, C>(v, cov)};
            // ProjTo
            [[maybe_unused]] auto r1{e.ProjToInPlace(e2)}; // & -> & reference
            [[maybe_unused]] auto r2{e.ProjTo(e2)};        // const& -> copy
            [[maybe_unused]] auto r3{
                Est{v, cov}
                .ProjTo(e2)
            }; // && -> move
            // ProjFrom
            [[maybe_unused]] auto r4{e.ProjFromInPlace(e2)}; // & -> & reference
            [[maybe_unused]] auto r5{e.ProjFrom(e2)};        // const& -> copy
            [[maybe_unused]] auto r6{
                Est{v, cov}
                .ProjFrom(e2)
            }; // && -> move
        }

        // --- Cross-type Estimate ---
        if constexpr (C == CovarianceOption::Full) {
            using EstD = Estimate<K, CovarianceOption::Diagonal>;
            auto covD{MakeTestCov<K, CovarianceOption::Diagonal>(dim)};
            EstD eCross{v, covD};
            [[maybe_unused]] auto r1{e.ProjToInPlace(eCross)};
            [[maybe_unused]] auto r2{e.ProjTo(eCross)};
            [[maybe_unused]] auto r3{
                Est{v, cov}
                .ProjTo(eCross)
            };
            [[maybe_unused]] auto r4{e.ProjFromInPlace(eCross)};
            [[maybe_unused]] auto r5{e.ProjFrom(eCross)};
            [[maybe_unused]] auto r6{
                Est{v, cov}
                .ProjFrom(eCross)
            };
        } else {
            using EstF = Estimate<K, CovarianceOption::Full>;
            auto covF{MakeTestCov<K, CovarianceOption::Full>(dim)};
            EstF eCross{v, covF};
            [[maybe_unused]] auto r1{e.ProjToInPlace(eCross)};
            [[maybe_unused]] auto r2{e.ProjTo(eCross)};
            [[maybe_unused]] auto r3{
                Est{v, cov}
                .ProjTo(eCross)
            };
            [[maybe_unused]] auto r4{e.ProjFromInPlace(eCross)};
            [[maybe_unused]] auto r5{e.ProjFrom(eCross)};
            [[maybe_unused]] auto r6{
                Est{v, cov}
                .ProjFrom(eCross)
            };
        }

        // --- Plain vector ---
        {
            auto vec{MakeTestValue<K, C>(dim)};
            [[maybe_unused]] auto r1{e.ProjToInPlace(vec)};
            [[maybe_unused]] auto r2{e.ProjTo(vec)};
            [[maybe_unused]] auto r3{
                Est{v, cov}
                .ProjTo(vec)
            };
            [[maybe_unused]] auto r4{e.ProjFromInPlace(vec)};
            [[maybe_unused]] auto r5{e.ProjFrom(vec)};
            [[maybe_unused]] auto r6{
                Est{v, cov}
                .ProjFrom(vec)
            };
        }

        // --- Free function Project ---
        {
            Est e2{MakeEstimate<K, C>(v, cov)};
            // All 4 permutations of lval/rval
            [[maybe_unused]] auto p1{Project(e, e2)};                    // const& × const&
            [[maybe_unused]] auto p2{Project(e, Est{v, cov})};           // const& × &&
            [[maybe_unused]] auto p3{Project(Est{v, cov}, e2)};          // && × const&
            [[maybe_unused]] auto p4{Project(Est{v, cov}, Est{v, cov})}; // && × &&

            // Cross-type free function
            if constexpr (C == CovarianceOption::Full) {
                using EstD = Estimate<K, CovarianceOption::Diagonal>;
                auto covD{MakeTestCov<K, CovarianceOption::Diagonal>(dim)};
                EstD eCross{v, covD};
                [[maybe_unused]] auto pc1{Project(e, eCross)};
                [[maybe_unused]] auto pc2{Project(Est{v, cov}, EstD{v, covD})};
            } else {
                using EstF = Estimate<K, CovarianceOption::Full>;
                auto covF{MakeTestCov<K, CovarianceOption::Full>(dim)};
                EstF eCross{v, covF};
                [[maybe_unused]] auto pc1{Project(e, eCross)};
                [[maybe_unused]] auto pc2{Project(Est{v, cov}, EstF{v, covF})};
            }
        }
    }
}};

// =========================================================================
// Section 36a: ProjTo value correctness
// =========================================================================

constexpr auto sec36aProjToValue{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{MakeEstimate<K, C>(v, cov)};

        auto v2{MakeTestValue<K, C>(dim)};
        for (auto i{0}; i < dim; ++i) {
            const_cast<double&>(v2(i)) = dim - i;
        }
        const auto cov2{MakeTestCov<K, C>(dim)};
        Est e2{MakeEstimate<K, C>(v2, cov2)};

        const auto c{v.dot(v2) / v2.squaredNorm()};
        Eigen::VectorXd expectedValue{dim};
        expectedValue = c * v2;

        // 36a-a: ProjTo value (copy)
        {
            const auto result{e.ProjTo(e2)};
            for (auto i{0}; i < dim; ++i) {
                CheckClose(result.Value(i), expectedValue(i), "36a-a: ProjTo value");
            }
        }

        // 36a-b: ProjTo plain vector value
        {
            const auto result{e.ProjTo(v2)};
            for (auto i{0}; i < dim; ++i) {
                CheckClose(result.Value(i), expectedValue(i), "36a-b: ProjTo plain vector value");
            }
        }
    }
}};

// =========================================================================
// Section 36b: ProjFrom value correctness
// =========================================================================

constexpr auto sec36bProjFromValue{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{MakeEstimate<K, C>(v, cov)};

        auto v2{MakeTestValue<K, C>(dim)};
        for (auto i{0}; i < dim; ++i) {
            const_cast<double&>(v2(i)) = dim - i;
        }
        const auto cov2{MakeTestCov<K, C>(dim)};
        Est e2{MakeEstimate<K, C>(v2, cov2)};

        const auto c{v2.dot(v) / v.squaredNorm()};
        Eigen::VectorXd expectedValue{dim};
        expectedValue = c * v;

        // 36b-a: ProjFrom value (copy)
        {
            const auto result{e.ProjFrom(e2)};
            for (auto i{0}; i < dim; ++i) {
                CheckClose(result.Value(i), expectedValue(i), "36b-a: ProjFrom value");
            }
        }

        // 36b-b: ProjFrom plain vector value
        {
            const auto result{e.ProjFrom(v2)};
            for (auto i{0}; i < dim; ++i) {
                CheckClose(result.Value(i), expectedValue(i), "36b-b: ProjFrom plain vector value");
            }
        }
    }
}};

// =========================================================================
// Section 36c: ProjTo covariance verification
// =========================================================================

constexpr auto sec36cProjToCov{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{MakeEstimate<K, C>(v, cov)};

        auto v2{MakeTestValue<K, C>(dim)};
        for (auto i{0}; i < dim; ++i) {
            const_cast<double&>(v2(i)) = dim - i;
        }
        const auto cov2{MakeTestCov<K, C>(dim)};
        Est e2{MakeEstimate<K, C>(v2, cov2)};

        const auto expectedCovMat{manualProjToCov(v, v2, toFullCov(e, dim), toFullCov(e2, dim))};

        // 36c-a: ProjTo covariance
        {
            const auto result{e.ProjTo(e2)};
            for (auto i{0}; i < dim; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    if constexpr (C == CovarianceOption::Diagonal) {
                        if (i != j) {
                            continue;
                        }
                    }
                    CheckClose(result.Covariance(i, j), expectedCovMat(i, j),
                               fmt::format("36c-a: ProjTo cov({},{})", i, j));
                }
            }
        }

        // 36c-b: ProjTo plain vector covariance (simplified formula)
        {
            const auto result{e.ProjTo(v2)};
            const auto yNormSq{v2.squaredNorm()};
            const double beta{(v2.transpose() * toFullCov(e, dim) * v2)(0, 0) / (yNormSq * yNormSq)};
            Eigen::MatrixXd expectedCov{dim, dim};
            expectedCov = beta * v2 * v2.transpose();
            for (auto i{0}; i < dim; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    if constexpr (C == CovarianceOption::Diagonal) {
                        if (i != j) {
                            continue;
                        }
                    }
                    CheckClose(result.Covariance(i, j), expectedCov(i, j),
                               fmt::format("36c-b: ProjTo plain cov({},{})", i, j));
                }
            }
        }
    }
}};

// =========================================================================
// Section 36d: ProjFrom covariance verification
// =========================================================================

constexpr auto sec36dProjFromCov{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{MakeEstimate<K, C>(v, cov)};

        auto v2{MakeTestValue<K, C>(dim)};
        for (auto i{0}; i < dim; ++i) {
            const_cast<double&>(v2(i)) = dim - i;
        }
        const auto cov2{MakeTestCov<K, C>(dim)};
        Est e2{MakeEstimate<K, C>(v2, cov2)};

        // ProjFrom = swap x↔y in manualProjToCov
        const auto expectedCovMat{manualProjToCov(v2, v, toFullCov(e2, dim), toFullCov(e, dim))};

        // 36d-a: ProjFrom covariance
        {
            const auto result{e.ProjFrom(e2)};
            for (auto i{0}; i < dim; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    if constexpr (C == CovarianceOption::Diagonal) {
                        if (i != j) {
                            continue;
                        }
                    }
                    CheckClose(result.Covariance(i, j), expectedCovMat(i, j),
                               fmt::format("36d-a: ProjFrom cov({},{})", i, j));
                }
            }
        }

        // 36d-b: ProjFrom plain vector covariance (simplified formula)
        {
            const auto result{e.ProjFrom(v2)};
            const auto xNormSq{v.squaredNorm()};
            const auto c{v2.dot(v) / xNormSq};
            const auto invXSq{1. / xNormSq};
            Eigen::VectorXd vVec{(v2 - 2. * c * v) * invXSq};
            auto covXFull{toFullCov(e, dim)};
            Eigen::VectorXd w{covXFull * vVec};
            const double beta{vVec.dot(w)};
            Eigen::MatrixXd expectedCov{c * c * covXFull};
            expectedCov.noalias() += beta * v * v.transpose();
            expectedCov.noalias() += c * (v * w.transpose() + w * v.transpose());
            for (auto i{0}; i < dim; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    if constexpr (C == CovarianceOption::Diagonal) {
                        if (i != j) {
                            continue;
                        }
                    }
                    CheckClose(result.Covariance(i, j), expectedCov(i, j),
                               fmt::format("36d-b: ProjFrom plain cov({},{})", i, j));
                }
            }
        }
    }
}};

// =========================================================================
// Section 36e: Free function Project
// =========================================================================

constexpr auto sec36eProject{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{MakeEstimate<K, C>(v, cov)};

        auto v2{MakeTestValue<K, C>(dim)};
        for (auto i{0}; i < dim; ++i) {
            const_cast<double&>(v2(i)) = dim - i;
        }
        const auto cov2{MakeTestCov<K, C>(dim)};
        Est e2{MakeEstimate<K, C>(v2, cov2)};

        const auto expectedCovMat{manualProjToCov(v, v2, toFullCov(e, dim), toFullCov(e2, dim))};

        // 36e-a: Project(e, e2) == e.ProjTo(e2)
        {
            const auto p{Project(e, e2)};
            const auto t{e.ProjTo(e2)};
            for (auto i{0}; i < dim; ++i) {
                CheckClose(p.Value(i), t.Value(i), "36e-a: Project value == ProjTo");
            }
            for (auto i{0}; i < dim; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    if constexpr (C == CovarianceOption::Diagonal) {
                        if (i != j) {
                            continue;
                        }
                    }
                    CheckClose(p.Covariance(i, j), t.Covariance(i, j),
                               fmt::format("36e-a: Project cov({},{}) == ProjTo", i, j));
                }
            }
        }

        // 36e-b: Project(rval, lval) also matches
        {
            const auto p{Project(Est{v, cov}, e2)};
            const auto t{Project(e, e2)};
            for (auto i{0}; i < dim; ++i) {
                CheckClose(p.Value(i), t.Value(i), "36e-b: Project(rval,lval) value");
            }
        }

        // 36e-c: Project(lval, rval) also matches
        {
            const auto p{Project(e, Est{v2, cov2})};
            const auto t{Project(e, e2)};
            for (auto i{0}; i < dim; ++i) {
                CheckClose(p.Value(i), t.Value(i), "36e-c: Project(lval,rval) value");
            }
        }

        // 36e-d: Project(rval, rval) also matches
        {
            const auto p{Project(Est{v, cov}, Est{v2, cov2})};
            const auto t{Project(e, e2)};
            for (auto i{0}; i < dim; ++i) {
                CheckClose(p.Value(i), t.Value(i), "36e-d: Project(rval,rval) value");
            }
        }
    }
}};

// =========================================================================
// Section 36f: Zero norm guard
// =========================================================================

constexpr auto sec36fZeroGuard{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{MakeEstimate<K, C>(v, cov)};

        auto vZero{MakeTestValue<K, C>(dim)};
        for (auto i{0}; i < dim; ++i) {
            const_cast<double&>(vZero(i)) = 0.;
        }

        // 36f-a: ProjTo onto zero vector
        {
            const auto result{e.ProjTo(Est{vZero, cov})};
            for (auto i{0}; i < dim; ++i) {
                CheckClose(result.Value(i), 0., "36f-a: ProjTo onto zero value");
            }
            for (auto i{0}; i < dim; ++i) {
                CheckClose(result.Variance(i), 0., "36f-a: ProjTo onto zero variance");
            }
        }

        // 36f-b: ProjFrom from zero self
        {
            Est eZero{MakeEstimate<K, C>(vZero, cov)};
            const auto result{eZero.ProjFrom(e)};
            for (auto i{0}; i < dim; ++i) {
                CheckClose(result.Value(i), 0., "36f-b: ProjFrom zero self value");
            }
            for (auto i{0}; i < dim; ++i) {
                CheckClose(result.Variance(i), 0., "36f-b: ProjFrom zero self variance");
            }
        }

        // 36f-c: ProjTo plain vector onto zero
        {
            const auto result{e.ProjTo(vZero)};
            for (auto i{0}; i < dim; ++i) {
                CheckClose(result.Value(i), 0., "36f-c: ProjTo plain zero value");
            }
            for (auto i{0}; i < dim; ++i) {
                CheckClose(result.Variance(i), 0., "36f-c: ProjTo plain zero variance");
            }
        }
    }
}};

// =========================================================================
// Section 36g: Dynamic dimension
// =========================================================================

auto sec36gDynamic() -> void {
    constexpr int dim{3};
    using Est = Estimate<Eigen::Dynamic, CovarianceOption::Full>;

    const auto v{MakeTestValue<Eigen::Dynamic, CovarianceOption::Full>(dim)};
    const auto cov{MakeTestCov<Eigen::Dynamic, CovarianceOption::Full>(dim)};
    Est e{MakeEstimate<Eigen::Dynamic, CovarianceOption::Full>(v, cov)};

    auto v2{MakeTestValue<Eigen::Dynamic, CovarianceOption::Full>(dim)};
    for (auto i{0}; i < dim; ++i) {
        const_cast<double&>(v2(i)) = dim - i;
    }
    const auto cov2{MakeTestCov<Eigen::Dynamic, CovarianceOption::Full>(dim)};
    Est e2{MakeEstimate<Eigen::Dynamic, CovarianceOption::Full>(v2, cov2)};

    // Value check
    const auto c{v.dot(v2) / v2.squaredNorm()};
    {
        const auto result{e.ProjTo(e2)};
        for (auto i{0}; i < dim; ++i) {
            CheckClose(result.Value(i), c * v2(i), "36g: dynamic ProjTo value");
        }
    }

    // Covariance check
    {
        const auto expectedCovMat{manualProjToCov(v, v2, toFullCov(e, dim), toFullCov(e2, dim))};
        const auto result{e.ProjTo(e2)};
        for (auto i{0}; i < dim; ++i) {
            for (auto j{0}; j < dim; ++j) {
                CheckClose(result.Covariance(i, j), expectedCovMat(i, j),
                           fmt::format("36g: dynamic ProjTo cov({},{})", i, j));
            }
        }
    }

    // Free function
    {
        const auto p{Project(e, e2)};
        const auto t{e.ProjTo(e2)};
        for (auto i{0}; i < dim; ++i) {
            CheckClose(p.Value(i), t.Value(i), "36g: dynamic Project value");
        }
    }
}

// =========================================================================
// Section 36h: InPlace identity (ProjToInPlace equals ProjTo copy)
// =========================================================================

constexpr auto sec36hInPlaceIdentity{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;

        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{MakeEstimate<K, C>(v, cov)};

        auto v2{MakeTestValue<K, C>(dim)};
        for (auto i{0}; i < dim; ++i) {
            const_cast<double&>(v2(i)) = dim - i;
        }
        const auto cov2{MakeTestCov<K, C>(dim)};
        Est e2{MakeEstimate<K, C>(v2, cov2)};

        // ProjTo: InPlace copy == copy
        {
            auto copied{e};
            copied.ProjToInPlace(e2);
            const auto result{e.ProjTo(e2)};
            for (auto i{0}; i < dim; ++i) {
                CheckClose(copied.Value(i), result.Value(i),
                           fmt::format("36h-a: ProjToInPlace vs ProjTo value {}", i));
            }
            for (auto i{0}; i < dim; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    if constexpr (C == CovarianceOption::Diagonal) {
                        if (i != j) {
                            continue;
                        }
                    }
                    CheckClose(copied.Covariance(i, j), result.Covariance(i, j),
                               fmt::format("36h-a: ProjToInPlace vs ProjTo cov({},{})", i, j));
                }
            }
        }

        // ProjFrom: InPlace copy == copy
        {
            auto copied{e};
            copied.ProjFromInPlace(e2);
            const auto result{e.ProjFrom(e2)};
            for (auto i{0}; i < dim; ++i) {
                CheckClose(copied.Value(i), result.Value(i),
                           fmt::format("36h-b: ProjFromInPlace vs ProjFrom value {}", i));
            }
            for (auto i{0}; i < dim; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    if constexpr (C == CovarianceOption::Diagonal) {
                        if (i != j) {
                            continue;
                        }
                    }
                    CheckClose(copied.Covariance(i, j), result.Covariance(i, j),
                               fmt::format("36h-b: ProjFromInPlace vs ProjFrom cov({},{})", i, j));
                }
            }
        }
    }
}};

// =========================================================================
// Section 36i: Cross-type covariance
// =========================================================================

auto sec36iCrossType() -> void {
    constexpr int dim{3};
    using EstFF = Estimate<dim, CovarianceOption::Full>;
    using EstFD = Estimate<dim, CovarianceOption::Diagonal>;
    using EstDF = Estimate<dim, CovarianceOption::Diagonal>;

    const auto v{MakeTestValue<dim, CovarianceOption::Full>(dim)};
    auto v2{MakeTestValue<dim, CovarianceOption::Full>(dim)};
    for (auto i{0}; i < dim; ++i) {
        const_cast<double&>(v2(i)) = dim - i;
    }
    const auto covFull{MakeTestCov<dim, CovarianceOption::Full>(dim)};
    const auto covDiag{MakeTestCov<dim, CovarianceOption::Diagonal>(dim)};

    // 36i-a: ProjTo Full × Diagonal
    {
        EstFF eF{v, covFull};
        EstFD eD{v2, covDiag};
        const auto result{eF.ProjTo(eD)};
        const auto expectedCovMat{manualProjToCov(v, v2, toFullCov(eF, dim), toFullCov(eD, dim))};
        for (auto i{0}; i < dim; ++i) {
            for (auto j{0}; j < dim; ++j) {
                CheckClose(result.Covariance(i, j), expectedCovMat(i, j),
                           fmt::format("36i-a: ProjTo(Full,Diag) cov({},{})", i, j));
            }
        }
    }

    // 36i-b: ProjTo Diagonal × Full
    {
        EstDF eD{v, covDiag};
        EstFF eF{v2, covFull};
        const auto result{eD.ProjTo(eF)};
        const auto expectedCovMat{manualProjToCov(v, v2, toFullCov(eD, dim), toFullCov(eF, dim))};
        for (auto i{0}; i < dim; ++i) {
            for (auto j{0}; j < dim; ++j) {
                if (i != j) {
                    continue;
                }
                CheckClose(result.Covariance(i, j), expectedCovMat(i, j),
                           fmt::format("36i-b: ProjTo(Diag,Full) cov({},{})", i, j));
            }
        }
    }

    // 36i-c: ProjTo Diagonal × Diagonal
    {
        EstDF eD1{v, covDiag};
        EstDF eD2{v2, covDiag};
        const auto result{eD1.ProjTo(eD2)};
        const auto expectedCovMat{manualProjToCov(v, v2, toFullCov(eD1, dim), toFullCov(eD2, dim))};
        for (auto i{0}; i < dim; ++i) {
            for (auto j{0}; j < dim; ++j) {
                if (i != j) {
                    continue;
                }
                CheckClose(result.Covariance(i, j), expectedCovMat(i, j),
                           fmt::format("36i-c: ProjTo(Diag,Diag) cov({},{})", i, j));
            }
        }
    }

    // 36i-d: ProjFrom Full × Diagonal
    {
        EstFF eF{v, covFull};
        EstFD eD{v2, covDiag};
        const auto result{eF.ProjFrom(eD)};
        const auto expectedCovMat{manualProjToCov(v2, v, toFullCov(eD, dim), toFullCov(eF, dim))};
        for (auto i{0}; i < dim; ++i) {
            for (auto j{0}; j < dim; ++j) {
                CheckClose(result.Covariance(i, j), expectedCovMat(i, j),
                           fmt::format("36i-d: ProjFrom(Full,Diag) cov({},{})", i, j));
            }
        }
    }

    // 36i-e: ProjFrom Diagonal × Full
    {
        EstDF eD{v, covDiag};
        EstFF eF{v2, covFull};
        const auto result{eD.ProjFrom(eF)};
        const auto expectedCovMat{manualProjToCov(v2, v, toFullCov(eF, dim), toFullCov(eD, dim))};
        for (auto i{0}; i < dim; ++i) {
            for (auto j{0}; j < dim; ++j) {
                if (i != j) {
                    continue;
                }
                CheckClose(result.Covariance(i, j), expectedCovMat(i, j),
                           fmt::format("36i-e: ProjFrom(Diag,Full) cov({},{})", i, j));
            }
        }
    }

    // 36i-f: ProjFrom Diagonal × Diagonal
    {
        EstDF eD1{v, covDiag};
        EstDF eD2{v2, covDiag};
        const auto result{eD1.ProjFrom(eD2)};
        const auto expectedCovMat{manualProjToCov(v2, v, toFullCov(eD2, dim), toFullCov(eD1, dim))};
        for (auto i{0}; i < dim; ++i) {
            for (auto j{0}; j < dim; ++j) {
                if (i != j) {
                    continue;
                }
                CheckClose(result.Covariance(i, j), expectedCovMat(i, j),
                           fmt::format("36i-f: ProjFrom(Diag,Diag) cov({},{})", i, j));
            }
        }
    }

    // 36i-g: Free function cross-type
    {
        EstFF eF{v, covFull};
        EstFD eD{v2, covDiag};
        const auto p{Project(eF, eD)};
        const auto t{eF.ProjTo(eD)};
        for (auto i{0}; i < dim; ++i) {
            CheckClose(p.Value(i), t.Value(i), "36i-g: Project cross-type value");
        }
    }
};

} // namespace TestEstimateSection

auto TestEstimate36::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 36: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec36Smoke);
    PrintLn("  36 smoke passed: all call combinations (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 36a: ProjTo Value ---");
    RunOverAllDims<AllStaticDims>(sec36aProjToValue);
    PrintLn("  36a passed: ProjTo value (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 36b: ProjFrom Value ---");
    RunOverAllDims<AllStaticDims>(sec36bProjFromValue);
    PrintLn("  36b passed: ProjFrom value (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 36c: ProjTo Covariance ---");
    RunOverAllDims<AllStaticDims>(sec36cProjToCov);
    PrintLn("  36c passed: ProjTo covariance (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 36d: ProjFrom Covariance ---");
    RunOverAllDims<AllStaticDims>(sec36dProjFromCov);
    PrintLn("  36d passed: ProjFrom covariance (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 36e: Free function Project ---");
    RunOverAllDims<AllStaticDims>(sec36eProject);
    PrintLn("  36e passed: Project free function (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 36f: Zero norm guard ---");
    RunOverAllDims<AllStaticDims>(sec36fZeroGuard);
    PrintLn("  36f passed: Zero norm guard (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 36g: Dynamic dimension ---");
    sec36gDynamic();
    PrintLn("  36g passed: Dynamic dimension");

    PrintLn("--- Section 36h: InPlace identity ---");
    RunOverAllDims<AllStaticDims>(sec36hInPlaceIdentity);
    PrintLn("  36h passed: ProjToInPlace == ProjTo");

    PrintLn("--- Section 36i: Cross-type covariance ---");
    sec36iCrossType();
    PrintLn("  36i passed: Cross-type Full/Diagonal combinations");

    PrintLn("All TestEstimate36 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
