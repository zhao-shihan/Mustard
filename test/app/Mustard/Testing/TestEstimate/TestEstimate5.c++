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

TestEstimate5::TestEstimate5() :
    Subprogram{"TestEstimate5", "Test Mustard::Math::Estimate (Section 5: operator* and operator/)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Section 5 Smoke: operator* and operator/
// =========================================================================

constexpr auto sec5Smoke{[]<int K, CovarianceOption C>() {
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

    // ---- operator* ----
    vec = (est * e3).Value();
    vec = (newEst() * e3).Value();
    vec = (est * newEst()).Value();
    vec = (newEst() * newEst()).Value();
    if constexpr (K != 1) {
        vec = (est * x).Value();
        vec = (x * est).Value();
    }
    vec = (est * 2.0).Value();
    vec = (2.0 * est).Value();

    // ---- operator/ ----
    vec = (est / e3).Value();
    vec = (newEst() / e3).Value();
    vec = (est / newEst()).Value();
    vec = (newEst() / newEst()).Value();
    if constexpr (K != 1) {
        vec = (est / x).Value();
        vec = (x / est).Value();
    }
    vec = (est / 2.0).Value();
    vec = (2.0 / est).Value();
}};

// =========================================================================
// Section 5 Smoke CrossType: operator* and operator/ cross-type
// =========================================================================

constexpr auto sec5SmokeCrossType{[] {
    [[maybe_unused]] double scalar{};
    constexpr auto full{CovarianceOption::Full};
    constexpr auto diag{CovarianceOption::Diagonal};
    constexpr int dyn{Eigen::Dynamic};

    // A: Same K, different C (K=3, Full <-> Diag)
    {
        const auto x3{MakeTestValue<3, full>(3)};
        const auto cov3f{MakeTestCov<3, full>(3)};
        const auto cov3d{MakeTestCov<3, diag>(3)};
        Estimate<3, full> ef3{x3, cov3f};
        Estimate<3, diag> ed3{x3, cov3d};
        scalar = (ef3 * ed3).Value(0);
        scalar = (ed3 * ef3).Value(0);
        scalar = (ef3 / ed3).Value(0);
        scalar = (ed3 / ef3).Value(0);
    }

    // B: Static <-> Dynamic, same C
    {
        const auto x3{MakeTestValue<3, full>(3)};
        const auto cov3f{MakeTestCov<3, full>(3)};
        const auto cov3d{MakeTestCov<3, diag>(3)};
        const auto covDynF3{MakeTestCov<dyn, full>(3)};
        const auto covDynD3{MakeTestCov<dyn, diag>(3)};
        const auto xDyn3{MakeTestValue<dyn, full>(3)};
        Estimate<3, full> e3f{x3, cov3f};
        Estimate<dyn, full> edf3{xDyn3, covDynF3};
        scalar = (e3f * edf3).Value(0);
        scalar = (edf3 * e3f).Value(0);
        scalar = (e3f / edf3).Value(0);
        scalar = (edf3 / e3f).Value(0);
        Estimate<3, diag> e3d{MakeTestValue<3, diag>(3), cov3d};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};
        scalar = (e3d * edd3).Value(0);
        scalar = (edd3 * e3d).Value(0);
        scalar = (e3d / edd3).Value(0);
        scalar = (edd3 / e3d).Value(0);
    }

    // C: Dynamic <-> Dynamic
    {
        const auto xDyn3{MakeTestValue<dyn, full>(3)};
        const auto covDynF3{MakeTestCov<dyn, full>(3)};
        const auto covDynD3{MakeTestCov<dyn, diag>(3)};
        Estimate<dyn, full> edf3{xDyn3, covDynF3};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};
        scalar = (edf3 * edd3).Value(0);
        scalar = (edd3 * edf3).Value(0);
        scalar = (edf3 / edd3).Value(0);
        scalar = (edd3 / edf3).Value(0);
    }

    // D: Static <-> Dynamic, different C
    {
        const auto x3{MakeTestValue<3, full>(3)};
        const auto cov3f{MakeTestCov<3, full>(3)};
        const auto cov3d{MakeTestCov<3, diag>(3)};
        const auto xDyn3{MakeTestValue<dyn, full>(3)};
        const auto covDynF3{MakeTestCov<dyn, full>(3)};
        const auto covDynD3{MakeTestCov<dyn, diag>(3)};
        Estimate<3, full> e3f{x3, cov3f};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};
        scalar = (e3f * edd3).Value(0);
        scalar = (edd3 * e3f).Value(0);
        scalar = (e3f / edd3).Value(0);
        scalar = (edd3 / e3f).Value(0);
        Estimate<3, diag> e3d{MakeTestValue<3, diag>(3), cov3d};
        Estimate<dyn, full> edf3{xDyn3, covDynF3};
        scalar = (e3d * edf3).Value(0);
        scalar = (edf3 * e3d).Value(0);
        scalar = (e3d / edf3).Value(0);
        scalar = (edf3 / e3d).Value(0);
    }
}};

// =========================================================================
// Section 5 Numerical: operator* and operator/
// =========================================================================

constexpr auto sec5MulDiv{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    const auto v{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    auto e1{MakeEstimate<K, C>(v, cov)};
    auto e2{MakeEstimate<K, C>(v, cov)};

    // 5a: e1 * e2 free function
    {
        const auto e3{e1 * e2};
        if constexpr (K == 1) {
            CheckClose(e3.Value(), 1.0, "5a: Value() e1*e2");
        } else {
            CheckClose(e3.Value(0), 1.0, "5a: Value(0) e1*e2");
            if constexpr (dim >= 2) {
                CheckClose(e3.Value(1), 4.0, "5a: Value(1) e1*e2");
            }
        }
        // Cov = diag(y)*Cov1*diag(y) + diag(x)*Cov2*diag(x)
        if constexpr (K != 1) {
            CheckBinaryCov(e3, cov, cov, "5a: e1*e2 cov", [&](int i) { return v(i); }, [&](int i) { return v(i); });
        } else {
            CheckClose(e3.Variance(), 0.8, "5a: Var() e1*e2");
        }
    }

    // 5b: e1 / e2 free function
    {
        const auto e4{e1 / e2};
        if constexpr (K == 1) {
            CheckClose(e4.Value(), 1.0, "5b: Value() e1/e2");
        } else {
            CheckClose(e4.Value(0), 1.0, "5b: Value(0) e1/e2");
        }
        // Cov = diag(1/y)*Cov1*diag(1/y) + diag(x/y^2)*Cov2*diag(x/y^2)
        if constexpr (K != 1) {
            CheckBinaryCov(e4, cov, cov, "5b: e1/e2 cov", [&](int i) { return 1.0 / v(i); }, [&](int i) { return 1.0 / v(i); });
        } else {
            CheckClose(e4.Variance(), 0.8, "5b: Var() e1/e2");
        }
    }

    // 5c: e * vec, e * scalar, scalar * e
    if constexpr (K != 1) {
        const auto e5{e1 * v};
        CheckClose(e5.Value(0), 1.0, "5c: Value(0) e*vec");
    }
    {
        const auto e6{e1 * 3.0};
        if constexpr (K == 1) {
            CheckClose(e6.Value(), 3.0, "5c: Value() e*scalar");
        } else {
            CheckClose(e6.Value(0), 3.0, "5c: Value(0) e*scalar");
        }
        const auto e7{3.0 * e1};
        if constexpr (K == 1) {
            CheckClose(e7.Value(), 3.0, "5c: Value() scalar*e");
        } else {
            CheckClose(e7.Value(0), 3.0, "5c: Value(0) scalar*e");
        }
    }

    // 5d: e / scalar, vec / e
    {
        const auto e8{e1 / 2.0};
        if constexpr (K == 1) {
            CheckClose(e8.Value(), 0.5, "5d: Value() e/scalar");
        } else {
            CheckClose(e8.Value(0), 0.5, "5d: Value(0) e/scalar");
        }
    }
    if constexpr (K != 1) {
        const auto e9{v / e1};
        CheckClose(e9.Value(0), 1.0, "5d: Value(0) vec/e");
    }
}};

// =========================================================================
// Section 5 Self: *= and /= (aliasing)
// =========================================================================

constexpr auto sec5SelfMulDiv{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};

    const auto v{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    // e *= e : J_x = diag(v), J_y = diag(v) -> Cov = 2*diag(v)*Cov*diag(v)
    {
        auto e{MakeEstimate<K, C>(v, cov)};
        e *= e;
        if constexpr (K == 1) {
            CheckClose(e.Value(), 1.0, "5s: Value() self*=");
            CheckClose(e.Variance(), 0.8, "5s: Var() self*=");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), v(i) * v(i), fmt::format("5s: Value({}) self*=", i));
            }
            CheckBinaryCov(e, cov, cov, "5s: self*= cov", [&](int i) { return v(i); }, [&](int i) { return v(i); });
        }
    }

    // e /= e : J_x = 1/v, J_y = x/y^2 = 1/v -> Cov = 2*diag(1/v)*Cov*diag(1/v)
    {
        auto e{MakeEstimate<K, C>(v, cov)};
        e /= e;
        if constexpr (K == 1) {
            CheckClose(e.Value(), 1.0, "5s: Value() self/=");
            CheckClose(e.Variance(), 0.8, "5s: Var() self/=");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Value(i), 1.0, fmt::format("5s: Value({}) self/=", i));
            }
            CheckBinaryCov(e, cov, cov, "5s: self/= cov", [&](int i) { return 1.0 / v(i); }, [&](int i) { return 1.0 / v(i); });
        }
    }
}};

} // namespace TestEstimateSection

auto TestEstimate5::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 5: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec5Smoke);
    PrintLn("  5 smoke passed: operator* and operator/ compilation smoke");
    sec5SmokeCrossType();
    PrintLn("  5 smoke cross-type passed: operator* and operator/ cross-type compilation smoke");

    PrintLn("--- Section 5: operator* and operator/ Numerical ---");
    RunOverAllDims<AllStaticDims>(sec5MulDiv);
    PrintLn("  5 passed: operator* and operator/ (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 5: Self-Operations (aliasing) ---");
    RunOverAllDims<AllStaticDims>(sec5SelfMulDiv);
    PrintLn("  5 self passed: self *= and /= (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestEstimate5 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
