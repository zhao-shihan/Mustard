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

TestEstimate8::TestEstimate8() :
    Subprogram{"TestEstimate8", "Test Mustard::Math::Estimate (Section 8: Pow, Exp, and pow)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Section 8 Smoke: Pow, Exp in-place, and pow free function
// =========================================================================

constexpr auto sec5PowExpSmoke{[]<int K, CovarianceOption C>() {
    using Est = Estimate<K, C>;
    constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    [[maybe_unused]] double scalar{};
    [[maybe_unused]] typename Est::ValueType vec{};

    const auto newEst{[&] {
        if constexpr (K == 1) {
            return Est{1.0, 0.5};
        } else {
            return Est{x, cov};
        }
    }};

    Est e3 = newEst();
    auto est{newEst()};

    // ---- in-place Pow with Estimate, vector, scalar ----
    est = newEst();
    est = std::move(est).Pow(e3);
    if constexpr (K != 1) {
        est = std::move(est).Pow(x);
    }
    est = std::move(est).Pow(2.0);

    // ---- in-place Exp with Estimate, vector, scalar ----
    est = newEst();
    est = std::move(est).Exp(e3);
    if constexpr (K != 1) {
        est = std::move(est).Exp(x);
    }
    est = std::move(est).Exp(2.0);

    // ---- free pow ----
    vec = pow(est, e3).Value();
    vec = pow(newEst(), e3).Value();
    vec = pow(est, newEst()).Value();
    vec = pow(newEst(), newEst()).Value();
    if constexpr (K != 1) {
        vec = pow(est, x).Value();
        vec = pow(x, est).Value();
    }
    vec = pow(est, 2.0).Value();
    vec = pow(2.0, est).Value();

    // ---- Pow with scalar estimate ----
    est = newEst();
    est = std::move(est).Pow(Estimate<1, C>{3.0, 0.16});
    {
        Estimate<1, C> c{3.0, 0.16};
        auto estConst{newEst()};
        auto r1{estConst.Pow(c)}; // const&
        auto r2{newEst().Pow(c)}; // && with lvalue arg
        vec = r1.Value();
        vec = r2.Value();
    }

    // ---- Exp with scalar estimate ----
    est = newEst();
    est = std::move(est).Exp(Estimate<1, C>{3.0, 0.16});
    {
        Estimate<1, C> c{3.0, 0.16};
        auto estConst{newEst()};
        auto r1{estConst.Exp(c)}; // const&
        auto r2{newEst().Exp(c)}; // && with lvalue arg
        vec = r1.Value();
        vec = r2.Value();
    }

    // ---- cross covariance option ----
    {
        if constexpr (C == CovarianceOption::Full) {
            Estimate<1, CovarianceOption::Diagonal> cD{3.0, 0.16};
            auto estC{newEst()};
            estC.PowInPlace(cD);
            estC = newEst();
            estC.ExpInPlace(cD);
        } else {
            Estimate<1, CovarianceOption::Full> cF{3.0, 0.16};
            auto estC{newEst()};
            estC.PowInPlace(cF);
            estC = newEst();
            estC.ExpInPlace(cF);
        }
    }
}};

// =========================================================================
// Section 8 Smoke CrossType: Pow, Exp, pow cross-type
// =========================================================================

constexpr auto sec5PowExpSmokeCrossType{[] {
    [[maybe_unused]] double scalar{};
    constexpr auto full{CovarianceOption::Full};
    constexpr auto diag{CovarianceOption::Diagonal};
    constexpr int dyn{Eigen::Dynamic};

    // A: Same K, different C (K=3, Full <-> Diag) -- pow only
    {
        const auto x3{MakeTestValue<3, full>(3)};
        const auto cov3f{MakeTestCov<3, full>(3)};
        const auto cov3d{MakeTestCov<3, diag>(3)};
        Estimate<3, full> ef3{x3, cov3f};
        Estimate<3, diag> ed3{x3, cov3d};
        scalar = pow(ef3, ed3).Value(0);
        scalar = pow(ed3, ef3).Value(0);
    }

    // B: Static <-> Dynamic, same C -- pow only
    {
        const auto x3{MakeTestValue<3, full>(3)};
        const auto cov3f{MakeTestCov<3, full>(3)};
        const auto cov3d{MakeTestCov<3, diag>(3)};
        const auto covDynF3{MakeTestCov<dyn, full>(3)};
        const auto covDynD3{MakeTestCov<dyn, diag>(3)};
        const auto xDyn3{MakeTestValue<dyn, full>(3)};
        Estimate<3, full> e3f{x3, cov3f};
        Estimate<dyn, full> edf3{xDyn3, covDynF3};
        scalar = pow(e3f, edf3).Value(0);
        scalar = pow(edf3, e3f).Value(0);
        Estimate<3, diag> e3d{MakeTestValue<3, diag>(3), cov3d};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};
        scalar = pow(e3d, edd3).Value(0);
        scalar = pow(edd3, e3d).Value(0);
    }

    // C: Dynamic <-> Dynamic -- pow only
    {
        const auto xDyn3{MakeTestValue<dyn, full>(3)};
        const auto covDynF3{MakeTestCov<dyn, full>(3)};
        const auto covDynD3{MakeTestCov<dyn, diag>(3)};
        Estimate<dyn, full> edf3{xDyn3, covDynF3};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};
        scalar = pow(edf3, edd3).Value(0);
        scalar = pow(edd3, edf3).Value(0);
    }

    // D: Static <-> Dynamic, different C -- pow only
    {
        const auto x3{MakeTestValue<3, full>(3)};
        const auto cov3f{MakeTestCov<3, full>(3)};
        const auto cov3d{MakeTestCov<3, diag>(3)};
        const auto xDyn3{MakeTestValue<dyn, full>(3)};
        const auto covDynF3{MakeTestCov<dyn, full>(3)};
        const auto covDynD3{MakeTestCov<dyn, diag>(3)};
        Estimate<3, full> e3f{x3, cov3f};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};
        scalar = pow(e3f, edd3).Value(0);
        scalar = pow(edd3, e3f).Value(0);
        Estimate<3, diag> e3d{MakeTestValue<3, diag>(3), cov3d};
        Estimate<dyn, full> edf3{xDyn3, covDynF3};
        scalar = pow(e3d, edf3).Value(0);
        scalar = pow(edf3, e3d).Value(0);
    }

    Estimate<1, full> cFull{3.0, 0.16};
    Estimate<1, diag> cDiag{3.0, 0.16};

    // E1: Static Full receiving Diagonal scalar -- Pow and Exp
    {
        Estimate<3, full> ef3{MakeTestValue<3, full>(3), MakeTestCov<3, full>(3)};
        ef3.PowInPlace(cDiag);
        scalar = ef3.Value(0);
        ef3 = Estimate<3, full>{MakeTestValue<3, full>(3), MakeTestCov<3, full>(3)};
        ef3.ExpInPlace(cDiag);
        scalar = ef3.Value(0);
    }

    // E2: Static Diag receiving Full scalar -- Pow and Exp
    {
        Estimate<3, diag> ed3{MakeTestValue<3, diag>(3), MakeTestCov<3, diag>(3)};
        ed3.PowInPlace(cFull);
        scalar = ed3.Variance(0);
        ed3 = Estimate<3, diag>{MakeTestValue<3, diag>(3), MakeTestCov<3, diag>(3)};
        ed3.ExpInPlace(cFull);
        scalar = ed3.Variance(0);
    }

    // E3: Dynamic -- Pow and Exp
    {
        auto xDyn3{MakeTestValue<dyn, full>(3)};
        Estimate<dyn, full> edf3{xDyn3, MakeTestCov<dyn, full>(3)};
        edf3.PowInPlace(cDiag);
        scalar = edf3.Value(0);
        edf3 = Estimate<dyn, full>{xDyn3, MakeTestCov<dyn, full>(3)};
        edf3.ExpInPlace(cDiag);
        scalar = edf3.Value(0);
    }

    // E4: K=1
    {
        Estimate<1, full> e1f{1.0, 0.5};
        e1f.PowInPlace(cDiag);
        scalar = e1f.Value();
    }

    // E5: const& forwarding
    {
        const Estimate<3, full> e3f{MakeTestValue<3, full>(3), MakeTestCov<3, full>(3)};
        scalar = e3f.Pow(cDiag).Value(0);
        scalar = e3f.Exp(cDiag).Value(0);
    }
}};

// =========================================================================
// Section 8 Numerical: pow free function, Pow, Exp
// =========================================================================

constexpr auto sec5PowExp{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    const auto v{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    auto e1{MakeEstimate<K, C>(v, cov)};
    auto e2{MakeEstimate<K, C>(v, cov)};

    // pow(e1, e2) free function
    {
        const auto e12{pow(MakeEstimate<K, C>(v, cov), e2)};
        if constexpr (K == 1) {
            CheckClose(e12.Value(), 1.0, "8a: Value() pow(e1,e2)");
        } else {
            CheckClose(e12.Value(0), 1.0, "8a: Value(0) pow(e1,e2)");
        }
        // Cov = diag(y*x^(y-1))*Cov1*diag(y*x^(y-1)) + diag(x^y*log(x))*Cov2*diag(x^y*log(x))
        if constexpr (K != 1) {
            CheckBinaryCov(e12, cov, cov, "8a: pow(e1,e2) cov", [&](int i) { return std::pow(v(i), v(i)); }, [&](int i) { return std::pow(v(i), v(i)) * std::log(v(i)); });
        } else {
            auto jacX{std::pow(v(0), v(0))};
            auto jacY{std::pow(v(0), v(0)) * std::log(v(0))};
            CheckClose(e12.Variance(), (jacX * jacX + jacY * jacY) * cov.diagonal()(0), "8a: Var() pow(e1,e2)");
        }
    }

    // pow(e, scalar)
    {
        const auto e13{pow(e1, 2.0)};
        if constexpr (K == 1) {
            CheckClose(e13.Value(), 1.0, "8b: Value() pow(e,2)");
        } else {
            CheckClose(e13.Value(0), 1.0, "8b: Value(0) pow(e,2)");
            if constexpr (dim >= 2) {
                CheckClose(e13.Value(1), 4.0, "8b: Value(1) pow(e,2)");
            }
        }
    }

    // pow(double, Estimate)
    {
        const auto e19{pow(2.0, MakeEstimate<K, C>(v, cov))};
        if constexpr (K == 1) {
            CheckClose(e19.Value(), 2.0, "8c: pow(scalar,e) value()");
        } else {
            CheckClose(e19.Value(0), 2.0, "8c: pow(scalar,e) value(0)");
        }
    }

    // ExpInPlace(base, exponent) -- base is argument, exponent is this
    {
        auto e14{MakeEstimate<K, C>(v, cov)};
        e14.ExpInPlace(e2);
        if constexpr (K == 1) {
            CheckClose(e14.Value(), 1.0, "8d: Value() Exp(base,exp)");
        } else {
            CheckClose(e14.Value(0), 1.0, "8d: Value(0) Exp(base,exp)");
        }
        // Cov = diag(y^x*log(y))*Cov1*diag(y^x*log(y)) + diag(x/y*y^x)*Cov2*diag(x/y*y^x)
        if constexpr (K != 1) {
            CheckBinaryCov(e14, cov, cov, "8d: Exp(base,e) cov", [&](int i) { return std::pow(v(i), v(i)) * std::log(v(i)); }, [&](int i) { return std::pow(v(i), v(i)); });
        } else {
            auto jacX{std::pow(v(0), v(0)) * std::log(v(0))};
            auto jacY{std::pow(v(0), v(0))};
            CheckClose(e14.Variance(), (jacX * jacX + jacY * jacY) * cov.diagonal()(0), "8d: Var() Exp(base,e)");
        }
    }

    // PowInPlace(vec): x -> x^y
    if constexpr (K != 1) {
        auto eVecPow{MakeEstimate<K, C>(v, cov)};
        eVecPow.PowInPlace(v);
        CheckClose(eVecPow.Value(0), std::pow(v(0), v(0)), "8e: Pow(vec) value(0)");
        CheckMathFunction(eVecPow, cov, "8e: Pow(vec)",
                          [&](int i) {
                              auto powVal{std::pow(v(i), v(i))};
                              return v(i) / v(i) * powVal;
                          });
    }

    // PowInPlace(scalar): x -> x^c
    {
        auto eScalarPow{MakeEstimate<K, C>(v, cov)};
        eScalarPow.PowInPlace(3.0);
        constexpr double cPow{3.0};
        if constexpr (K == 1) {
            CheckClose(eScalarPow.Value(), std::pow(v(0), cPow), "8f: Pow(scalar) value()");
            auto jac{cPow / v(0) * std::pow(v(0), cPow)};
            CheckClose(eScalarPow.Variance(), jac * jac * cov.coeff(0, 0), "8f: Pow(scalar) var");
        } else {
            CheckClose(eScalarPow.Value(0), std::pow(v(0), cPow), "8f: Pow(scalar) value(0)");
            CheckMathFunction(eScalarPow, cov, "8f: Pow(scalar)",
                              [&](int i) { return cPow / v(i) * std::pow(v(i), cPow); });
        }
    }

    // ExpInPlace(vec): x -> y^x
    if constexpr (K != 1) {
        // Use y = [2, 3, ...] to avoid log(1)=0 masking
        Eigen::Vector<double, K> baseVec;
        if constexpr (K == Eigen::Dynamic) {
            baseVec.resize(dim);
        }
        baseVec.setConstant(2.0);
        auto eVecExp{MakeEstimate<K, C>(v, cov)};
        eVecExp.ExpInPlace(baseVec);
        CheckClose(eVecExp.Value(0), std::pow(2.0, v(0)), "8g: Exp(vec) value(0)");
        CheckMathFunction(eVecExp, cov, "8g: Exp(vec)",
                          [&](int i) {
                              return std::pow(2.0, v(i)) * std::log(2.0);
                          });
    }

    // ExpInPlace(scalar): x -> c^x
    {
        auto eScalarExp{MakeEstimate<K, C>(v, cov)};
        eScalarExp.ExpInPlace(2.0);
        constexpr double cExp{2.0};
        if constexpr (K == 1) {
            CheckClose(eScalarExp.Value(), std::pow(cExp, v(0)), "8h: Exp(scalar) value()");
            auto jac{std::pow(cExp, v(0)) * std::log(cExp)};
            CheckClose(eScalarExp.Variance(), jac * jac * cov.coeff(0, 0), "8h: Exp(scalar) var");
        } else {
            CheckClose(eScalarExp.Value(0), std::pow(cExp, v(0)), "8h: Exp(scalar) value(0)");
            CheckMathFunction(eScalarExp, cov, "8h: Exp(scalar)",
                              [&](int i) { return std::pow(cExp, v(i)) * std::log(cExp); });
        }
    }
}};

// =========================================================================
// Section 8 Numerical: Pow/Exp with scalar Estimate<1,C>
//   (InPlace, const&, &&)
// =========================================================================

constexpr auto sec5PowExpScalarEst{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    constexpr bool isFull{C == CovarianceOption::Full};

    const auto v{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    Estimate<1, C> c{3.0, 0.16};

    // ---- Pow with scalar estimate ----
    // PowInPlace: x = x^c
    {
        auto eInPlace{MakeEstimate<K, C>(v, cov)};
        eInPlace.PowInPlace(c);

        if constexpr (K == 1) {
            CheckClose(eInPlace.Value(), 1.0, "8i: Value() PowInPlace scalarEst");
            // Var = 0.4*(3/1*1)^2 + 0.16*(1*0)^2 = 0.4*9 = 3.6
            CheckClose(eInPlace.Variance(), 3.6, "8i: Var() PowInPlace scalarEst");
        } else {
            CheckClose(eInPlace.Value(0), 1.0, "8i: Value(0) PowInPlace scalarEst");
            if constexpr (dim >= 2) {
                CheckClose(eInPlace.Value(1), 8.0, "8i: Value(1) PowInPlace scalarEst");
                // jacX = c/x*x^c: [3/1*1, 3/2*8] = [3, 12]
                // rank1 = x^c*log(x): [0, 8*ln(2)]
                const double ln2{std::log(2.0)};
                const double rank1Val{8.0 * ln2};
                CheckClose(eInPlace.Variance(1), 43.2 + 0.16 * rank1Val * rank1Val, "8i: Var(1) PowInPlace scalarEst");
                if constexpr (isFull) {
                    CheckClose(eInPlace.Covariance(0, 1), 3.6, "8i: Cov(0,1) PowInPlace scalarEst");
                }
            }
            CheckClose(eInPlace.Variance(0), 3.6, "8i: Var(0) PowInPlace scalarEst");
        }
    }

    // Pow const& and &&
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        auto eInPlace{MakeEstimate<K, C>(v, cov)};
        eInPlace.PowInPlace(c);

        auto rConst{e1.Pow(c)};
        auto eMove{MakeEstimate<K, C>(v, cov)};
        auto rMove{std::move(eMove).Pow(c)};
        if constexpr (K == 1) {
            CheckClose(rConst.Value(), eInPlace.Value(), "8j: Pow const& vs InPlace value");
            CheckClose(rMove.Value(), eInPlace.Value(), "8j: Pow && vs InPlace value");
        } else {
            CheckClose(rConst.Value(0), eInPlace.Value(0), "8j: Pow const& vs InPlace value(0)");
            CheckClose(rMove.Value(0), eInPlace.Value(0), "8j: Pow && vs InPlace value(0)");
        }
    }

    // Pow with c=2.0
    {
        Estimate<1, C> c2{2.0, 0.25};
        auto ePow2{MakeEstimate<K, C>(v, cov)};
        ePow2.PowInPlace(c2);
        if constexpr (K == 1) {
            CheckClose(ePow2.Value(), 1.0, "8k: Value() Pow(c=2) scalarEst");
            // Var = 0.4*(2/1*1)^2 + 0.25*(1*0)^2 = 0.4*4 = 1.6
            CheckClose(ePow2.Variance(), 1.6, "8k: Var() Pow(c=2) scalarEst");
        } else {
            CheckClose(ePow2.Value(0), 1.0, "8k: Value(0) Pow(c=2) scalarEst");
            if constexpr (dim >= 2) {
                CheckClose(ePow2.Value(1), 4.0, "8k: Value(1) Pow(c=2) scalarEst");
            }
        }
    }

    // ---- Exp with scalar estimate ----
    // ExpInPlace: x = c^x
    {
        Estimate<1, C> cExp{3.0, 0.16};
        auto eInPlace{MakeEstimate<K, C>(v, cov)};
        eInPlace.ExpInPlace(cExp);

        const double ln3{std::log(3.0)};
        if constexpr (K == 1) {
            CheckClose(eInPlace.Value(), 3.0, "8l: Value() ExpInPlace scalarEst");
            const double expectedVar{0.4 * 9.0 * ln3 * ln3 + 0.16};
            CheckClose(eInPlace.Variance(), expectedVar, "8l: Var() ExpInPlace scalarEst");
        } else {
            CheckClose(eInPlace.Value(0), 3.0, "8l: Value(0) ExpInPlace scalarEst");
            if constexpr (dim >= 2) {
                CheckClose(eInPlace.Value(1), 9.0, "8l: Value(1) ExpInPlace scalarEst");
                const double jac0{3.0 * ln3};
                const double jac1{9.0 * ln3};
                const double expectedVar0{0.4 * jac0 * jac0 + 0.16 * 1.0};
                const double expectedVar1{0.3 * jac1 * jac1 + 0.16 * 36.0};
                CheckClose(eInPlace.Variance(0), expectedVar0, "8l: Var(0) ExpInPlace scalarEst");
                CheckClose(eInPlace.Variance(1), expectedVar1, "8l: Var(1) ExpInPlace scalarEst");
                if constexpr (isFull) {
                    const double expectedCov01{0.1 * jac0 * jac1 + 0.16 * 6.0};
                    CheckClose(eInPlace.Covariance(0, 1), expectedCov01, "8l: Cov(0,1) ExpInPlace scalarEst");
                }
            }
        }
    }

    // Exp const& and &&
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        auto eInPlace{MakeEstimate<K, C>(v, cov)};
        eInPlace.ExpInPlace(c);

        auto rConst{e1.Exp(c)};
        auto eMove{MakeEstimate<K, C>(v, cov)};
        auto rMove{std::move(eMove).Exp(c)};
        if constexpr (K == 1) {
            CheckClose(rConst.Value(), eInPlace.Value(), "8m: Exp const& vs InPlace value");
            CheckClose(rMove.Value(), eInPlace.Value(), "8m: Exp && vs InPlace value");
        } else {
            CheckClose(rConst.Value(0), eInPlace.Value(0), "8m: Exp const& vs InPlace value(0)");
            CheckClose(rMove.Value(0), eInPlace.Value(0), "8m: Exp && vs InPlace value(0)");
        }
    }

    // Exp with c=2.0
    {
        Estimate<1, C> c2{2.0, 0.25};
        auto eExp2{MakeEstimate<K, C>(v, cov)};
        eExp2.ExpInPlace(c2);
        const double ln2{std::log(2.0)};
        if constexpr (K == 1) {
            CheckClose(eExp2.Value(), 2.0, "8n: Value() Exp(c=2) scalarEst");
            const double expectedVar{0.4 * 4.0 * ln2 * ln2 + 0.25 * 1.0};
            CheckClose(eExp2.Variance(), expectedVar, "8n: Var() Exp(c=2) scalarEst");
        } else {
            CheckClose(eExp2.Value(0), 2.0, "8n: Value(0) Exp(c=2) scalarEst");
        }
    }

    // Exp cross covariance option
    if constexpr (isFull and dim >= 2) {
        auto eFull{MakeEstimate<K, C>(v, cov)};
        Estimate<1, CovarianceOption::Diagonal> cD{3.0, 0.16};
        eFull.ExpInPlace(cD);
        CheckClose(eFull.Value(0), 3.0, "8o: Value(0) Full Exp DiagScalar");
    }
    if constexpr (not isFull) {
        auto eDiag{MakeEstimate<K, C>(v, cov)};
        Estimate<1, CovarianceOption::Full> cF{3.0, 0.16};
        eDiag.ExpInPlace(cF);
        if constexpr (K == 1) {
            CheckClose(eDiag.Value(), 3.0, "8o: Value() Diag Exp FullScalar");
        } else {
            CheckClose(eDiag.Value(0), 3.0, "8o: Value(0) Diag Exp FullScalar");
        }
    }
}};

// =========================================================================
// Section 8 Self: Pow, Exp, pow self-operations (aliasing)
// =========================================================================

constexpr auto sec5PowExpSelf{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    const auto v{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    // e.Pow(e) : f = v^v
    {
        auto e{MakeEstimate<K, C>(v, cov)};
        e.PowInPlace(e);
        if constexpr (K == 1) {
            CheckClose(e.Value(), 1.0, "8s: Value() self-Pow");
            auto jacX{std::pow(v(0), v(0))};
            auto jacY{std::pow(v(0), v(0)) * std::log(v(0))};
            CheckClose(e.Variance(), (jacX * jacX + jacY * jacY) * cov.diagonal()(0), "8s: Var() self-Pow");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::pow(v(i), v(i)), fmt::format("8s: Value({}) self-Pow", i));
            }
            CheckBinaryCov(e, cov, cov, "8s: self-Pow cov", [&](int i) { return std::pow(v(i), v(i)); }, [&](int i) { return std::pow(v(i), v(i)) * std::log(v(i)); });
        }
    }

    // e.Exp(e) : f = v^v
    {
        auto e{MakeEstimate<K, C>(v, cov)};
        e.ExpInPlace(e);
        if constexpr (K == 1) {
            CheckClose(e.Value(), 1.0, "8s: Value() self-Exp");
            auto jacX{std::pow(v(0), v(0)) * std::log(v(0))};
            auto jacY{std::pow(v(0), v(0))};
            CheckClose(e.Variance(), (jacX * jacX + jacY * jacY) * cov.diagonal()(0), "8s: Var() self-Exp");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), std::pow(v(i), v(i)), fmt::format("8s: Value({}) self-Exp", i));
            }
            CheckBinaryCov(e, cov, cov, "8s: self-Exp cov", [&](int i) { return std::pow(v(i), v(i)) * std::log(v(i)); }, [&](int i) { return std::pow(v(i), v(i)); });
        }
    }

    // K=1 self-operations with scalar Estimate<1,D> overload
    if constexpr (K == 1) {
        // Self Pow with scalar estimate: c^c (c=3, so 3^3=27)
        {
            Estimate<1, C> cSelf{3.0, 0.16};
            cSelf.PowInPlace(cSelf);
            CheckClose(cSelf.Value(), 27.0, "8t: Value() self-Pow scalarEst");
            const double ln3{std::log(3.0)};
            const double jacX{27.0};
            const double rank1Val{27.0 * ln3};
            const double expectedVar{0.16 * jacX * jacX + 0.16 * rank1Val * rank1Val};
            CheckClose(cSelf.Variance(), expectedVar, "8t: Var() self-Pow scalarEst");
        }

        // Self Exp with scalar estimate: c^c = 27
        {
            Estimate<1, C> cSelf{3.0, 0.16};
            cSelf.ExpInPlace(cSelf);
            CheckClose(cSelf.Value(), 27.0, "8t: Value() self-Exp scalarEst");
            const double ln3{std::log(3.0)};
            const double jacX{27.0 * ln3};
            const double rank1Val{27.0};
            const double expectedVar{0.16 * jacX * jacX + 0.16 * rank1Val * rank1Val};
            CheckClose(cSelf.Variance(), expectedVar, "8t: Var() self-Exp scalarEst");
        }
    }
}};

} // namespace TestEstimateSection

auto TestEstimate8::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 8: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec5PowExpSmoke);
    PrintLn("  8 smoke passed: Pow, Exp, pow compilation smoke");
    sec5PowExpSmokeCrossType();
    PrintLn("  8 smoke cross-type passed: Pow, Exp, pow cross-type compilation smoke");

    PrintLn("--- Section 8: pow, Pow, Exp Numerical ---");
    RunOverAllDims<AllStaticDims>(sec5PowExp);
    PrintLn("  8 passed: pow, Pow, Exp (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 8: Pow/Exp with Scalar Estimate<1> ---");
    RunOverAllDims<AllStaticDims>(sec5PowExpScalarEst);
    PrintLn("  8s scalarEst passed: Pow/Exp with Estimate<1,D> (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 8: Self-Operations (aliasing) ---");
    RunOverAllDims<AllStaticDims>(sec5PowExpSelf);
    PrintLn("  8 self passed: Pow/Exp self operations (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestEstimate8 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
