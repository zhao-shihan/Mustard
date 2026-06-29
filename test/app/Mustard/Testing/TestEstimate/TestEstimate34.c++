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

#include <cmath>
#include <cstdlib>

namespace Mustard::Testing {

TestEstimate34::TestEstimate34() :
    Subprogram{"TestEstimate34", "Test Mustard::Math::Estimate (Section 34: Cosine, Angle)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Section 34: Smoke Test — all call combinations
// =========================================================================

constexpr auto sec34Smoke{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;
        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{MakeEstimate<K, C>(v, cov)};

        // Same-type Estimate
        {
            Est e2{MakeEstimate<K, C>(v, cov)};
            [[maybe_unused]] auto r2{e.Cosine(e2)};
            [[maybe_unused]] auto r3{e.Angle(e2)};
        }

        // Cross-type Estimate
        if constexpr (C == CovarianceOption::Full) {
            using EstD = Estimate<K, CovarianceOption::Diagonal>;
            auto covD{MakeTestCov<K, CovarianceOption::Diagonal>(dim)};
            EstD eCross{v, covD};
            [[maybe_unused]] auto r2{e.Cosine(eCross)};
            [[maybe_unused]] auto r3{e.Angle(eCross)};
        } else {
            using EstF = Estimate<K, CovarianceOption::Full>;
            auto covF{MakeTestCov<K, CovarianceOption::Full>(dim)};
            EstF eCross{v, covF};
            [[maybe_unused]] auto r2{e.Cosine(eCross)};
            [[maybe_unused]] auto r3{e.Angle(eCross)};
        }

        // Plain vector
        {
            [[maybe_unused]] auto r2{e.Cosine(v)};
            [[maybe_unused]] auto r3{e.Angle(v)};
        }
    }
}};

// =========================================================================
// Section 34a: Cosine
// =========================================================================

constexpr auto sec34Cosine{[]<int K, CovarianceOption C>() {
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

        // Expected value: v·v2 / (|v|·|v2|)
        const double expectedValue{[&] {
            double dot{0.};
            for (auto i{0}; i < dim; ++i) { dot += v(i) * (dim - i); }
            double normSq1{0.}, normSq2{0.};
            for (auto i{0}; i < dim; ++i) {
                normSq1 += v(i) * v(i);
                normSq2 += (dim - i) * (dim - i);
            }
            return dot / (std::sqrt(normSq1) * std::sqrt(normSq2));
        }()};

        // 34a-a. Cosine value correctness
        {
            const auto result{e.Cosine(e2)};
            CheckClose(result.Value(), expectedValue, "34a-a: Cosine value");
        }

        // 34a-b. Cosine variance verification
        {
            const auto result{e.Cosine(e2)};
            const auto xDotY{v.dot(v2)};
            const auto xNormSq{v.squaredNorm()};
            const auto yNormSq{v2.squaredNorm()};
            const auto invDenom{1. / (xNormSq * yNormSq)};
            const auto jacX{v2 - (xDotY / xNormSq) * v};
            const auto jacY{v - (xDotY / yNormSq) * v2};

            Eigen::MatrixXd covX(dim, dim), covY(dim, dim);
            for (auto i{0}; i < dim; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    covX(i, j) = e.Covariance(i, j);
                    covY(i, j) = e2.Covariance(i, j);
                }
            }
            const double expectedVar{invDenom * ((jacX.transpose() * covX * jacX).coeff(0, 0) +
                                                 (jacY.transpose() * covY * jacY).coeff(0, 0))};
            CheckClose(result.Variance(), expectedVar, "34a-b: Cosine variance");
        }

        // 34a-c. Plain vector overload
        {
            const auto result{e.Cosine(v2)};
            CheckClose(result.Value(), expectedValue, "34a-c: Cosine plain vector value");
            const auto xDotY{v.dot(v2)};
            const auto xNormSq{v.squaredNorm()};
            const auto yNormSq{v2.squaredNorm()};
            const auto invDenom{1. / (xNormSq * yNormSq)};
            const auto jacX{v2 - (xDotY / xNormSq) * v};

            Eigen::MatrixXd covX(dim, dim);
            for (auto i{0}; i < dim; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    covX(i, j) = e.Covariance(i, j);
                }
            }
            const double expectedVar{invDenom * (jacX.transpose() * covX * jacX).coeff(0, 0)};
            CheckClose(result.Variance(), expectedVar, "34a-c: Cosine plain vector variance");
        }

        // 34a-d. Near-zero |y| guard
        {
            auto vZero{MakeTestValue<K, C>(dim)};
            for (auto i{0}; i < dim; ++i) { const_cast<double&>(vZero(i)) = 0.; }
            Est eZero{MakeEstimate<K, C>(vZero, cov2)};
            const auto result{e.Cosine(eZero)};
            CheckClose(result.Value(), 0., "34a-d: Cosine near-zero y value");
            CheckClose(result.Variance(), 0., "34a-d: Cosine near-zero y variance");
        }

        // 34a-e. Near-zero |x| guard
        {
            auto vZero{MakeTestValue<K, C>(dim)};
            for (auto i{0}; i < dim; ++i) { const_cast<double&>(vZero(i)) = 0.; }
            Est eZero{MakeEstimate<K, C>(vZero, cov)};
            const auto result{eZero.Cosine(e2)};
            CheckClose(result.Value(), 0., "34a-e: Cosine near-zero x value");
            CheckClose(result.Variance(), 0., "34a-e: Cosine near-zero x variance");
        }
    }
}};

// =========================================================================
// Section 34b: Angle
// =========================================================================

constexpr auto sec34Angle{[]<int K, CovarianceOption C>() {
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

        // Expected value: acos(v·v2 / (|v|·|v2|))
        const double expectedValue{[&] {
            double dot{0.};
            for (auto i{0}; i < dim; ++i) { dot += v(i) * (dim - i); }
            double normSq1{0.}, normSq2{0.};
            for (auto i{0}; i < dim; ++i) {
                normSq1 += v(i) * v(i);
                normSq2 += (dim - i) * (dim - i);
            }
            return std::acos(dot / (std::sqrt(normSq1) * std::sqrt(normSq2)));
        }()};

        // 34b-a. Angle value correctness
        {
            const auto result{e.Angle(e2)};
            CheckClose(result.Value(), expectedValue, "34b-a: Angle value");
        }

        // 34b-b. Angle variance matches Cosine().Acos()
        {
            const auto angleResult{e.Angle(e2)};
            const auto acosResult{e.Cosine(e2).Acos()};
            CheckClose(angleResult.Value(), acosResult.Value(), "34b-b: Angle value == Cosine().Acos()");
            CheckClose(angleResult.Variance(), acosResult.Variance(), "34b-b: Angle variance == Cosine().Acos()");
        }

        // 34b-c. Angle == free function acos(Cosine)
        {
            const auto angleResult{e.Angle(e2)};
            const auto freeAcosResult{acos(e.Cosine(e2))};
            CheckClose(angleResult.Value(), freeAcosResult.Value(), "34b-c: Angle value == acos(Cosine)");
            CheckClose(angleResult.Variance(), freeAcosResult.Variance(), "34b-c: Angle variance == acos(Cosine)");
        }

        // 34b-d. Plain vector overload
        {
            const auto result{e.Angle(v2)};
            CheckClose(result.Value(), expectedValue, "34b-d: Angle plain vector value");
            const auto acosResult{acos(e.Cosine(v2))};
            CheckClose(result.Variance(), acosResult.Variance(), "34b-d: Angle plain vector variance");
        }
    }
}};

// =========================================================================
// Section 34c: Cross-type tests
// =========================================================================

auto sec34CrossType() -> void {
    constexpr int dim{3};
    using EstFF = Estimate<dim, CovarianceOption::Full>;
    using EstFD = Estimate<dim, CovarianceOption::Diagonal>;
    using EstDF = Estimate<dim, CovarianceOption::Diagonal>;
    using EstDD = Estimate<dim, CovarianceOption::Diagonal>;

    const auto v{MakeTestValue<dim, CovarianceOption::Full>(dim)};
    auto v2{MakeTestValue<dim, CovarianceOption::Full>(dim)};
    for (auto i{0}; i < dim; ++i) { const_cast<double&>(v2(i)) = dim - i; }
    const auto covFull{MakeTestCov<dim, CovarianceOption::Full>(dim)};
    const auto covDiag{MakeTestCov<dim, CovarianceOption::Diagonal>(dim)};

    // Helper: compute manual variance for Cosine given cov matrices
    auto manualCosineVar{[](const Eigen::Vector3d& x, const Eigen::Vector3d& y,
                            const Eigen::Matrix3d& covX, const Eigen::Matrix3d& covY) -> double {
        const auto xDotY{x.dot(y)};
        const auto xNormSq{x.squaredNorm()};
        const auto yNormSq{y.squaredNorm()};
        const auto invDenom{1. / (xNormSq * yNormSq)};
        const auto jacX{y - (xDotY / xNormSq) * x};
        const auto jacY{x - (xDotY / yNormSq) * y};
        return invDenom * ((jacX.transpose() * covX * jacX).coeff(0, 0) +
                           (jacY.transpose() * covY * jacY).coeff(0, 0));
    }};

    // Helper: build full cov matrix from Estimate accessors
    auto toFullCov{[](const auto& est) {
        Eigen::Matrix3d m;
        for (auto i{0}; i < dim; ++i) {
            for (auto j{0}; j < dim; ++j) {
                m(i, j) = est.Covariance(i, j);
            }
        }
        return m;
    }};

    // 34c-a. Cosine: Full × Diagonal
    {
        EstFF eF{v, covFull};
        EstFD eD{v2, covDiag};
        const auto result{eF.Cosine(eD)};
        const double expectedValue{v.dot(v2) / (v.norm() * v2.norm())};
        CheckClose(result.Value(), expectedValue, "34c-a: Cosine(Full,Diag) value");
        const double expectedVar{manualCosineVar(v, v2, toFullCov(eF), toFullCov(eD))};
        CheckClose(result.Variance(), expectedVar, "34c-a: Cosine(Full,Diag) variance");
    }

    // 34c-b. Cosine: Diagonal × Full
    {
        EstDF eD{v, covDiag};
        EstFF eF{v2, covFull};
        const auto result{eD.Cosine(eF)};
        const double expectedValue{v.dot(v2) / (v.norm() * v2.norm())};
        CheckClose(result.Value(), expectedValue, "34c-b: Cosine(Diag,Full) value");
        const double expectedVar{manualCosineVar(v, v2, toFullCov(eD), toFullCov(eF))};
        CheckClose(result.Variance(), expectedVar, "34c-b: Cosine(Diag,Full) variance");
    }

    // 34c-c. Cosine: Diagonal × Diagonal
    {
        EstDD eD1{v, covDiag};
        EstDD eD2{v2, covDiag};
        const auto result{eD1.Cosine(eD2)};
        const double expectedValue{v.dot(v2) / (v.norm() * v2.norm())};
        CheckClose(result.Value(), expectedValue, "34c-c: Cosine(Diag,Diag) value");
        const double expectedVar{manualCosineVar(v, v2, toFullCov(eD1), toFullCov(eD2))};
        CheckClose(result.Variance(), expectedVar, "34c-c: Cosine(Diag,Diag) variance");
    }

    // 34c-d. Angle: Full × Diagonal
    {
        EstFF eF{v, covFull};
        EstFD eD{v2, covDiag};
        const auto result{eF.Angle(eD)};
        const auto expectedAcos{eF.Cosine(eD).Acos()};
        CheckClose(result.Value(), expectedAcos.Value(), "34c-d: Angle(Full,Diag) value");
        CheckClose(result.Variance(), expectedAcos.Variance(), "34c-d: Angle(Full,Diag) variance");
    }

    // 34c-e. Angle: Diagonal × Full
    {
        EstDF eD{v, covDiag};
        EstFF eF{v2, covFull};
        const auto result{eD.Angle(eF)};
        const auto expectedAcos{eD.Cosine(eF).Acos()};
        CheckClose(result.Value(), expectedAcos.Value(), "34c-e: Angle(Diag,Full) value");
        CheckClose(result.Variance(), expectedAcos.Variance(), "34c-e: Angle(Diag,Full) variance");
    }

    // 34c-f. Angle: Diagonal × Diagonal
    {
        EstDD eD1{v, covDiag};
        EstDD eD2{v2, covDiag};
        const auto result{eD1.Angle(eD2)};
        const auto expectedAcos{eD1.Cosine(eD2).Acos()};
        CheckClose(result.Value(), expectedAcos.Value(), "34c-f: Angle(Diag,Diag) value");
        CheckClose(result.Variance(), expectedAcos.Variance(), "34c-f: Angle(Diag,Diag) variance");
    }
};

// =========================================================================
// Section 34: Free Function Smoke — all call combinations
// =========================================================================

constexpr auto sec34SmokeFree{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;
        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{MakeEstimate<K, C>(v, cov)};
        Est e2{MakeEstimate<K, C>(v, cov)};

        // Cosine free functions
        [[maybe_unused]] auto r4{Cosine(e, e2)};
        [[maybe_unused]] auto r5{Cosine(e, v)};
        [[maybe_unused]] auto r6{Cosine(v, e)};

        // Angle free functions
        [[maybe_unused]] auto r7{Angle(e, e2)};
        [[maybe_unused]] auto r8{Angle(e, v)};
        [[maybe_unused]] auto r9{Angle(v, e)};
    }
}};

// =========================================================================
// Section 34: Free Function Smoke CrossType
// =========================================================================

constexpr auto sec34SmokeFreeCrossType{[] {
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
        scalar = Cosine(ef3, ed3).Value();
        scalar = Cosine(ed3, ef3).Value();
        scalar = Angle(ef3, ed3).Value();
        scalar = Angle(ed3, ef3).Value();
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
        scalar = Cosine(e3f, edf3).Value();
        scalar = Angle(e3f, edf3).Value();
        Estimate<3, diag> e3d{MakeTestValue<3, diag>(3), cov3d};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};
        scalar = Cosine(e3d, edd3).Value();
        scalar = Angle(e3d, edd3).Value();
    }

    // C: Dynamic ↔ Dynamic
    {
        const auto xDyn3{MakeTestValue<dyn, full>(3)};
        const auto covDynF3{MakeTestCov<dyn, full>(3)};
        const auto covDynD3{MakeTestCov<dyn, diag>(3)};
        Estimate<dyn, full> edf3{xDyn3, covDynF3};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};
        scalar = Cosine(edf3, edd3).Value();
        scalar = Angle(edf3, edd3).Value();
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
        scalar = Cosine(e3f, edd3).Value();
        scalar = Angle(e3f, edd3).Value();
        Estimate<3, diag> e3d{MakeTestValue<3, diag>(3), cov3d};
        Estimate<dyn, full> edf3{xDyn3, covDynF3};
        scalar = Cosine(e3d, edf3).Value();
        scalar = Angle(e3d, edf3).Value();
    }
}};

// =========================================================================
// Section 34d: Free function Cosine
// =========================================================================

constexpr auto sec34FreeCosine{[]<int K, CovarianceOption C>() {
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

        // 34d-a: Cosine(e1, e2) == e1.Cosine(e2)
        {
            const auto freeResult{Cosine(e, e2)};
            const auto memberResult{e.Cosine(e2)};
            CheckClose(freeResult.Value(), memberResult.Value(), "34d-a: Free Cosine(e1,e2) value");
            CheckClose(freeResult.Variance(), memberResult.Variance(), "34d-a: Free Cosine(e1,e2) variance");
        }

        // 34d-b: Cosine(e, v2) == e.Cosine(v2)
        {
            const auto freeResult{Cosine(e, v2)};
            const auto memberResult{e.Cosine(v2)};
            CheckClose(freeResult.Value(), memberResult.Value(), "34d-b: Free Cosine(e,v) value");
            CheckClose(freeResult.Variance(), memberResult.Variance(), "34d-b: Free Cosine(e,v) variance");
        }

        // 34d-c: Cosine(v, e2) == e2.Cosine(v)
        {
            const auto freeResult{Cosine(v, e2)};
            const auto memberResult{e2.Cosine(v)};
            CheckClose(freeResult.Value(), memberResult.Value(), "34d-c: Free Cosine(v,e) value");
            CheckClose(freeResult.Variance(), memberResult.Variance(), "34d-c: Free Cosine(v,e) variance");
        }
    }
}};

// =========================================================================
// Section 34e: Free function Angle
// =========================================================================

constexpr auto sec34FreeAngle{[]<int K, CovarianceOption C>() {
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

        // 34e-a: Angle(e1, e2) == e1.Angle(e2)
        {
            const auto freeResult{Angle(e, e2)};
            const auto memberResult{e.Angle(e2)};
            CheckClose(freeResult.Value(), memberResult.Value(), "34e-a: Free Angle(e1,e2) value");
            CheckClose(freeResult.Variance(), memberResult.Variance(), "34e-a: Free Angle(e1,e2) variance");
        }

        // 34e-b: Angle(e, v2) == e.Angle(v2)
        {
            const auto freeResult{Angle(e, v2)};
            const auto memberResult{e.Angle(v2)};
            CheckClose(freeResult.Value(), memberResult.Value(), "34e-b: Free Angle(e,v) value");
            CheckClose(freeResult.Variance(), memberResult.Variance(), "34e-b: Free Angle(e,v) variance");
        }

        // 34e-c: Angle(v, e2) == e2.Angle(v)
        {
            const auto freeResult{Angle(v, e2)};
            const auto memberResult{e2.Angle(v)};
            CheckClose(freeResult.Value(), memberResult.Value(), "34e-c: Free Angle(v,e) value");
            CheckClose(freeResult.Variance(), memberResult.Variance(), "34e-c: Free Angle(v,e) variance");
        }
    }
}};

} // namespace TestEstimateSection

auto TestEstimate34::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 34: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec34Smoke);
    PrintLn("  34 smoke passed: all call combinations (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 34a: Cosine ---");
    RunOverAllDims<AllStaticDims>(sec34Cosine);
    PrintLn("  34a passed: Cosine (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 34b: Angle ---");
    RunOverAllDims<AllStaticDims>(sec34Angle);
    PrintLn("  34b passed: Angle (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 34c: Cross-type ---");
    sec34CrossType();
    PrintLn("  34c passed: Cross-type Full/Diagonal combinations");

    PrintLn("--- Section 34: Free function smoke ---");
    RunOverAllDims<AllStaticDims>(sec34SmokeFree);
    PrintLn("  34 smoke free passed: all free function overloads compile");
    sec34SmokeFreeCrossType();
    PrintLn("  34 smoke free cross-type passed: cross-type free function compile");

    PrintLn("--- Section 34d: Free function Cosine ---");
    RunOverAllDims<AllStaticDims>(sec34FreeCosine);
    PrintLn("  34d passed: Free Cosine matches member");

    PrintLn("--- Section 34e: Free function Angle ---");
    RunOverAllDims<AllStaticDims>(sec34FreeAngle);
    PrintLn("  34e passed: Free Angle matches member");

    PrintLn("All TestEstimate34 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
