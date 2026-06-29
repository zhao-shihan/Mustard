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

TestEstimate7::TestEstimate7() :
    Subprogram{"TestEstimate7", "Test Mustard::Math::Estimate (Section 7: Divide)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Section 7 Smoke: Divide in-place and free function
// =========================================================================

constexpr auto sec5DivideSmoke{[]<int K, CovarianceOption C>() {
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

    // ---- in-place Divide with Estimate, vector, scalar ----
    est = newEst();
    est = std::move(est).Divide(e3);
    if constexpr (K != 1) {
        est = std::move(est).Divide(x);
    }
    est = std::move(est).Divide(4.0);

    // ---- Divide with scalar estimate ----
    est = newEst();
    est = std::move(est).Divide(Estimate<1, C>{3.0, 0.16});
    {
        Estimate<1, C> c{3.0, 0.16};
        auto estConst{newEst()};
        auto r1{estConst.Divide(c)}; // const&
        auto r2{newEst().Divide(c)}; // && with lvalue arg
        vec = r1.Value();
        vec = r2.Value();
    }

    // ---- cross covariance option ----
    {
        if constexpr (C == CovarianceOption::Full) {
            Estimate<1, CovarianceOption::Diagonal> cD{3.0, 0.16};
            auto estC{newEst()};
            estC.DivideInPlace(cD);
        } else {
            Estimate<1, CovarianceOption::Full> cF{3.0, 0.16};
            auto estC{newEst()};
            estC.DivideInPlace(cF);
        }
    }
}};

// =========================================================================
// Section 7 Smoke CrossType: Divide cross-type
// =========================================================================

constexpr auto sec5DivideSmokeCrossType{[] {
    [[maybe_unused]] double scalar{};
    constexpr auto full{CovarianceOption::Full};
    constexpr auto diag{CovarianceOption::Diagonal};
    constexpr int dyn{Eigen::Dynamic};

    Estimate<1, full> cFull{3.0, 0.16};
    Estimate<1, diag> cDiag{3.0, 0.16};

    // E1: Static Full receiving Diagonal scalar
    {
        Estimate<3, full> ef3{MakeTestValue<3, full>(3), MakeTestCov<3, full>(3)};
        ef3.DivideInPlace(cDiag);
        scalar = ef3.Value(0);
    }

    // E2: Static Diag receiving Full scalar
    {
        Estimate<3, diag> ed3{MakeTestValue<3, diag>(3), MakeTestCov<3, diag>(3)};
        ed3.DivideInPlace(cFull);
        scalar = ed3.Variance(0);
    }

    // E3: Dynamic
    {
        auto xDyn3{MakeTestValue<dyn, full>(3)};
        Estimate<dyn, full> edf3{xDyn3, MakeTestCov<dyn, full>(3)};
        edf3.DivideInPlace(cDiag);
        scalar = edf3.Value(0);
    }

    // E4: K=1
    {
        Estimate<1, full> e1f{1.0, 0.5};
        e1f.DivideInPlace(cDiag);
        scalar = e1f.Value();
    }

    // E5: const& forwarding
    {
        const Estimate<3, full> e3f{MakeTestValue<3, full>(3), MakeTestCov<3, full>(3)};
        scalar = e3f.Divide(cDiag).Value(0);
    }
}};

// =========================================================================
// Section 7 Numerical: Divide with Estimate, vector, scalar
// =========================================================================

constexpr auto sec5Divide{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    const auto v{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    auto e2{MakeEstimate<K, C>(v, cov)};

    // DivideInPlace(e2)
    {
        auto e11{MakeEstimate<K, C>(v, cov)};
        e11.DivideInPlace(e2);
        if constexpr (K == 1) {
            CheckClose(e11.Value(), 1.0, "7a: Value() Divide(e2)");
        } else {
            CheckClose(e11.Value(0), 1.0, "7a: Value(0) Divide(e2)");
        }
        // Cov = diag(y/x^2)*Cov1*diag(y/x^2) + diag(1/x)*Cov2*diag(1/x)
        if constexpr (K != 1) {
            CheckBinaryCov(e11, cov, cov, "7a: Divide(e2) cov", [&](int i) { return 1.0 / v(i); }, [&](int i) { return 1.0 / v(i); });
        } else {
            CheckClose(e11.Variance(), 0.8, "7a: Var() Divide(e2)");
        }
    }

    // DivideInPlace(vec)
    if constexpr (K != 1) {
        auto e17{MakeEstimate<K, C>(v, cov)};
        e17.DivideInPlace(v);
        CheckClose(e17.Value(0), 1.0, "7b: Divide(vec) value(0)");
    }

    // DivideInPlace(scalar): x -> c/x, J = diag(-c/x^2)
    {
        auto e18{MakeEstimate<K, C>(v, cov)};
        e18.DivideInPlace(10.0);
        const auto jacDiv{[&](int i) { return -10.0 / (v(i) * v(i)); }};
        if constexpr (K == 1) {
            CheckClose(e18.Value(), 10.0, "7c: Divide(scalar) value()");
            CheckClose(e18.Variance(), jacDiv(0) * jacDiv(0) * cov.coeff(0, 0), "7c: Divide(scalar) var");
        } else {
            CheckClose(e18.Value(0), 10.0, "7c: Divide(scalar) value(0)");
            CheckMathFunction(e18, cov, "7c: Divide(scalar)",
                              [&](int i) { return -10.0 / (v(i) * v(i)); });
        }
    }
}};

// =========================================================================
// Section 7 Numerical: Divide with scalar Estimate<1,C>
//   (InPlace, const&, &&)
// =========================================================================

constexpr auto sec5DivideScalarEst{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    constexpr bool isFull{C == CovarianceOption::Full};

    const auto v{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    Estimate<1, C> c{3.0, 0.16};

    // DivideInPlace: x = c/x
    {
        auto eInPlace{MakeEstimate<K, C>(v, cov)};
        eInPlace.DivideInPlace(c);

        if constexpr (K == 1) {
            CheckClose(eInPlace.Value(), 3.0, "7d: Value() DivideInPlace scalarEst");
            // Var = 0.4*(3/(1)^2)^2 + 0.16*1 = 0.4*9 + 0.16 = 3.76
            CheckClose(eInPlace.Variance(), 3.76, "7d: Var() DivideInPlace scalarEst");
        } else {
            CheckClose(eInPlace.Value(0), 3.0, "7d: Value(0) DivideInPlace scalarEst");
            if constexpr (dim >= 2) {
                CheckClose(eInPlace.Value(1), 1.5, "7d: Value(1) DivideInPlace scalarEst");
                // Jac = c/x^2: [3/1^2, 3/2^2] = [3, 0.75]
                // Cov_scaled = diag(3,0.75)*cov*diag(3,0.75) + Var(c)*rank1(1/x)
                // Full: [[3.6,0.225],[0.225,0.16875]] + 0.16*[[1,0.5],[0.5,0.25]]
                // = [[3.76,0.305],[0.305,0.20875]]
                CheckClose(eInPlace.Variance(1), 0.20875, "7d: Var(1) DivideInPlace scalarEst");
                if constexpr (isFull) {
                    CheckClose(eInPlace.Covariance(0, 1), 0.305, "7d: Cov(0,1) DivideInPlace scalarEst");
                }
            }
            CheckClose(eInPlace.Variance(0), 3.76, "7d: Var(0) DivideInPlace scalarEst");
        }
    }

    // Divide const& and &&
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        auto eInPlace{MakeEstimate<K, C>(v, cov)};
        eInPlace.DivideInPlace(c);

        auto rConst{e1.Divide(c)};
        if constexpr (K == 1) {
            CheckClose(rConst.Value(), eInPlace.Value(), "7e: Divide const& vs InPlace value");
        } else {
            CheckClose(rConst.Value(0), eInPlace.Value(0), "7e: Divide const& vs InPlace value(0)");
        }

        auto eMove{MakeEstimate<K, C>(v, cov)};
        auto rMove{std::move(eMove).Divide(c)};
        if constexpr (K == 1) {
            CheckClose(rMove.Value(), eInPlace.Value(), "7f: Divide && vs InPlace value");
        } else {
            CheckClose(rMove.Value(0), eInPlace.Value(0), "7f: Divide && vs InPlace value(0)");
        }
    }
}};

// =========================================================================
// Section 7 Self: Divide self-operation (aliasing)
// =========================================================================

constexpr auto sec5DivideSelf{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    const auto v{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    // e.Divide(e) : f = v/v = 1, J_x = 1/v, J_y = 1/v
    {
        auto e{MakeEstimate<K, C>(v, cov)};
        e.DivideInPlace(e);
        if constexpr (K == 1) {
            CheckClose(e.Value(), 1.0, "7s: Value() self-Divide");
            CheckClose(e.Variance(), 0.8, "7s: Var() self-Divide");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), 1.0, fmt::format("7s: Value({}) self-Divide", i));
            }
            CheckBinaryCov(e, cov, cov, "7s: self-Divide cov", [&](int i) { return 1.0 / v(i); }, [&](int i) { return 1.0 / v(i); });
        }
    }

    // K=1 self-operation with scalar Estimate<1,D> overload
    if constexpr (K == 1) {
        // Self Divide with scalar estimate: c / c = 1
        {
            Estimate<1, C> cSelf{3.0, 0.16};
            cSelf.DivideInPlace(cSelf);
            CheckClose(cSelf.Value(), 1.0, "7t: Value() self-Divide scalarEst");
            const double expectedVar{0.16 / 9.0 + 0.16 / 9.0};
            CheckClose(cSelf.Variance(), expectedVar, "7t: Var() self-Divide scalarEst");
        }
    }
}};

} // namespace TestEstimateSection

auto TestEstimate7::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 7: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec5DivideSmoke);
    PrintLn("  7 smoke passed: Divide compilation smoke");
    sec5DivideSmokeCrossType();
    PrintLn("  7 smoke cross-type passed: Divide cross-type compilation smoke");

    PrintLn("--- Section 7: Divide with Estimate, Vector, Scalar ---");
    RunOverAllDims<AllStaticDims>(sec5Divide);
    PrintLn("  7 passed: Divide (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 7: Divide with Scalar Estimate<1> ---");
    RunOverAllDims<AllStaticDims>(sec5DivideScalarEst);
    PrintLn("  7s scalarEst passed: Divide with Estimate<1,D> (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 7: Self-Operations (aliasing) ---");
    RunOverAllDims<AllStaticDims>(sec5DivideSelf);
    PrintLn("  7 self passed: Divide self operations (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestEstimate7 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
