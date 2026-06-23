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

TestEstimate3::TestEstimate3() :
    Subprogram{"TestEstimate3", "Test Mustard::Math::Estimate (Section 3: +, -, unary -)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Section 3 Smoke: Free Unary and Binary +,- (from sec0Smoke)
// =========================================================================

constexpr auto sec3Smoke{[]<int K, CovarianceOption C>() {
    using Est = Estimate<K, C>;
    constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

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

    // ---- free unary operators ----
    vec = operator+(est).Value();
    vec = operator+(std::as_const(est)).Value();
    vec = operator+(newEst()).Value();
    vec = operator+(static_cast<const Est&&>(newEst())).Value();
    vec = operator-(newEst()).Value();

    // ---- free binary +, - ----
    // operator+
    vec = (est + e3).Value();
    vec = (newEst() + e3).Value();
    vec = (est + newEst()).Value();
    vec = (newEst() + newEst()).Value();
    if constexpr (K != 1) {
        vec = (est + x).Value();
        vec = (x + est).Value();
    }
    vec = (est + 1.0).Value();
    vec = (1.0 + est).Value();

    // operator-
    vec = (est - e3).Value();
    vec = (newEst() - e3).Value();
    vec = (est - newEst()).Value();
    vec = (newEst() - newEst()).Value();
    if constexpr (K != 1) {
        vec = (est - x).Value();
        vec = (x - est).Value();
    }
    vec = (est - 1.0).Value();
    vec = (1.0 - est).Value();
}};

// =========================================================================
// Section 3 Smoke CrossType: Cross + and - (from sec0Smoke)
// =========================================================================

constexpr auto sec3SmokeCrossType{[] {
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
        scalar = (ef3 + ed3).Value(0);
        scalar = (ed3 + ef3).Value(0);
        scalar = (ef3 - ed3).Value(0);
        scalar = (ed3 - ef3).Value(0);
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
        scalar = (e3f + edf3).Value(0);
        scalar = (edf3 + e3f).Value(0);
        scalar = (e3f - edf3).Value(0);
        scalar = (edf3 - e3f).Value(0);
        Estimate<3, diag> e3d{MakeTestValue<3, diag>(3), cov3d};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};
        scalar = (e3d + edd3).Value(0);
        scalar = (edd3 + e3d).Value(0);
        scalar = (e3d - edd3).Value(0);
        scalar = (edd3 - e3d).Value(0);
    }

    // C: Dynamic ↔ Dynamic
    {
        const auto xDyn3{MakeTestValue<dyn, full>(3)};
        const auto covDynF3{MakeTestCov<dyn, full>(3)};
        const auto covDynD3{MakeTestCov<dyn, diag>(3)};
        Estimate<dyn, full> edf3{xDyn3, covDynF3};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};
        scalar = (edf3 + edd3).Value(0);
        scalar = (edd3 + edf3).Value(0);
        scalar = (edf3 - edd3).Value(0);
        scalar = (edd3 - edf3).Value(0);
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
        scalar = (e3f + edd3).Value(0);
        scalar = (edd3 + e3f).Value(0);
        scalar = (e3f - edd3).Value(0);
        scalar = (edd3 - e3f).Value(0);
        Estimate<3, diag> e3d{MakeTestValue<3, diag>(3), cov3d};
        Estimate<dyn, full> edf3{xDyn3, covDynF3};
        scalar = (e3d + edf3).Value(0);
        scalar = (edf3 + e3d).Value(0);
        scalar = (e3d - edf3).Value(0);
        scalar = (edf3 - e3d).Value(0);
    }
}};

constexpr auto sec3FreeAddSub{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    const auto v{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    auto e1{MakeEstimate<K, C>(v, cov)};
    auto e2{MakeEstimate<K, C>(v, cov)};

    // e1 + e2
    {
        const auto e3{e1 + e2};
        if constexpr (K == 1) {
            CheckClose(e3.Value(), 2.0, "3a: Value() e1+e2");
            CheckClose(e3.Variance(), 0.8, "3a: Var() e1+e2");
        } else {
            CheckClose(e3.Value(0), 2.0, "3a: Value(0) e1+e2");
            if constexpr (dim >= 2) {
                CheckClose(e3.Value(1), 4.0, "3a: Value(1) e1+e2");
                CheckClose(e3.Variance(1), 0.6, "3a: Var(1) e1+e2");
            }
            CheckClose(e3.Variance(0), 0.8, "3a: Var(0) e1+e2");
        }
    }

    // e1 - e2
    {
        const auto e4{e1 - e2};
        if constexpr (K == 1) {
            CheckClose(e4.Value(), 0.0, "3b: Value() e1-e2");
            CheckClose(e4.Variance(), 0.8, "3b: Var() e1-e2");
        } else {
            CheckClose(e4.Value(0), 0.0, "3b: Value(0) e1-e2");
            if constexpr (dim >= 2) {
                CheckClose(e4.Value(1), 0.0, "3b: Value(1) e1-e2");
            }
            CheckClose(e4.Variance(0), 0.8, "3b: Var(0) e1-e2");
        }
    }

    // Unary -
    {
        const auto e5{-e1};
        if constexpr (K == 1) {
            CheckClose(e5.Value(), -1.0, "3f: Value() -e");
            CheckClose(e5.Variance(), e1.Variance(), "3f: Var() -e unchanged");
        } else {
            CheckClose(e5.Value(0), -1.0, "3f: Value(0) -e");
            if constexpr (dim >= 2) {
                CheckClose(e5.Value(1), -2.0, "3f: Value(1) -e");
            }
            CheckClose(e5.Variance(0), e1.Variance(0), "3f: Var(0) -e unchanged");
        }
    }

    // e + vector
    if constexpr (K != 1) {
        const auto e6{e1 + v};
        CheckClose(e6.Value(0), 2.0, "3g: Value(0) e+vec");
        CheckClose(e6.Variance(0), 0.4, "3g: Var(0) e+vec unchanged");
    }

    // vector + e
    if constexpr (K != 1) {
        const auto e7{v + e1};
        CheckClose(e7.Value(0), 2.0, "3g: Value(0) vec+e");
        CheckClose(e7.Variance(0), 0.4, "3g: Var(0) vec+e unchanged");
    }

    // e + scalar
    {
        const auto e8{e1 + 10.0};
        if constexpr (K == 1) {
            CheckClose(e8.Value(), 11.0, "3h: Value() e+scalar");
            CheckClose(e8.Variance(), 0.4, "3h: Var() e+scalar unchanged");
        } else {
            CheckClose(e8.Value(0), 11.0, "3h: Value(0) e+scalar");
            CheckClose(e8.Variance(0), 0.4, "3h: Var(0) e+scalar unchanged");
        }
    }

    // scalar + e
    {
        const auto e9{10.0 + e1};
        if constexpr (K == 1) {
            CheckClose(e9.Value(), 11.0, "3h: Value() scalar+e");
        } else {
            CheckClose(e9.Value(0), 11.0, "3h: Value(0) scalar+e");
        }
    }

    // e - vector
    if constexpr (K != 1) {
        const auto e10{e1 - v};
        CheckClose(e10.Value(0), 0.0, "3i: Value(0) e-vec");
    }

    // vector - e (uses NegateAdd)
    if constexpr (K != 1) {
        const auto e11{v - e1};
        CheckClose(e11.Value(0), 0.0, "3i: Value(0) vec-e");
        CheckClose(e11.Variance(0), 0.4, "3i: Var(0) vec-e unchanged");
    }
}};

} // namespace TestEstimateSection

auto TestEstimate3::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 3: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec3Smoke);
    PrintLn("  3 smoke passed: free unary and binary +,- compilation smoke");
    sec3SmokeCrossType();
    PrintLn("  3 smoke cross-type passed: cross +,- compilation smoke");

    PrintLn("--- Section 3: Arithmetic Free Functions (+ , -, unary -) ---");
    RunOverAllDims<AllStaticDims>(sec3FreeAddSub);
    PrintLn("  3 passed: free functions +, -, unary - (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestEstimate3 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
