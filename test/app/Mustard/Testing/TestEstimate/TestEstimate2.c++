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

TestEstimate2::TestEstimate2() :
    Subprogram{"TestEstimate2", "Test Mustard::Math::Estimate (Section 2: += and -=)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Section 2 Smoke: += and -= (from sec0Smoke)
// =========================================================================

constexpr auto sec2Smoke{[]<int K, CovarianceOption C>() {
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

    // ---- compound assignment: +=, -= ----
    est += e3;
    if constexpr (K != 1) {
        est += x;
    }
    est += 1.0;

    est -= e3;
    if constexpr (K != 1) {
        est -= x;
    }
    est -= 1.0;

    // ---- compound assignment with scalar Estimate: +=, -= ----
    {
        Estimate<1, C> c{3.0, 0.09};
        est += c;
        est -= c;
    }
    // cross covariance option scalar estimate
    {
        if constexpr (C == CovarianceOption::Full) {
            Estimate<1, CovarianceOption::Diagonal> cD{3.0, 0.09};
            est += cD;
            est -= cD;
        } else {
            Estimate<1, CovarianceOption::Full> cF{3.0, 0.09};
            est += cF;
            est -= cF;
        }
    }
}};

// =========================================================================
// Section 2 Smoke CrossType: Cross += and -= (from sec0Smoke)
// =========================================================================

constexpr auto sec2SmokeCrossType{[] {
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
        efTmp += ed3;
        scalar = efTmp.Value(0);
        efTmp -= ed3;
        scalar = efTmp.Value(0);
        auto edTmp{ed3};
        edTmp += ef3;
        scalar = edTmp.Value(0);
        edTmp -= ef3;
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
        eTmp += edf3;
        scalar = eTmp.Value(0);
        eTmp -= edf3;
        scalar = eTmp.Value(0);
        auto edTmp{edf3};
        edTmp += e3f;
        scalar = edTmp.Value(0);
        edTmp -= e3f;
        scalar = edTmp.Value(0);
        Estimate<3, diag> e3d{MakeTestValue<3, diag>(3), cov3d};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};
        auto eTmp2{e3d};
        eTmp2 += edd3;
        scalar = eTmp2.Variance(0);
        eTmp2 -= edd3;
        scalar = eTmp2.Variance(0);
        auto edTmp2{edd3};
        edTmp2 += e3d;
        scalar = edTmp2.Variance(0);
        edTmp2 -= e3d;
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
        eTmp += edd3;
        scalar = eTmp.Value(0);
        eTmp -= edd3;
        scalar = eTmp.Value(0);
        auto edTmp{edd3};
        edTmp += edf3;
        scalar = edTmp.Variance(0);
        edTmp -= edf3;
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
        eTmp += edd3;
        scalar = eTmp.Value(0);
        eTmp -= edd3;
        scalar = eTmp.Value(0);
        auto edTmp{edd3};
        edTmp += e3f;
        scalar = edTmp.Variance(0);
        edTmp -= e3f;
        scalar = edTmp.Variance(0);
        Estimate<3, diag> e3d{MakeTestValue<3, diag>(3), cov3d};
        Estimate<dyn, full> edf3{xDyn3, covDynF3};
        auto eTmp2{e3d};
        eTmp2 += edf3;
        scalar = eTmp2.Variance(0);
        eTmp2 -= edf3;
        scalar = eTmp2.Variance(0);
        auto edTmp2{edf3};
        edTmp2 += e3d;
        scalar = edTmp2.Value(0);
        edTmp2 -= e3d;
        scalar = edTmp2.Value(0);
    }

    // E: Scalar Estimate<1> += and -= with various K/C combinations
    {
        Estimate<1, full> cFull{3.0, 0.09};
        Estimate<1, diag> cDiag{3.0, 0.09};

        // E1: Static K=3 Full receiving Diagonal scalar, and vice versa
        {
            const auto x3{MakeTestValue<3, full>(3)};
            const auto cov3f{MakeTestCov<3, full>(3)};
            const auto cov3d{MakeTestCov<3, diag>(3)};
            Estimate<3, full> ef3{x3, cov3f};
            ef3 += cDiag;
            scalar = ef3.Value(0);
            ef3 -= cDiag;
            scalar = ef3.Value(0);
            Estimate<3, diag> ed3{x3, cov3d};
            ed3 += cFull;
            scalar = ed3.Variance(0);
            ed3 -= cFull;
            scalar = ed3.Variance(0);
        }

        // E2: Dynamic full/diag receiving the opposite scalar type
        {
            const auto xDyn3{MakeTestValue<dyn, full>(3)};
            Estimate<dyn, full> edf3{xDyn3, MakeTestCov<dyn, full>(3)};
            edf3 += cDiag;
            scalar = edf3.Value(0);
            edf3 -= cDiag;
            scalar = edf3.Value(0);
            Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), MakeTestCov<dyn, diag>(3)};
            edd3 += cFull;
            scalar = edd3.Variance(0);
            edd3 -= cFull;
            scalar = edd3.Variance(0);
        }

        // E3: Static K=5 receiving scalar
        {
            Estimate<5, full> e5f{MakeTestValue<5, full>(5), MakeTestCov<5, full>(5)};
            e5f += cDiag;
            scalar = e5f.Value(4);
            e5f -= cDiag;
            scalar = e5f.Value(4);
        }

        // E4: Static K=1 receiving scalar (both same-C and cross-C)
        {
            Estimate<1, full> e1f{1.0, 0.5};
            e1f += cDiag;
            scalar = e1f.Value();
            e1f -= cDiag;
            scalar = e1f.Value();
            Estimate<1, diag> e1d{1.0, 0.5};
            e1d += cFull;
            scalar = e1d.Variance();
            e1d -= cFull;
            scalar = e1d.Variance();
        }
    }
}};

constexpr auto sec2AddSubInPlace{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    using Est = Estimate<K, C>;
    constexpr bool isFull{C == CovarianceOption::Full};

    const auto v{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    // e1 += e2
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        auto e2{MakeEstimate<K, C>(v, cov)};
        e1 += e2;
        if constexpr (K == 1) {
            CheckClose(e1.Value(), 2.0, "2a: Value() after +=");
            CheckClose(e1.Variance(), 0.8, "2a: Var() after +=");
        } else {
            for (auto i{0}; i < std::min(dim, 2); ++i) {
                CheckClose(e1.Value(i), 2.0 * v(i), fmt::format("2a: Value({}) after +=", i));
            }
            CheckClose(e1.Variance(0), 0.8, "2a: Var(0) after +=");
            if constexpr (dim >= 2) {
                CheckClose(e1.Variance(1), 0.6, "2a: Var(1) after +=");
            }
        }
    }

    // e1 -= e2
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        auto e2{MakeEstimate<K, C>(v, cov)};
        e1 -= e2;
        if constexpr (K == 1) {
            CheckClose(e1.Value(), 0.0, "2b: Value() after -=");
            CheckClose(e1.Variance(), 0.8, "2b: Var() after -=");
        } else {
            for (auto i{0}; i < std::min(dim, 2); ++i) {
                CheckClose(e1.Value(i), 0.0, fmt::format("2b: Value({}) after -=", i));
            }
            CheckClose(e1.Variance(0), 0.8, "2b: Var(0) after -=");
            if constexpr (dim >= 2) {
                CheckClose(e1.Variance(1), 0.6, "2b: Var(1) after -=");
            }
        }
    }

    // += with vector
    if constexpr (K != 1) {
        auto e1{MakeEstimate<K, C>(v, cov)};
        e1 += v;
        for (auto i{0}; i < std::min(dim, 2); ++i) {
            CheckClose(e1.Value(i), 2.0 * v(i), fmt::format("2d: Value({}) +=vec", i));
        }
        CheckClose(e1.Variance(0), 0.4, "2d: Var(0) +=vec unchanged");
    }

    // += with scalar
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        e1 += 10.0;
        if constexpr (K == 1) {
            CheckClose(e1.Value(), 11.0, "2e: Value() +=scalar");
            CheckClose(e1.Variance(), 0.4, "2e: Var() +=scalar unchanged");
        } else {
            CheckClose(e1.Value(0), 11.0, "2e: Value(0) +=scalar");
            if constexpr (dim >= 2) {
                CheckClose(e1.Value(1), 12.0, "2e: Value(1) +=scalar");
            }
            CheckClose(e1.Variance(0), 0.4, "2e: Var(0) +=scalar unchanged");
        }
    }

    // -= with vector/scalar
    if constexpr (K != 1) {
        auto e1{MakeEstimate<K, C>(v, cov)};
        e1 -= v;
        CheckClose(e1.Value(0), 0.0, "2f: Value(0) -=vec");
        if constexpr (dim >= 2) {
            CheckClose(e1.Value(1), 0.0, "2f: Value(1) -=vec");
        }
    }
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        e1 -= 5.0;
        if constexpr (K == 1) {
            CheckClose(e1.Value(), -4.0, "2f: Value() -=scalar");
        } else {
            CheckClose(e1.Value(0), -4.0, "2f: Value(0) -=scalar");
            if constexpr (dim >= 2) {
                CheckClose(e1.Value(1), -3.0, "2f: Value(1) -=scalar");
            }
        }
    }

    // Self +=
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        e1 += e1;
        if constexpr (K == 1) {
            CheckClose(e1.Value(), 2.0, "2c: Value() self+=");
            CheckClose(e1.Variance(), 0.8, "2c: Var() self+=");
        } else {
            CheckClose(e1.Value(0), 2.0, "2c: Value(0) self+=");
            if constexpr (dim >= 2) {
                CheckClose(e1.Value(1), 4.0, "2c: Value(1) self+=");
                CheckClose(e1.Variance(1), 0.6, "2c: Var(1) self+=");
            }
            CheckClose(e1.Variance(0), 0.8, "2c: Var(0) self+=");
        }
    }

    // Dimension mismatch (dynamic only)
    if constexpr (K == Eigen::Dynamic) {
        Est e1{2};
        e1 += MakeTestValue<K, C>(2);
        auto threw{false};
        try {
            Est e3{3};
            e1 += e3;
        } catch (const std::invalid_argument&) { threw = true; }
        if (not threw) {
            Throw<std::runtime_error>("2j: dimension mismatch should throw");
        }
    }

    if constexpr (isFull and K != 1 and dim >= 2) {
        // Full += Diagonal: off-diagonals from Full preserved
        const auto diagCov{MakeTestCov<K, CovarianceOption::Diagonal>(dim)};
        auto eD{MakeEstimate<K, CovarianceOption::Diagonal>(v, diagCov)};
        auto eF{MakeEstimate<K, C>(v, cov)};
        eF += eD;
        CheckClose(eF.Covariance(0, 1), 0.1, "2h: Cov(0,1) Full+=Diag preserved");
    }

    if constexpr (isFull and K != 1 and dim >= 2) {
        // Full *= Diagonal: Cov = diag(y)·Cov_full·diag(y) + diag contribution on diagonal
        // Off-diagonal: cov_ij = y_i * cov_full(i,j) * y_j
        const auto diagCov{MakeTestCov<K, CovarianceOption::Diagonal>(dim)};
        auto eD{MakeEstimate<K, CovarianceOption::Diagonal>(v, diagCov)};
        auto eF{MakeEstimate<K, C>(v, cov)};
        eF *= eD;
        CheckClose(eF.Value(0), v(0) * v(0), "2i: Value(0) Full*=Diag");
        CheckClose(eF.Value(1), v(1) * v(1), "2i: Value(1) Full*=Diag");
        CheckClose(eF.Covariance(0, 1), cov.coeff(0, 1) * v(0) * v(1), "2i: Cov(0,1) Full*=Diag");
    }

    if constexpr (isFull and K != 1 and dim >= 2) {
        // Full /= Diagonal: Cov = diag(1/y)·Cov_full·diag(1/y) + diag contribution on diagonal
        // Off-diagonal: cov_ij = cov_full(i,j) / (y_i * y_j)
        const auto diagCov{MakeTestCov<K, CovarianceOption::Diagonal>(dim)};
        auto eD{MakeEstimate<K, CovarianceOption::Diagonal>(v, diagCov)};
        auto eF{MakeEstimate<K, C>(v, cov)};
        eF /= eD;
        CheckClose(eF.Value(0), 1.0, "2j: Value(0) Full/=Diag");
        CheckClose(eF.Value(1), 1.0, "2j: Value(1) Full/=Diag");
        CheckClose(eF.Covariance(0, 1), cov.coeff(0, 1) / (v(0) * v(1)), "2j: Cov(0,1) Full/=Diag");
    }
}};

// =========================================================================
// Section 2 Functional: += and -= with scalar Estimate<1,D>
// =========================================================================

constexpr auto sec2AddSubScalarEst{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    constexpr bool isFull{C == CovarianceOption::Full};

    const auto v{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    Estimate<1, C> c{3.0, 0.16};

    // += with scalar estimate
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        e1 += c;
        if constexpr (K == 1) {
            CheckClose(e1.Value(), 4.0, "2sa: Value() += scalarEst");
            CheckClose(e1.Variance(), 0.56, "2sa: Var() += scalarEst");
        } else {
            for (auto i{0}; i < std::min(dim, 2); ++i) {
                CheckClose(e1.Value(i), v(i) + 3.0, fmt::format("2sa: Value({}) += scalarEst", i));
            }
            // Cov adds Var(c)=0.16 to every element
            CheckClose(e1.Variance(0), 0.56, "2sa: Var(0) += scalarEst");
            if constexpr (dim >= 2) {
                CheckClose(e1.Variance(1), 0.46, "2sa: Var(1) += scalarEst");
                if constexpr (isFull) {
                    CheckClose(e1.Covariance(0, 1), 0.26, "2sa: Cov(0,1) += scalarEst");
                }
            }
        }
    }

    // -= with scalar estimate
    {
        auto e2{MakeEstimate<K, C>(v, cov)};
        e2 -= c;
        if constexpr (K == 1) {
            CheckClose(e2.Value(), -2.0, "2sb: Value() -= scalarEst");
            CheckClose(e2.Variance(), 0.56, "2sb: Var() -= scalarEst");
        } else {
            for (auto i{0}; i < std::min(dim, 2); ++i) {
                CheckClose(e2.Value(i), v(i) - 3.0, fmt::format("2sb: Value({}) -= scalarEst", i));
            }
            // Cov adds Var(c) to every element (same as +=)
            CheckClose(e2.Variance(0), 0.56, "2sb: Var(0) -= scalarEst");
            if constexpr (dim >= 2) {
                CheckClose(e2.Variance(1), 0.46, "2sb: Var(1) -= scalarEst");
                if constexpr (isFull) {
                    CheckClose(e2.Covariance(0, 1), 0.26, "2sb: Cov(0,1) -= scalarEst");
                }
            }
        }
    }

    // += with zero-variance scalar estimate
    {
        auto e3{MakeEstimate<K, C>(v, cov)};
        Estimate<1, C> cZero{5.0, 0.0};
        e3 += cZero;
        if constexpr (K == 1) {
            CheckClose(e3.Value(), 6.0, "2sc: Value() += zeroVarScalarEst");
            CheckClose(e3.Variance(), 0.4, "2sc: Var() += zeroVarScalarEst unchanged");
        } else {
            for (auto i{0}; i < std::min(dim, 2); ++i) {
                CheckClose(e3.Value(i), v(i) + 5.0, fmt::format("2sc: Value({}) += zeroVarScalarEst", i));
            }
            CheckClose(e3.Variance(0), 0.4, "2sc: Var(0) += zeroVarScalarEst unchanged");
        }
    }

    // Cross covariance option: Full receiving Diagonal scalar
    if constexpr (isFull and dim >= 2) {
        auto e4{MakeEstimate<K, C>(v, cov)};
        Estimate<1, CovarianceOption::Diagonal> cD{3.0, 0.16};
        e4 += cD;
        CheckClose(e4.Value(0), 4.0, "2sd: Value(0) Full+=DiagScalar");
        CheckClose(e4.Variance(0), 0.56, "2sd: Var(0) Full+=DiagScalar");
        // Off-diagonal also gets Var(c) added
        CheckClose(e4.Covariance(0, 1), 0.26, "2sd: Cov(0,1) Full+=DiagScalar");
    }

    // Cross covariance option: Diagonal receiving Full scalar
    if constexpr (not isFull) {
        auto e5{MakeEstimate<K, C>(v, cov)};
        Estimate<1, CovarianceOption::Full> cF{3.0, 0.16};
        e5 += cF;
        if constexpr (K == 1) {
            CheckClose(e5.Variance(), 0.56, "2se: Var() Diag+=FullScalar");
        } else {
            CheckClose(e5.Variance(0), 0.56, "2se: Var(0) Diag+=FullScalar");
        }
    }

    // K=1 self += with scalar estimate (treated as independent)
    if constexpr (K == 1) {
        Estimate<1, C> cSelf{3.0, 0.16};
        cSelf += cSelf;
        CheckClose(cSelf.Value(), 6.0, "2sf: Value() self+= scalarEst");
        CheckClose(cSelf.Variance(), 0.32, "2sf: Var() self+= scalarEst");
    }
}};

// =========================================================================
// Section 2 Self: self -= (aliasing)
// =========================================================================

constexpr auto sec2SelfSub{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    const auto v{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    // e -= e : Cov(-x, +y) adds covariances -> Cov_result = 2*Cov
    {
        auto e{MakeEstimate<K, C>(v, cov)};
        e -= e;
        if constexpr (K == 1) {
            CheckClose(e.Value(), 0.0, "2sg: Value() self-=");
            CheckClose(e.Variance(), 0.8, "2sg: Var() self-=");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), 0.0, fmt::format("2sg: Value({}) self-=", i));
            }
            CheckBinaryCov(e, cov, cov, "2sg: self-= cov", [](int) { return 1.0; }, [](int) { return 1.0; });
        }
    }
}};

} // namespace TestEstimateSection

auto TestEstimate2::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 2: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec2Smoke);
    PrintLn("  2 smoke passed: += and -= compilation smoke");
    sec2SmokeCrossType();
    PrintLn("  2 smoke cross-type passed: cross += and -= compilation smoke");

    PrintLn("--- Section 2: Arithmetic In-Place (+= and -=) ---");
    RunOverAllDims<AllStaticDims>(sec2AddSubInPlace);
    PrintLn("  2 passed: += and -= (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 2: Arithmetic In-Place with Scalar Estimate (+= and -=) ---");
    RunOverAllDims<AllStaticDims>(sec2AddSubScalarEst);
    PrintLn("  2s passed: += and -= with Estimate<1,D> (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 2: Self-Operations (aliasing) ---");
    RunOverAllDims<AllStaticDims>(sec2SelfSub);
    PrintLn("  2 self passed: self -= (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestEstimate2 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
