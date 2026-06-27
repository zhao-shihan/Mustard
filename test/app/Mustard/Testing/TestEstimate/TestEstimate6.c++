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

TestEstimate6::TestEstimate6() :
    Subprogram{"TestEstimate6", "Test Mustard::Math::Estimate (Section 6: NegateAdd)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Section 6 Smoke: NegateAdd in-place and free function
// =========================================================================

constexpr auto sec5NegateAddSmoke{[]<int K, CovarianceOption C>() {
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

    // ---- in-place NegateAdd with Estimate, vector, scalar ----
    est = newEst();
    est = std::move(est).NegateAdd(e3);
    if constexpr (K != 1) {
        est = std::move(est).NegateAdd(x);
    }
    est = std::move(est).NegateAdd(3.0);

    // ---- NegateAdd with scalar estimate ----
    est = newEst();
    est = std::move(est).NegateAdd(Estimate<1, C>{3.0, 0.16});
    {
        Estimate<1, C> c{3.0, 0.16};
        auto estConst{newEst()};
        auto r1{estConst.NegateAdd(c)}; // const&
        auto r2{newEst().NegateAdd(c)}; // && with lvalue arg
        vec = r1.Value();
        vec = r2.Value();
    }

    // ---- cross covariance option ----
    {
        if constexpr (C == CovarianceOption::Full) {
            Estimate<1, CovarianceOption::Diagonal> cD{3.0, 0.16};
            auto estC{newEst()};
            estC.NegateAddInPlace(cD);
        } else {
            Estimate<1, CovarianceOption::Full> cF{3.0, 0.16};
            auto estC{newEst()};
            estC.NegateAddInPlace(cF);
        }
    }
}};

// =========================================================================
// Section 6 Smoke CrossType: NegateAdd cross-type
// =========================================================================

constexpr auto sec5NegateAddSmokeCrossType{[] {
    [[maybe_unused]] double scalar{};
    constexpr auto full{CovarianceOption::Full};
    constexpr auto diag{CovarianceOption::Diagonal};
    constexpr int dyn{Eigen::Dynamic};

    Estimate<1, full> cFull{3.0, 0.16};
    Estimate<1, diag> cDiag{3.0, 0.16};

    // E1: Static Full receiving Diagonal scalar
    {
        Estimate<3, full> ef3{MakeTestValue<3, full>(3), MakeTestCov<3, full>(3)};
        ef3.NegateAddInPlace(cDiag);
        scalar = ef3.Value(0);
    }

    // E2: Static Diag receiving Full scalar
    {
        Estimate<3, diag> ed3{MakeTestValue<3, diag>(3), MakeTestCov<3, diag>(3)};
        ed3.NegateAddInPlace(cFull);
        scalar = ed3.Variance(0);
    }

    // E3: Dynamic
    {
        auto xDyn3{MakeTestValue<dyn, full>(3)};
        Estimate<dyn, full> edf3{xDyn3, MakeTestCov<dyn, full>(3)};
        edf3.NegateAddInPlace(cDiag);
        scalar = edf3.Value(0);
    }

    // E4: K=1
    {
        Estimate<1, full> e1f{1.0, 0.5};
        e1f.NegateAddInPlace(cDiag);
        scalar = e1f.Value();
    }

    // E5: const& forwarding
    {
        const Estimate<3, full> e3f{MakeTestValue<3, full>(3), MakeTestCov<3, full>(3)};
        scalar = e3f.NegateAdd(cDiag).Value(0);
    }
}};

// =========================================================================
// Section 6 Numerical: NegateAdd with Estimate, vector, scalar
// =========================================================================

constexpr auto sec5NegateAdd{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    const auto v{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    auto e2{MakeEstimate<K, C>(v, cov)};

    // NegateAddInPlace(e2)
    {
        auto e10{MakeEstimate<K, C>(v, cov)};
        e10.NegateAddInPlace(e2);
        if constexpr (K == 1) {
            CheckClose(e10.Value(), 0.0, "6a: Value() NegateAdd(e2)");
        } else {
            CheckClose(e10.Value(0), 0.0, "6a: Value(0) NegateAdd(e2)");
        }
        // Cov = Cov_x + Cov_y (f = y - x, J_x = -1, J_y = 1, minus sign squares away)
        if constexpr (K != 1) {
            CheckBinaryCov(e10, cov, cov, "6a: NegateAdd(e2) cov", [](int) { return 1.0; }, [](int) { return 1.0; });
        } else {
            CheckClose(e10.Variance(), 0.8, "6a: Var() NegateAdd(e2)");
        }
    }

    // NegateAddInPlace(vec)
    if constexpr (K != 1) {
        auto e15{MakeEstimate<K, C>(v, cov)};
        e15.NegateAddInPlace(v);
        CheckClose(e15.Value(0), 0.0, "6b: NegateAdd(vec) value(0)");
    }

    // NegateAddInPlace(scalar)
    {
        auto e16{MakeEstimate<K, C>(v, cov)};
        e16.NegateAddInPlace(10.0);
        if constexpr (K == 1) {
            CheckClose(e16.Value(), 9.0, "6c: NegateAdd(scalar) value()");
        } else {
            CheckClose(e16.Value(0), 9.0, "6c: NegateAdd(scalar) value(0)");
        }
    }
}};

// =========================================================================
// Section 6 Numerical: NegateAdd with scalar Estimate<1,C>
//   (InPlace, const&, &&)
// =========================================================================

constexpr auto sec5NegateAddScalarEst{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    constexpr bool isFull{C == CovarianceOption::Full};

    const auto v{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    Estimate<1, C> c{3.0, 0.16};

    // NegateAddInPlace: x = c - x, Cov += Var(c)*I
    {
        auto e1{MakeEstimate<K, C>(v, cov)};
        auto eInPlace{MakeEstimate<K, C>(v, cov)};
        eInPlace.NegateAddInPlace(c);

        if constexpr (K == 1) {
            CheckClose(eInPlace.Value(), 2.0, "6d: Value() NegateAddInPlace scalarEst");
            CheckClose(eInPlace.Variance(), 0.56, "6d: Var() NegateAddInPlace scalarEst");
        } else {
            CheckClose(eInPlace.Value(0), 2.0, "6d: Value(0) NegateAddInPlace scalarEst");
            if constexpr (dim >= 2) {
                CheckClose(eInPlace.Value(1), 1.0, "6d: Value(1) NegateAddInPlace scalarEst");
                CheckClose(eInPlace.Variance(1), 0.46, "6d: Var(1) NegateAddInPlace scalarEst");
                if constexpr (isFull) {
                    CheckClose(eInPlace.Covariance(0, 1), 0.26, "6d: Cov(0,1) NegateAddInPlace scalarEst");
                }
            }
            CheckClose(eInPlace.Variance(0), 0.56, "6d: Var(0) NegateAddInPlace scalarEst");
        }

        // const& version: copy then modify, should match InPlace
        {
            auto rConst{e1.NegateAdd(c)};
            if constexpr (K == 1) {
                CheckClose(rConst.Value(), eInPlace.Value(), "6e: NegateAdd const& vs InPlace value");
                CheckClose(rConst.Variance(), eInPlace.Variance(), "6e: NegateAdd const& vs InPlace var");
            } else {
                CheckClose(rConst.Value(0), eInPlace.Value(0), "6e: NegateAdd const& vs InPlace value(0)");
            }
            // Original e1 should be unchanged (const& makes a copy)
            if constexpr (K == 1) {
                CheckClose(e1.Value(), 1.0, "6e: original unchanged after NegateAdd const&");
            } else {
                CheckClose(e1.Value(0), 1.0, "6e: original unchanged after NegateAdd const&");
            }
        }

        // && version: move then modify, should match InPlace
        {
            auto eMove{MakeEstimate<K, C>(v, cov)};
            auto rMove{std::move(eMove).NegateAdd(c)};
            if constexpr (K == 1) {
                CheckClose(rMove.Value(), eInPlace.Value(), "6f: NegateAdd && vs InPlace value");
                CheckClose(rMove.Variance(), eInPlace.Variance(), "6f: NegateAdd && vs InPlace var");
            } else {
                CheckClose(rMove.Value(0), eInPlace.Value(0), "6f: NegateAdd && vs InPlace value(0)");
            }
        }
    }

    // NegateAdd cross covariance option
    if constexpr (isFull and dim >= 2) {
        auto eFull{MakeEstimate<K, C>(v, cov)};
        Estimate<1, CovarianceOption::Diagonal> cD{3.0, 0.16};
        eFull.NegateAddInPlace(cD);
        CheckClose(eFull.Value(0), 2.0, "6g: Value(0) Full NegateAdd DiagScalar");
        CheckClose(eFull.Variance(0), 0.56, "6g: Var(0) Full NegateAdd DiagScalar");
        // Off-diagonal also gets Var(c) since scalar variance adds to all elements
        CheckClose(eFull.Covariance(0, 1), 0.26, "6g: Cov(0,1) Full NegateAdd DiagScalar");
    }
}};

// =========================================================================
// Section 6 Self: NegateAdd self-operation (aliasing)
// =========================================================================

constexpr auto sec5NegateAddSelf{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    const auto v{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    // e.NegateAdd(e) : f = v - v = 0, Cov = Cov + Cov = 2*Cov
    {
        auto e{MakeEstimate<K, C>(v, cov)};
        e.NegateAddInPlace(e);
        if constexpr (K == 1) {
            CheckClose(e.Value(), 0.0, "6s: Value() self-NegateAdd");
            CheckClose(e.Variance(), 0.8, "6s: Var() self-NegateAdd");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), 0.0, fmt::format("6s: Value({}) self-NegateAdd", i));
            }
            CheckBinaryCov(e, cov, cov, "6s: self-NegateAdd cov", [](int) { return 1.0; }, [](int) { return 1.0; });
        }
    }

    // K=1 self-operation with scalar Estimate<1,D> overload
    if constexpr (K == 1) {
        // Self NegateAdd with scalar estimate: c - c = 0, Var adds
        {
            Estimate<1, C> cSelf{3.0, 0.16};
            cSelf.NegateAddInPlace(cSelf);
            CheckClose(cSelf.Value(), 0.0, "6t: Value() self-NegateAdd scalarEst");
            // Cov = Var + Var = 0.32 (treated as independent)
            CheckClose(cSelf.Variance(), 0.32, "6t: Var() self-NegateAdd scalarEst");
        }
    }
}};

} // namespace TestEstimateSection

auto TestEstimate6::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 6: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec5NegateAddSmoke);
    PrintLn("  6 smoke passed: NegateAdd compilation smoke");
    sec5NegateAddSmokeCrossType();
    PrintLn("  6 smoke cross-type passed: NegateAdd cross-type compilation smoke");

    PrintLn("--- Section 6: NegateAdd with Estimate, Vector, Scalar ---");
    RunOverAllDims<AllStaticDims>(sec5NegateAdd);
    PrintLn("  6 passed: NegateAdd (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 6: NegateAdd with Scalar Estimate<1> ---");
    RunOverAllDims<AllStaticDims>(sec5NegateAddScalarEst);
    PrintLn("  6s scalarEst passed: NegateAdd with Estimate<1,D> (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 6: Self-Operations (aliasing) ---");
    RunOverAllDims<AllStaticDims>(sec5NegateAddSelf);
    PrintLn("  6 self passed: NegateAdd self operations (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestEstimate6 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
