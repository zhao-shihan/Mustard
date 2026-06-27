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

TestEstimate35::TestEstimate35() :
    Subprogram{"TestEstimate35", "Test Mustard::Math::Estimate (Section 35: ScalarProjTo, ScalarProjFrom, ScalarProj)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Section 35: Smoke Test — all call combinations (compile-only)
// =========================================================================

constexpr auto sec35Smoke{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;
        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{MakeEstimate<K, C>(v, cov)};

        // --- ScalarProjTo member ---
        // Same-type Estimate
        {
            Est e2{MakeEstimate<K, C>(v, cov)};
            [[maybe_unused]] auto r1{e.ScalarProjTo(e2)};
        }
        // Cross-type Estimate
        if constexpr (C == CovarianceOption::Full) {
            using EstD = Estimate<K, CovarianceOption::Diagonal>;
            auto covD{MakeTestCov<K, CovarianceOption::Diagonal>(dim)};
            EstD eCross{v, covD};
            [[maybe_unused]] auto r2{e.ScalarProjTo(eCross)};
        } else {
            using EstF = Estimate<K, CovarianceOption::Full>;
            auto covF{MakeTestCov<K, CovarianceOption::Full>(dim)};
            EstF eCross{v, covF};
            [[maybe_unused]] auto r2{e.ScalarProjTo(eCross)};
        }
        // Plain vector
        {
            [[maybe_unused]] auto r3{e.ScalarProjTo(v)};
        }

        // --- ScalarProjFrom member ---
        // Same-type Estimate
        {
            Est e2{MakeEstimate<K, C>(v, cov)};
            [[maybe_unused]] auto r4{e.ScalarProjFrom(e2)};
        }
        // Cross-type Estimate
        if constexpr (C == CovarianceOption::Full) {
            using EstD = Estimate<K, CovarianceOption::Diagonal>;
            auto covD{MakeTestCov<K, CovarianceOption::Diagonal>(dim)};
            EstD eCross{v, covD};
            [[maybe_unused]] auto r5{e.ScalarProjFrom(eCross)};
        } else {
            using EstF = Estimate<K, CovarianceOption::Full>;
            auto covF{MakeTestCov<K, CovarianceOption::Full>(dim)};
            EstF eCross{v, covF};
            [[maybe_unused]] auto r5{e.ScalarProjFrom(eCross)};
        }
        // Plain vector
        {
            [[maybe_unused]] auto r6{e.ScalarProjFrom(v)};
        }

        // --- ScalarProj free function ---
        Est e2{MakeEstimate<K, C>(v, cov)};
        // Estimate × Estimate
        [[maybe_unused]] auto r7{ScalarProj(e, e2)};
        // Estimate × EigenVec
        [[maybe_unused]] auto r8{ScalarProj(e, v)};
        // EigenVec × Estimate
        [[maybe_unused]] auto r9{ScalarProj(v, e)};

        // Cross-type free function
        if constexpr (C == CovarianceOption::Full) {
            using EstD = Estimate<K, CovarianceOption::Diagonal>;
            auto covD{MakeTestCov<K, CovarianceOption::Diagonal>(dim)};
            EstD eCross{v, covD};
            [[maybe_unused]] auto rc1{ScalarProj(e, eCross)};
            [[maybe_unused]] auto rc2{ScalarProj(eCross, e)};
            [[maybe_unused]] auto rc3{ScalarProj(v, eCross)};
            [[maybe_unused]] auto rc4{ScalarProj(eCross, v)};
        } else {
            using EstF = Estimate<K, CovarianceOption::Full>;
            auto covF{MakeTestCov<K, CovarianceOption::Full>(dim)};
            EstF eCross{v, covF};
            [[maybe_unused]] auto rc1{ScalarProj(e, eCross)};
            [[maybe_unused]] auto rc2{ScalarProj(eCross, e)};
            [[maybe_unused]] auto rc3{ScalarProj(v, eCross)};
            [[maybe_unused]] auto rc4{ScalarProj(eCross, v)};
        }
    }
}};

// =========================================================================
// Helper: compute manual variance for ScalarProjTo given cov matrices
// =========================================================================

auto ManualScalarProjToVar(const Eigen::VectorXd& x, const Eigen::VectorXd& y,
                           const Eigen::MatrixXd& covX, const Eigen::MatrixXd& covY) -> double {
    const auto yNormSq{y.squaredNorm()};
    const auto c{x.dot(y) / yNormSq};
    const auto invYSq{1. / yNormSq};
    const auto residual{x - c * y};
    return invYSq * ((y.transpose() * covX * y).coeff(0, 0) +
                     (residual.transpose() * covY * residual).coeff(0, 0));
}

// Helper: build full cov matrix from Estimate accessors
template<int Dim>
auto ToFullCov(const auto& est) {
    Eigen::Matrix<double, Dim, Dim> m;
    for (auto i{0}; i < Dim; ++i) {
        for (auto j{0}; j < Dim; ++j) {
            m(i, j) = est.Covariance(i, j);
        }
    }
    return m;
}

// =========================================================================
// Section 35a: ScalarProjTo (member) value/variance correctness
// =========================================================================

constexpr auto sec35aScalarProjTo{[]<int K, CovarianceOption C>() {
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

        // Expected value: v·v2 / |v2|
        const double expectedValue{[&] {
            double dot{0.};
            for (auto i{0}; i < dim; ++i) { dot += v(i) * (dim - i); }
            double normSq{0.};
            for (auto i{0}; i < dim; ++i) { normSq += (dim - i) * (dim - i); }
            return dot / std::sqrt(normSq);
        }()};

        // 35a-a. ScalarProjTo value correctness
        {
            const auto result{e.ScalarProjTo(e2)};
            CheckClose(result.Value(), expectedValue, "35a-a: ScalarProjTo value");
        }

        // 35a-b. ScalarProjTo variance verification
        {
            const auto result{e.ScalarProjTo(e2)};
            Eigen::MatrixXd covX(dim, dim), covY(dim, dim);
            for (auto i{0}; i < dim; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    covX(i, j) = e.Covariance(i, j);
                    covY(i, j) = e2.Covariance(i, j);
                }
            }
            const double expectedVar{ManualScalarProjToVar(v, v2, covX, covY)};
            CheckClose(result.Variance(), expectedVar, "35a-b: ScalarProjTo variance");
        }

        // 35a-c. Plain vector overload
        {
            const auto result{e.ScalarProjTo(v2)};
            CheckClose(result.Value(), expectedValue, "35a-c: ScalarProjTo plain vector value");
            Eigen::MatrixXd covX(dim, dim);
            for (auto i{0}; i < dim; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    covX(i, j) = e.Covariance(i, j);
                }
            }
            const auto invYSq{1. / v2.squaredNorm()};
            const double expectedVar{invYSq * (v2.transpose() * covX * v2).coeff(0, 0)};
            CheckClose(result.Variance(), expectedVar, "35a-c: ScalarProjTo plain vector variance");
        }

        // 35a-d. Near-zero |y| guard
        {
            auto vZero{MakeTestValue<K, C>(dim)};
            for (auto i{0}; i < dim; ++i) { const_cast<double&>(vZero(i)) = 0.; }
            Est eZero{MakeEstimate<K, C>(vZero, cov2)};
            const auto result{e.ScalarProjTo(eZero)};
            CheckClose(result.Value(), 0., "35a-d: ScalarProjTo near-zero value");
            CheckClose(result.Variance(), 0., "35a-d: ScalarProjTo near-zero variance");
        }
    }
}};

// =========================================================================
// Section 35b: ScalarProjFrom (member) value/variance correctness
// =========================================================================

constexpr auto sec35bScalarProjFrom{[]<int K, CovarianceOption C>() {
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

        // ScalarProjFrom: project other onto self = (v2·v) / |v|
        const double expectedValue{[&] {
            double dot{0.};
            for (auto i{0}; i < dim; ++i) { dot += (dim - i) * v(i); }
            double normSq{0.};
            for (auto i{0}; i < dim; ++i) { normSq += v(i) * v(i); }
            return dot / std::sqrt(normSq);
        }()};

        // 35b-a. ScalarProjFrom value correctness
        {
            const auto result{e.ScalarProjFrom(e2)};
            CheckClose(result.Value(), expectedValue, "35b-a: ScalarProjFrom value");
        }

        // 35b-b. ScalarProjFrom variance verification
        {
            const auto result{e.ScalarProjFrom(e2)};
            // Var = (1/|x|²) * (residual^T CovX residual + x^T CovY x)
            // where residual = y - (y·x/|x|²)·x
            Eigen::MatrixXd covX(dim, dim), covY(dim, dim);
            for (auto i{0}; i < dim; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    covX(i, j) = e.Covariance(i, j);
                    covY(i, j) = e2.Covariance(i, j);
                }
            }
            const auto xNormSq{v.squaredNorm()};
            const auto yDotX{v2.dot(v)};
            const auto residual{v2 - (yDotX / xNormSq) * v};
            const double expectedVar{(1. / xNormSq) * ((v.transpose() * covY * v).coeff(0, 0) +
                                                       (residual.transpose() * covX * residual).coeff(0, 0))};
            CheckClose(result.Variance(), expectedVar, "35b-b: ScalarProjFrom variance");
        }

        // 35b-c. Plain vector overload (project plain vec onto self)
        {
            const auto result{e.ScalarProjFrom(v2)};
            CheckClose(result.Value(), expectedValue, "35b-c: ScalarProjFrom plain vector value");
            // Var = (1/|x|²) * residual^T CovX residual
            const auto xNormSq{v.squaredNorm()};
            const auto yDotX{v2.dot(v)};
            const auto residual{v2 - (yDotX / xNormSq) * v};
            Eigen::MatrixXd covX(dim, dim);
            for (auto i{0}; i < dim; ++i) {
                for (auto j{0}; j < dim; ++j) {
                    covX(i, j) = e.Covariance(i, j);
                }
            }
            const double expectedVar{(residual.transpose() * covX * residual).coeff(0, 0) / xNormSq};
            CheckClose(result.Variance(), expectedVar, "35b-c: ScalarProjFrom plain vector variance");
        }

        // 35b-d. Near-zero |self| guard
        {
            auto vZero{MakeTestValue<K, C>(dim)};
            for (auto i{0}; i < dim; ++i) { const_cast<double&>(vZero(i)) = 0.; }
            Est eZero{MakeEstimate<K, C>(vZero, cov)};
            const auto result{eZero.ScalarProjFrom(e2)};
            CheckClose(result.Value(), 0., "35b-d: ScalarProjFrom near-zero self value");
            CheckClose(result.Variance(), 0., "35b-d: ScalarProjFrom near-zero self variance");
        }
    }
}};

// =========================================================================
// Section 35c: Free function ScalarProj correctness
// =========================================================================

constexpr auto sec35cFreeScalarProj{[]<int K, CovarianceOption C>() {
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

        // 35c-a: ScalarProj(e1, e2) == e1.ScalarProjTo(e2)
        {
            const auto freeResult{ScalarProj(e, e2)};
            const auto memberResult{e.ScalarProjTo(e2)};
            CheckClose(freeResult.Value(), memberResult.Value(), "35c-a: Free ScalarProj(e1,e2) value");
            CheckClose(freeResult.Variance(), memberResult.Variance(), "35c-a: Free ScalarProj(e1,e2) variance");
        }

        // 35c-b: ScalarProj(e, v2) == e.ScalarProjTo(v2)
        {
            const auto freeResult{ScalarProj(e, v2)};
            const auto memberResult{e.ScalarProjTo(v2)};
            CheckClose(freeResult.Value(), memberResult.Value(), "35c-b: Free ScalarProj(e,v) value");
            CheckClose(freeResult.Variance(), memberResult.Variance(), "35c-b: Free ScalarProj(e,v) variance");
        }

        // 35c-c: ScalarProj(v, e2) == e2.ScalarProjFrom(v)  (the fixed overload)
        {
            const auto freeResult{ScalarProj(v, e2)};
            const auto memberResult{e2.ScalarProjFrom(v)};
            CheckClose(freeResult.Value(), memberResult.Value(), "35c-c: Free ScalarProj(v,e) value");
            CheckClose(freeResult.Variance(), memberResult.Variance(), "35c-c: Free ScalarProj(v,e) variance");
        }

        // 35c-d: Verify asymmetry — ScalarProj(e, e2) != ScalarProj(e2, e) for vectors of different length
        {
            // Use vectors of clearly different lengths: v3 = [1,0,...,0], v3x3 = [3,0,...,0]
            auto vUnit{MakeTestValue<K, C>(dim)};
            auto vScaled{MakeTestValue<K, C>(dim)};
            for (auto i{0}; i < dim; ++i) {
                const_cast<double&>(vScaled(i)) = 3. * (i == 0 ? 1. : 0.);
                const_cast<double&>(vUnit(i)) = (i == 0 ? 1. : 0.);
            }
            Est eUnit{MakeEstimate<K, C>(vUnit, cov)};
            Est eScaled{MakeEstimate<K, C>(vScaled, cov2)};

            const auto forward{ScalarProj(eUnit, eScaled)};
            const auto backward{ScalarProj(eScaled, eUnit)};
            // forward = 1*3/3 = 1, backward = 3*1/1 = 3
            CheckClose(forward.Value(), 1., "35c-d: ScalarProj asymmetric forward");
            CheckClose(backward.Value(), 3., "35c-d: ScalarProj asymmetric backward");
            const bool valuesDiffer{not muc::isclose(forward.Value(), backward.Value())};
            if (not valuesDiffer) {
                Throw<std::runtime_error>("35c-d: ScalarProj should be asymmetric for non-parallel vectors of different lengths");
            }
        }
    }
}};

// =========================================================================
// Section 35d: Cross-type covariance
// =========================================================================

auto sec35dCrossType() -> void {
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

    // 35d-a. ScalarProjTo: Full × Diagonal
    {
        EstFF eF{v, covFull};
        EstFD eD{v2, covDiag};
        const auto result{eF.ScalarProjTo(eD)};
        const double expectedValue{v.dot(v2) / v2.norm()};
        CheckClose(result.Value(), expectedValue, "35d-a: ScalarProjTo(Full,Diag) value");
        const double expectedVar{ManualScalarProjToVar(v, v2, ToFullCov<dim>(eF), ToFullCov<dim>(eD))};
        CheckClose(result.Variance(), expectedVar, "35d-a: ScalarProjTo(Full,Diag) variance");
    }

    // 35d-b. ScalarProjTo: Diagonal × Full
    {
        EstDF eD{v, covDiag};
        EstFF eF{v2, covFull};
        const auto result{eD.ScalarProjTo(eF)};
        const double expectedValue{v.dot(v2) / v2.norm()};
        CheckClose(result.Value(), expectedValue, "35d-b: ScalarProjTo(Diag,Full) value");
        const double expectedVar{ManualScalarProjToVar(v, v2, ToFullCov<dim>(eD), ToFullCov<dim>(eF))};
        CheckClose(result.Variance(), expectedVar, "35d-b: ScalarProjTo(Diag,Full) variance");
    }

    // 35d-c. ScalarProjTo: Diagonal × Diagonal
    {
        EstDD eD1{v, covDiag};
        EstDD eD2{v2, covDiag};
        const auto result{eD1.ScalarProjTo(eD2)};
        const double expectedValue{v.dot(v2) / v2.norm()};
        CheckClose(result.Value(), expectedValue, "35d-c: ScalarProjTo(Diag,Diag) value");
        const double expectedVar{ManualScalarProjToVar(v, v2, ToFullCov<dim>(eD1), ToFullCov<dim>(eD2))};
        CheckClose(result.Variance(), expectedVar, "35d-c: ScalarProjTo(Diag,Diag) variance");
    }

    // 35d-d. ScalarProjFrom: Full × Diagonal
    {
        EstFF eF{v, covFull};
        EstFD eD{v2, covDiag};
        const auto result{eF.ScalarProjFrom(eD)};
        const double expectedValue{v2.dot(v) / v.norm()};
        CheckClose(result.Value(), expectedValue, "35d-d: ScalarProjFrom(Full,Diag) value");
        const double expectedVar{ManualScalarProjToVar(v2, v, ToFullCov<dim>(eD), ToFullCov<dim>(eF))};
        CheckClose(result.Variance(), expectedVar, "35d-d: ScalarProjFrom(Full,Diag) variance");
    }

    // 35d-e. ScalarProjFrom: Diagonal × Full
    {
        EstDF eD{v, covDiag};
        EstFF eF{v2, covFull};
        const auto result{eD.ScalarProjFrom(eF)};
        const double expectedValue{v2.dot(v) / v.norm()};
        CheckClose(result.Value(), expectedValue, "35d-e: ScalarProjFrom(Diag,Full) value");
        const double expectedVar{ManualScalarProjToVar(v2, v, ToFullCov<dim>(eF), ToFullCov<dim>(eD))};
        CheckClose(result.Variance(), expectedVar, "35d-e: ScalarProjFrom(Diag,Full) variance");
    }

    // 35d-f. ScalarProjFrom: Diagonal × Diagonal
    {
        EstDD eD1{v, covDiag};
        EstDD eD2{v2, covDiag};
        const auto result{eD1.ScalarProjFrom(eD2)};
        const double expectedValue{v2.dot(v) / v.norm()};
        CheckClose(result.Value(), expectedValue, "35d-f: ScalarProjFrom(Diag,Diag) value");
        const double expectedVar{ManualScalarProjToVar(v2, v, ToFullCov<dim>(eD2), ToFullCov<dim>(eD1))};
        CheckClose(result.Variance(), expectedVar, "35d-f: ScalarProjFrom(Diag,Diag) variance");
    }
};

// =========================================================================
// Section 35e: Free function cross-type smoke (compile-only)
// =========================================================================

constexpr auto sec35eFreeCrossType{[] {
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
        scalar = ScalarProj(ef3, ed3).Value();
        scalar = ScalarProj(ed3, ef3).Value();
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
        scalar = ScalarProj(e3f, edf3).Value();
        Estimate<3, diag> e3d{MakeTestValue<3, diag>(3), cov3d};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};
        scalar = ScalarProj(e3d, edd3).Value();
    }

    // C: Dynamic ↔ Dynamic
    {
        const auto xDyn3{MakeTestValue<dyn, full>(3)};
        const auto covDynF3{MakeTestCov<dyn, full>(3)};
        const auto covDynD3{MakeTestCov<dyn, diag>(3)};
        Estimate<dyn, full> edf3{xDyn3, covDynF3};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};
        scalar = ScalarProj(edf3, edd3).Value();
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
        scalar = ScalarProj(e3f, edd3).Value();
        Estimate<3, diag> e3d{MakeTestValue<3, diag>(3), cov3d};
        Estimate<dyn, full> edf3{xDyn3, covDynF3};
        scalar = ScalarProj(e3d, edf3).Value();
    }

    // E: EigenVec × Estimate (the fixed overload)
    {
        const auto x3{MakeTestValue<3, full>(3)};
        const auto cov3f{MakeTestCov<3, full>(3)};
        Estimate<3, full> e3f{x3, cov3f};
        scalar = ScalarProj(x3, e3f).Value();
    }

    // F: Dynamic EigenVec × Estimate
    {
        const auto xDyn3{MakeTestValue<dyn, full>(3)};
        const auto covDynF3{MakeTestCov<dyn, full>(3)};
        Estimate<dyn, full> edf3{xDyn3, covDynF3};
        scalar = ScalarProj(xDyn3, edf3).Value();
    }
}};

// =========================================================================
// Section 35f: Edge cases (parallel, orthogonal, opposite)
// =========================================================================

auto sec35fEdgeCases() -> void {
    constexpr int dim{3};
    using Est = Estimate<dim, CovarianceOption::Full>;

    const auto cov{MakeTestCov<dim, CovarianceOption::Full>(dim)};

    // 35f-a. Parallel vectors: v2 = 2 * v1
    {
        Eigen::Vector3d v1;
        v1 << 1., 2., 3.;
        Eigen::Vector3d v2{2. * v1};
        Est e1{MakeEstimate<dim, CovarianceOption::Full>(v1, cov)};
        Est e2{MakeEstimate<dim, CovarianceOption::Full>(v2, cov)};

        // ScalarProjTo: project v1 onto v2 = v1·v2 / |v2| = 2|v1|² / (2|v1|) = |v1|
        const auto r1{e1.ScalarProjTo(e2)};
        CheckClose(r1.Value(), v1.norm(), "35f-a: ScalarProjTo parallel value");

        // ScalarProjFrom: project v2 onto v1 = v2·v1 / |v1| = 2|v1|² / |v1| = 2|v1|
        const auto r2{e1.ScalarProjFrom(e2)};
        CheckClose(r2.Value(), 2. * v1.norm(), "35f-a: ScalarProjFrom parallel value");
    }

    // 35f-b. Orthogonal vectors
    {
        Eigen::Vector3d v1;
        v1 << 1., 0., 0.;
        Eigen::Vector3d v2;
        v2 << 0., 2., 0.;
        Est e1{MakeEstimate<dim, CovarianceOption::Full>(v1, cov)};
        Est e2{MakeEstimate<dim, CovarianceOption::Full>(v2, cov)};

        const auto r1{e1.ScalarProjTo(e2)};
        CheckClose(r1.Value(), 0., "35f-b: ScalarProjTo orthogonal value");
        const auto r2{e1.ScalarProjFrom(e2)};
        CheckClose(r2.Value(), 0., "35f-b: ScalarProjFrom orthogonal value");
    }

    // 35f-c. Opposite vectors
    {
        Eigen::Vector3d v1;
        v1 << 1., 2., 3.;
        Eigen::Vector3d v2{-v1};
        Est e1{MakeEstimate<dim, CovarianceOption::Full>(v1, cov)};
        Est e2{MakeEstimate<dim, CovarianceOption::Full>(v2, cov)};

        // ScalarProjTo: project v1 onto -v1 = v1·(-v1)/|v1| = -|v1|
        const auto r1{e1.ScalarProjTo(e2)};
        CheckClose(r1.Value(), -v1.norm(), "35f-c: ScalarProjTo opposite value");

        // ScalarProjFrom: project -v1 onto v1 = (-v1)·v1/|v1| = -|v1|
        const auto r2{e1.ScalarProjFrom(e2)};
        CheckClose(r2.Value(), -v1.norm(), "35f-c: ScalarProjFrom opposite value");
    }

    // 35f-d. Free function with parallel vectors: ScalarProj(v1, v2) != ScalarProj(v2, v1)
    {
        Eigen::Vector3d v1;
        v1 << 1., 0., 0.;
        Eigen::Vector3d v2{3. * v1};
        Est e1{MakeEstimate<dim, CovarianceOption::Full>(v1, cov)};
        Est e2{MakeEstimate<dim, CovarianceOption::Full>(v2, cov)};

        const auto forward{ScalarProj(e1, e2)};
        const auto backward{ScalarProj(e2, e1)};
        // forward = |v1| = 1, backward = |v2| = 3
        CheckClose(forward.Value(), 1., "35f-d: ScalarProj(e1,e2) parallel");
        CheckClose(backward.Value(), 3., "35f-d: ScalarProj(e2,e1) parallel");
    }

    // 35f-e. Free function with EigenVec+Estimate
    {
        Eigen::Vector3d v1;
        v1 << 2., 0., 0.;
        Eigen::Vector3d v2;
        v2 << 3., 0., 0.;
        Est e2{MakeEstimate<dim, CovarianceOption::Full>(v2, cov)};

        // ScalarProj(v1, e2) = v1·v2/|v2| = 6/3 = 2
        const auto r{ScalarProj(v1, e2)};
        CheckClose(r.Value(), 2., "35f-e: ScalarProj(EigenVec,Estimate) parallel value");
    }
};

} // namespace TestEstimateSection

auto TestEstimate35::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 35: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec35Smoke);
    PrintLn("  35 smoke passed: all call combinations (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 35a: ScalarProjTo ---");
    RunOverAllDims<AllStaticDims>(sec35aScalarProjTo);
    PrintLn("  35a passed: ScalarProjTo (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 35b: ScalarProjFrom ---");
    RunOverAllDims<AllStaticDims>(sec35bScalarProjFrom);
    PrintLn("  35b passed: ScalarProjFrom (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 35c: Free function ScalarProj ---");
    RunOverAllDims<AllStaticDims>(sec35cFreeScalarProj);
    PrintLn("  35c passed: Free ScalarProj matches member");

    PrintLn("--- Section 35d: Cross-type covariance ---");
    sec35dCrossType();
    PrintLn("  35d passed: Cross-type Full/Diagonal combinations");

    PrintLn("--- Section 35e: Free function cross-type smoke ---");
    sec35eFreeCrossType();
    PrintLn("  35e passed: Cross-type free function compile");

    PrintLn("--- Section 35f: Edge cases ---");
    sec35fEdgeCases();
    PrintLn("  35f passed: Parallel, orthogonal, opposite vectors");

    PrintLn("All TestEstimate35 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
