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

TestEstimate4::TestEstimate4() :
    Subprogram{"TestEstimate4", "Test Mustard::Math::Estimate (Section 4: *= and /=)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Section 4 Smoke: *= and /= (from sec0Smoke)
// =========================================================================

constexpr auto sec4Smoke{[]<int K, CovarianceOption C>() {
    using Est = Estimate<K, C>;
    constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};

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

    // ---- compound assignment: *=, /= ----
    est *= e3;
    if constexpr (K != 1) {
        est *= x;
    }
    est *= 2.0;

    est /= e3;
    if constexpr (K != 1) {
        est /= x;
    }
    est /= 2.0;

    // ---- compound assignment with scalar Estimate: *=, /= ----
    {
        Estimate<1, C> c{3.0, 0.16};
        est *= c;
        est /= c;
    }
    // cross covariance option scalar estimate
    {
        if constexpr (C == CovarianceOption::Full) {
            Estimate<1, CovarianceOption::Diagonal> cD{3.0, 0.16};
            est *= cD;
            est /= cD;
        } else {
            Estimate<1, CovarianceOption::Full> cF{3.0, 0.16};
            est *= cF;
            est /= cF;
        }
    }
}};

// =========================================================================
// Section 4 Smoke CrossType: Cross *= and /= (from sec0Smoke)
// =========================================================================

constexpr auto sec4SmokeCrossType{[] {
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
        auto efTmp{ef3};
        efTmp *= ed3;
        scalar = efTmp.Value(0);
        efTmp /= ed3;
        scalar = efTmp.Value(0);
        auto edTmp{ed3};
        edTmp *= ef3;
        scalar = edTmp.Value(0);
        edTmp /= ef3;
        scalar = edTmp.Value(0);
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
        auto eTmp{e3f};
        eTmp *= edf3;
        scalar = eTmp.Value(0);
        eTmp /= edf3;
        scalar = eTmp.Value(0);
        auto edTmp{edf3};
        edTmp *= e3f;
        scalar = edTmp.Value(0);
        edTmp /= e3f;
        scalar = edTmp.Value(0);
        Estimate<3, diag> e3d{MakeTestValue<3, diag>(3), cov3d};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};
        auto eTmp2{e3d};
        eTmp2 *= edd3;
        scalar = eTmp2.Variance(0);
        eTmp2 /= edd3;
        scalar = eTmp2.Variance(0);
        auto edTmp2{edd3};
        edTmp2 *= e3d;
        scalar = edTmp2.Variance(0);
        edTmp2 /= e3d;
        scalar = edTmp2.Variance(0);
    }

    // C: Dynamic ↔ Dynamic
    {
        const auto xDyn3{MakeTestValue<dyn, full>(3)};
        const auto covDynF3{MakeTestCov<dyn, full>(3)};
        const auto covDynD3{MakeTestCov<dyn, diag>(3)};
        Estimate<dyn, full> edf3{xDyn3, covDynF3};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};
        auto eTmp{edf3};
        eTmp *= edd3;
        scalar = eTmp.Value(0);
        eTmp /= edd3;
        scalar = eTmp.Value(0);
        auto edTmp{edd3};
        edTmp *= edf3;
        scalar = edTmp.Variance(0);
        edTmp /= edf3;
        scalar = edTmp.Variance(0);
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
        auto eTmp{e3f};
        eTmp *= edd3;
        scalar = eTmp.Value(0);
        eTmp /= edd3;
        scalar = eTmp.Value(0);
        auto edTmp{edd3};
        edTmp *= e3f;
        scalar = edTmp.Variance(0);
        edTmp /= e3f;
        scalar = edTmp.Variance(0);
        Estimate<3, diag> e3d{MakeTestValue<3, diag>(3), cov3d};
        Estimate<dyn, full> edf3{xDyn3, covDynF3};
        auto eTmp2{e3d};
        eTmp2 *= edf3;
        scalar = eTmp2.Variance(0);
        eTmp2 /= edf3;
        scalar = eTmp2.Variance(0);
        auto edTmp2{edf3};
        edTmp2 *= e3d;
        scalar = edTmp2.Value(0);
        edTmp2 /= e3d;
        scalar = edTmp2.Value(0);
    }

    // E: Scalar Estimate<1> *= and /= with various K/C combinations
    {
        Estimate<1, full> cFull{3.0, 0.16};
        Estimate<1, diag> cDiag{3.0, 0.16};

        // E1: Static Full receiving Diagonal scalar, and vice versa
        {
            const auto x3{MakeTestValue<3, full>(3)};
            Estimate<3, full> ef3{x3, MakeTestCov<3, full>(3)};
            ef3 *= cDiag;
            scalar = ef3.Value(0);
            ef3 /= cDiag;
            scalar = ef3.Value(0);
            Estimate<3, diag> ed3{x3, MakeTestCov<3, diag>(3)};
            ed3 *= cFull;
            scalar = ed3.Variance(0);
            ed3 /= cFull;
            scalar = ed3.Variance(0);
        }

        // E2: Dynamic full/diag receiving the opposite
        {
            const auto xDyn3{MakeTestValue<dyn, full>(3)};
            Estimate<dyn, full> edf3{xDyn3, MakeTestCov<dyn, full>(3)};
            edf3 *= cDiag;
            scalar = edf3.Value(0);
            edf3 /= cDiag;
            scalar = edf3.Value(0);
            Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), MakeTestCov<dyn, diag>(3)};
            edd3 *= cFull;
            scalar = edd3.Variance(0);
            edd3 /= cFull;
            scalar = edd3.Variance(0);
        }

        // E3: K=1 receiving scalar of opposite covariance option
        {
            Estimate<1, full> e1f{1.0, 0.5};
            e1f *= cDiag;
            scalar = e1f.Value();
            e1f /= cDiag;
            scalar = e1f.Value();
            Estimate<1, diag> e1d{1.0, 0.5};
            e1d *= cFull;
            scalar = e1d.Variance();
            e1d /= cFull;
            scalar = e1d.Variance();
        }

        // E4: rvalue operator/=(Estimate<1,D>&&) smoke
        {
            Estimate<3, full> ef3{MakeTestValue<3, full>(3), MakeTestCov<3, full>(3)};
            Estimate<1, full> cTmp{3.0, 0.16};
            ef3 /= std::move(cTmp);
            scalar = ef3.Value(0);
        }
    }
}};

constexpr auto sec4MulDivInPlace{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    constexpr bool isFull{C == CovarianceOption::Full};

    const auto v{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    // e1 *= e2: J_x = diag(y), J_y = diag(x)
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        auto e2{MakeEstimate<K, C>(v, cov)};
        e1 *= e2;
        if constexpr (K == 1) {
            CheckClose(e1.Value(), 1.0, "4a: Value() *=e2");
            CheckClose(e1.Variance(), 0.8, "4a: Var() *=e2");
        } else {
            CheckClose(e1.Value(0), 1.0, "4a: Value(0) *=e2");
            if constexpr (dim >= 2) {
                CheckClose(e1.Value(1), 4.0, "4a: Value(1) *=e2");
                CheckClose(e1.Covariance(1, 1), 2.4, "4a: Cov(1,1) *=e2");
                if constexpr (isFull) {
                    CheckClose(e1.Covariance(0, 1), 0.4, "4a: Cov(0,1) *=e2");
                }
            }
            CheckClose(e1.Covariance(0, 0), 0.8, "4a: Cov(0,0) *=e2");
        }
    }

    // e1 *= vector: J = diag(v)
    if constexpr (K != 1) {
        auto e1{MakeEstimate<K, C>(v, cov)};
        e1 *= v;
        CheckClose(e1.Value(0), 1.0, "4b: Value(0) *=vec");
        if constexpr (dim >= 2) {
            CheckClose(e1.Value(1), 4.0, "4b: Value(1) *=vec");
            if constexpr (isFull) {
                CheckClose(e1.Covariance(0, 1), 0.2, "4b: Cov(0,1) *=vec");
            }
        }
        CheckClose(e1.Covariance(0, 0), 0.4, "4b: Cov(0,0) *=vec");
    }

    // e1 *= scalar
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        e1 *= 3.0;
        if constexpr (K == 1) {
            CheckClose(e1.Value(), 3.0, "4c: Value() *=3");
            CheckClose(e1.Variance(), 3.6, "4c: Var() *=3");
        } else {
            CheckClose(e1.Value(0), 3.0, "4c: Value(0) *=3");
            if constexpr (dim >= 2) {
                CheckClose(e1.Value(1), 6.0, "4c: Value(1) *=3");
            }
            CheckClose(e1.Covariance(0, 0), 3.6, "4c: Cov(0,0) *=3");
        }
    }

    // e1 /= e2
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        auto e2{MakeEstimate<K, C>(v, cov)};
        e1 /= e2;
        if constexpr (K == 1) {
            CheckClose(e1.Value(), 1.0, "4d: Value() /=e2");
            CheckClose(e1.Variance(), 0.8, "4d: Var() /=e2");
        } else {
            CheckClose(e1.Value(0), 1.0, "4d: Value(0) /=e2");
            if constexpr (dim >= 2) {
                CheckClose(e1.Value(1), 1.0, "4d: Value(1) /=e2");
                CheckClose(e1.Covariance(1, 1), 0.15, "4d: Cov(1,1) /=e2");
            }
            CheckClose(e1.Covariance(0, 0), 0.8, "4d: Cov(0,0) /=e2");
        }
    }

    // e1 /= vector: J = diag(1/y)
    if constexpr (K != 1) {
        auto e1{MakeEstimate<K, C>(v, cov)};
        e1 /= v;
        CheckClose(e1.Value(0), 1.0, "4e: Value(0) /=vec");
        if constexpr (dim >= 2) {
            CheckClose(e1.Value(1), 1.0, "4e: Value(1) /=vec");
        }
        // Cov = diag(1/y) * Cov * diag(1/y)
        for (auto i{0}; i < dim; ++i) {
            for (auto j{0}; j < dim; ++j) {
                const auto expectedCov{cov.coeff(i, j) / (v(i) * v(j))};
                CheckClose(e1.Covariance(i, j), expectedCov,
                           fmt::format("4e: Cov({},{}) /=vec", i, j));
            }
        }
    }

    // e1 /= scalar
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        e1 /= 2.0;
        if constexpr (K == 1) {
            CheckClose(e1.Value(), 0.5, "4f: Value() /=2");
            CheckClose(e1.Variance(), 0.1, "4f: Var() /=2");
        } else {
            CheckClose(e1.Value(0), 0.5, "4f: Value(0) /=2");
            if constexpr (dim >= 2) {
                CheckClose(e1.Value(1), 1.0, "4f: Value(1) /=2");
            }
            CheckClose(e1.Covariance(0, 0), 0.1, "4f: Cov(0,0) /=2");
        }
    }

    // e1 *= 0
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        e1 *= 0.0;
        if constexpr (K == 1) {
            CheckClose(e1.Value(), 0.0, "4j: Value() *=0");
            CheckClose(e1.Variance(), 0.0, "4j: Var() *=0");
        } else {
            CheckClose(e1.Value(0), 0.0, "4j: Value(0) *=0");
            if constexpr (dim >= 2) {
                CheckClose(e1.Value(1), 0.0, "4j: Value(1) *=0");
                CheckClose(e1.Covariance(1, 1), 0.0, "4j: Cov(1,1) *=0");
            }
            CheckClose(e1.Covariance(0, 0), 0.0, "4j: Cov(0,0) *=0");
        }
    }
}};

// =========================================================================
// Section 4 Functional: *= and /= with scalar Estimate<1,D>
// =========================================================================

constexpr auto sec4MulDivScalarEst{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    constexpr bool isFull{C == CovarianceOption::Full};

    const auto v{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    Estimate<1, C> c{3.0, 0.16};

    // *= with scalar estimate (same covariance option)
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        e1 *= c;
        if constexpr (K == 1) {
            CheckClose(e1.Value(), 3.0, "4sa: Value() *= scalarEst");
            // Var = 0.4*9 + 0.16*1 = 3.76
            CheckClose(e1.Variance(), 3.76, "4sa: Var() *= scalarEst");
        } else {
            CheckClose(e1.Value(0), 3.0, "4sa: Value(0) *= scalarEst");
            if constexpr (dim >= 2) {
                CheckClose(e1.Value(1), 6.0, "4sa: Value(1) *= scalarEst");
            }
            // Cov: step1 *=9, step2 rank1 update Var(c)*x*x^T
            // Full: [[0.4*9, 0.1*9], [0.1*9, 0.3*9]] = [[3.6, 0.9], [0.9, 2.7]]
            //       + 0.16*[[1,2],[2,4]] = [[0.16,0.32],[0.32,0.64]]
            //       = [[3.76, 1.22], [1.22, 3.34]]
            CheckClose(e1.Variance(0), 3.76, "4sa: Var(0) *= scalarEst");
            if constexpr (dim >= 2) {
                CheckClose(e1.Variance(1), 3.34, "4sa: Var(1) *= scalarEst");
                if constexpr (isFull) {
                    CheckClose(e1.Covariance(0, 1), 1.22, "4sa: Cov(0,1) *= scalarEst");
                }
            }
        }
    }

    // *= with zero-value scalar estimate (c.Value() == 0)
    // Implementation: fCov *= c^2 (=0), then CovRankUpdate(c.Variance(), fX),
    // then fX *= c.Value() (=0). So variance = c.Variance() * x^2.
    {
        auto e2{MakeEstimate<K, C>(v, cov)};
        Estimate<1, C> cZero{0.0, 0.16};
        e2 *= cZero;
        if constexpr (K == 1) {
            CheckClose(e2.Value(), 0.0, "4sb: Value() *=0 scalarEst");
            // fCov *= 0 then + c.Variance() * x^2 = 0.16 * 1 = 0.16
            CheckClose(e2.Variance(), 0.16, "4sb: Var() *=0 scalarEst");
        } else {
            CheckClose(e2.Value(0), 0.0, "4sb: Value(0) *=0 scalarEst");
            if constexpr (dim >= 2) {
                CheckClose(e2.Value(1), 0.0, "4sb: Value(1) *=0 scalarEst");
            }
            // fCov *= 0 then rank-1: Var(i) = 0.16 * v(i)^2
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e2.Variance(i), 0.16 * v(i) * v(i),
                           fmt::format("4sb: Var({}) *=0 scalarEst", i));
            }
            if constexpr (isFull and dim >= 2) {
                // Cov(i,j) = 0.16 * v(i) * v(j)
                CheckClose(e2.Covariance(0, 1), 0.16 * v(0) * v(1), "4sb: Cov(0,1) *=0 scalarEst");
            }
        }
    }

    // /= with scalar estimate (delegates to *= Inverse)
    {
        auto e3{MakeEstimate<K, C>(v, cov)};
        e3 /= c;
        // Inverse: invValue = 1/3, invVar = 0.16/81
        if constexpr (K == 1) {
            CheckClose(e3.Value(), 1.0 / 3.0, "4sc: Value() /= scalarEst");
            // Var = 0.4*(1/9) + (0.16/81)*1 = 0.04444... + 0.001975... = 0.04642...
            const double expectedVar{0.4 / 9.0 + (0.16 / 81.0) * 1.0};
            CheckClose(e3.Variance(), expectedVar, "4sc: Var() /= scalarEst");
        } else {
            CheckClose(e3.Value(0), 1.0 / 3.0, "4sc: Value(0) /= scalarEst");
            if constexpr (dim >= 2) {
                CheckClose(e3.Value(1), 2.0 / 3.0, "4sc: Value(1) /= scalarEst");
            }
            // Full cov: step1 *= (1/3)^2=1/9, step2 rank1 update (0.16/81)*x*x^T
            // [[0.4/9,0.1/9],[0.1/9,0.3/9]] + 0.16/81*[[1,2],[2,4]]
            const double invVar{0.16 / 81.0};
            CheckClose(e3.Variance(0), 0.4 / 9.0 + invVar * 1.0, "4sc: Var(0) /= scalarEst");
            if constexpr (dim >= 2) {
                CheckClose(e3.Variance(1), 0.3 / 9.0 + invVar * 4.0, "4sc: Var(1) /= scalarEst");
                if constexpr (isFull) {
                    CheckClose(e3.Covariance(0, 1), 0.1 / 9.0 + invVar * 2.0, "4sc: Cov(0,1) /= scalarEst");
                }
            }
        }
    }

    // Cross covariance option: Full receiving Diagonal scalar
    if constexpr (isFull and dim >= 2) {
        auto e4{MakeEstimate<K, C>(v, cov)};
        Estimate<1, CovarianceOption::Diagonal> cD{3.0, 0.16};
        e4 *= cD;
        CheckClose(e4.Value(0), 3.0, "4sd: Value(0) Full*=DiagScalar");
        CheckClose(e4.Variance(0), 3.76, "4sd: Var(0) Full*=DiagScalar");
        CheckClose(e4.Covariance(0, 1), 1.22, "4sd: Cov(0,1) Full*=DiagScalar");
    }

    // Cross covariance option: Diagonal receiving Full scalar
    if constexpr (not isFull) {
        auto e5{MakeEstimate<K, C>(v, cov)};
        Estimate<1, CovarianceOption::Full> cF{3.0, 0.16};
        e5 *= cF;
        if constexpr (K == 1) {
            CheckClose(e5.Value(), 3.0, "4se: Value() Diag*=FullScalar");
            CheckClose(e5.Variance(), 3.76, "4se: Var() Diag*=FullScalar");
        } else {
            CheckClose(e5.Value(0), 3.0, "4se: Value(0) Diag*=FullScalar");
            CheckClose(e5.Variance(0), 3.76, "4se: Var(0) Diag*=FullScalar");
        }
    }

    // Cross covariance option: Full /= Diagonal scalar
    if constexpr (isFull and dim >= 2) {
        auto e6{MakeEstimate<K, C>(v, cov)};
        Estimate<1, CovarianceOption::Diagonal> cD{3.0, 0.16};
        e6 /= cD;
        CheckClose(e6.Value(0), 1.0 / 3.0, "4sf: Value(0) Full/=DiagScalar");
        const double invVar{0.16 / 81.0};
        CheckClose(e6.Variance(0), 0.4 / 9.0 + invVar, "4sf: Var(0) Full/=DiagScalar");
        if constexpr (dim >= 2) {
            CheckClose(e6.Covariance(0, 1), 0.1 / 9.0 + invVar * 2.0, "4sf: Cov(0,1) Full/=DiagScalar");
        }
    }

    // K=1 self *= with scalar estimate (treated as independent)
    if constexpr (K == 1) {
        Estimate<1, C> cSelf{3.0, 0.16};
        cSelf *= cSelf;
        CheckClose(cSelf.Value(), 9.0, "4sg: Value() self*= scalarEst");
        // fCov *= 9 → 1.44, CovRankUpdate(0.16, 3.0) → +1.44, total = 2.88
        CheckClose(cSelf.Variance(), 2.88, "4sg: Var() self*= scalarEst");
    }
}};

} // namespace TestEstimateSection

auto TestEstimate4::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 4: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec4Smoke);
    PrintLn("  4 smoke passed: *= and /= compilation smoke");
    sec4SmokeCrossType();
    PrintLn("  4 smoke cross-type passed: cross *= and /= compilation smoke");

    PrintLn("--- Section 4: Arithmetic In-Place (*= and /=) ---");
    RunOverAllDims<AllStaticDims>(sec4MulDivInPlace);
    PrintLn("  4 passed: *= and /= (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 4: Arithmetic In-Place with Scalar Estimate (*= and /=) ---");
    RunOverAllDims<AllStaticDims>(sec4MulDivScalarEst);
    PrintLn("  4s passed: *= and /= with Estimate<1,D> (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestEstimate4 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
