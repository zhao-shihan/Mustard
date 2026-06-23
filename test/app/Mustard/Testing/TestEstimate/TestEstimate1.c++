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

TestEstimate1::TestEstimate1() :
    Subprogram{"TestEstimate1", "Test Mustard::Math::Estimate (Section 1: Assignment)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Section 1 Smoke: Assignment Operators (from sec0Smoke)
// =========================================================================

constexpr auto sec1Smoke{[]<int K, CovarianceOption C>() {
    using Est = Estimate<K, C>;
    constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
    constexpr bool isFull{C == CovarianceOption::Full};
    constexpr CovarianceOption oppositeC{isFull ? CovarianceOption::Diagonal : CovarianceOption::Full};
    using OppositeEst = Estimate<K, oppositeC>;

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    const auto xOpp{MakeTestValue<K, oppositeC>(dim)};
    const auto covOpp{MakeTestCov<K, oppositeC>(dim)};

    [[maybe_unused]] double scalar{};

    const auto newEst{[&] {
        if constexpr (K == 1) {
            return Est{1.0, 0.5};
        } else {
            return Est{x, cov};
        }
    }};
    const auto newOpp{[&] {
        if constexpr (K == 1) {
            return OppositeEst{0.5, 0.25};
        } else {
            return OppositeEst{xOpp, covOpp};
        }
    }};

    Est e3 = newEst();

    // ---- assignment operators ----
    auto est{newEst()};
    {
        est = e3;
        {
            auto e3m{newEst()};
            est = std::move(e3m);
        }
        auto& estRef{est};
        est = estRef;
        est = std::move(estRef);
        OppositeEst eOpp2{newOpp()};
        est = eOpp2;
        est = newEst();
    }
}};

// =========================================================================
// Section 1 Smoke CrossType: Cross-Assignment (from sec0Smoke)
// Covers Full↔Diagonal, Static↔Dynamic, Dynamic↔Dynamic, Combined
// =========================================================================

constexpr auto sec1SmokeCrossType{[] {
    [[maybe_unused]] double scalar{};
    constexpr auto full{CovarianceOption::Full};
    constexpr auto diag{CovarianceOption::Diagonal};
    constexpr int dyn{Eigen::Dynamic};

    // A: Same K, different CovarianceOption (K=3, Full ↔ Diagonal)
    {
        const auto x3{MakeTestValue<3, full>(3)};
        const auto cov3f{MakeTestCov<3, full>(3)};
        const auto cov3d{MakeTestCov<3, diag>(3)};
        Estimate<3, full> ef3{x3, cov3f};
        Estimate<3, diag> ed3{x3, cov3d};

        Estimate<3, full> efAssign{x3, cov3f};
        efAssign = ed3;
        scalar = efAssign.Value(0);
        Estimate<3, diag> edAssign{x3, cov3d};
        edAssign = ef3;
        scalar = edAssign.Value(0);
    }

    // B: Static ↔ Dynamic, same CovarianceOption
    {
        const auto x3{MakeTestValue<3, full>(3)};
        const auto cov3f{MakeTestCov<3, full>(3)};
        const auto cov3d{MakeTestCov<3, diag>(3)};
        const auto covDynF3{MakeTestCov<dyn, full>(3)};
        const auto covDynD3{MakeTestCov<dyn, diag>(3)};
        const auto xDyn3{MakeTestValue<dyn, full>(3)};

        // Static Full ↔ Dynamic Full
        Estimate<3, full> e3f{x3, cov3f};
        Estimate<dyn, full> edf3{xDyn3, covDynF3};

        Estimate<dyn, full> edfAssign{MakeTestValue<dyn, full>(3), MakeTestCov<dyn, full>(3)};
        edfAssign = e3f;
        scalar = edfAssign.Value(0);
        Estimate<3, full> e3fAssign{x3, cov3f};
        e3fAssign = edf3;
        scalar = e3fAssign.Value(0);

        // Static Diag ↔ Dynamic Diag
        Estimate<3, diag> e3d{MakeTestValue<3, diag>(3), cov3d};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};

        Estimate<dyn, diag> eddAssign{MakeTestValue<dyn, diag>(3), MakeTestCov<dyn, diag>(3)};
        eddAssign = e3d;
        scalar = eddAssign.Variance(0);
        Estimate<3, diag> e3dAssign{MakeTestValue<3, diag>(3), cov3d};
        e3dAssign = edd3;
        scalar = e3dAssign.Variance(0);
    }

    // C: Dynamic ↔ Dynamic, same dimension
    {
        const auto xDyn3{MakeTestValue<dyn, full>(3)};
        const auto covDynF3{MakeTestCov<dyn, full>(3)};
        const auto covDynD3{MakeTestCov<dyn, diag>(3)};

        Estimate<dyn, full> edf3{xDyn3, covDynF3};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};

        Estimate<dyn, full> edfAssign{MakeTestValue<dyn, full>(3), MakeTestCov<dyn, full>(3)};
        edfAssign = edd3;
        scalar = edfAssign.Value(0);
        Estimate<dyn, diag> eddAssign{MakeTestValue<dyn, diag>(3), MakeTestCov<dyn, diag>(3)};
        eddAssign = edf3;
        scalar = eddAssign.Variance(0);
    }

    // D: Static ↔ Dynamic, different CovarianceOption
    {
        const auto x3{MakeTestValue<3, full>(3)};
        const auto cov3f{MakeTestCov<3, full>(3)};
        const auto cov3d{MakeTestCov<3, diag>(3)};
        const auto xDyn3{MakeTestValue<dyn, full>(3)};
        const auto covDynF3{MakeTestCov<dyn, full>(3)};
        const auto covDynD3{MakeTestCov<dyn, diag>(3)};

        // Static Full ↔ Dynamic Diag
        Estimate<3, full> e3f{x3, cov3f};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};

        Estimate<dyn, diag> eddAssign{MakeTestValue<dyn, diag>(3), MakeTestCov<dyn, diag>(3)};
        eddAssign = e3f;
        scalar = eddAssign.Variance(0);
        Estimate<3, full> e3fAssign{x3, cov3f};
        e3fAssign = edd3;
        scalar = e3fAssign.Value(0);

        // Static Diag ↔ Dynamic Full
        Estimate<3, diag> e3d{MakeTestValue<3, diag>(3), cov3d};
        Estimate<dyn, full> edf3{xDyn3, covDynF3};

        Estimate<dyn, full> edfAssign{MakeTestValue<dyn, full>(3), MakeTestCov<dyn, full>(3)};
        edfAssign = e3d;
        scalar = edfAssign.Value(0);
        Estimate<3, diag> e3dAssign{MakeTestValue<3, diag>(3), cov3d};
        e3dAssign = edf3;
        scalar = e3dAssign.Variance(0);
    }
}};

// =========================================================================
// Section 1: Assignment Operators
// =========================================================================

constexpr auto sec1Assignment{[]<int K, CovarianceOption C>() {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    constexpr bool isFull{C == CovarianceOption::Full};

    const auto v{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};

    auto e1{MakeEstimate<K, C>(v, cov)};
    auto e2{MakeEstimate<K, C>(v, cov)};

    // Move assignment
    auto e3{MakeEstimate<K, C>(v, cov)};
    e3 = std::move(e2);
    if constexpr (K == 1) {
        CheckClose(e3.Value(), 1.0, "1a: MoveAssign Value()");
        CheckClose(e3.Variance(), 0.4, "1a: MoveAssign Variance()");
    } else {
        CheckClose(e3.Value(0), 1.0, "1a: MoveAssign Value(0)");
        if constexpr (dim >= 2) {
            CheckClose(e3.Variance(1), 0.3, "1a: MoveAssign Variance(1)");
        }
    }

    // Copy assignment
    auto e4{MakeEstimate<K, C>(v, cov)};
    e4 = e1;
    if constexpr (K == 1) {
        CheckClose(e4.Value(), 1.0, "1b: CopyAssign Value()");
    } else {
        CheckClose(e4.Value(0), 1.0, "1b: CopyAssign Value(0)");
    }

    // Self-assignment
    auto& e1Ref{e1};
    e1 = e1Ref;
    e1 = std::move(e1Ref);
    if constexpr (K == 1) {
        CheckClose(e1.Value(), 1.0, "1c: SelfAssign Value()");
        CheckClose(e1.Variance(), 0.4, "1c: SelfAssign Variance()");
    } else {
        CheckClose(e1.Value(0), 1.0, "1c: SelfAssign Value(0)");
        if constexpr (isFull and dim >= 2) {
            CheckClose(e1.Covariance(0, 1), 0.1, "1c: SelfAssign Cov(0,1)");
        }
    }

    // Cross-CovarianceOption assignment
    if constexpr (isFull) {
        const auto diagCov{MakeTestCov<K, CovarianceOption::Diagonal>(dim)};
        auto eDiag{MakeEstimate<K, CovarianceOption::Diagonal>(v, diagCov)};
        auto eFull{MakeEstimate<K, C>(v, cov)};
        eFull = eDiag;
        if constexpr (K == 1) {
            CheckClose(eFull.Value(), 1.0, "1d: Full=Diag Value()");
            CheckClose(eFull.Variance(), 0.4, "1d: Full=Diag Variance()");
        } else {
            CheckClose(eFull.Value(0), 1.0, "1d: Full=Diag Value(0)");
            if constexpr (dim >= 2) {
                CheckClose(eFull.Variance(1), 0.3, "1d: Full=Diag Variance(1)");
                CheckClose(eFull.Covariance(0, 1), 0.0, "1d: Full=Diag Cov(0,1)=0");
            }
        }

        auto eDiag2{MakeEstimate<K, CovarianceOption::Diagonal>(v, diagCov)};
        eDiag2 = MakeEstimate<K, C>(v, cov);
        if constexpr (K == 1) {
            CheckClose(eDiag2.Value(), 1.0, "1d: Diag=Full Value()");
            CheckClose(eDiag2.Variance(), 0.4, "1d: Diag=Full Variance()");
        } else {
            CheckClose(eDiag2.Value(0), 1.0, "1d: Diag=Full Value(0)");
            CheckClose(eDiag2.Variance(0), 0.4, "1d: Diag=Full Variance(0)");
            if constexpr (dim >= 2) {
                CheckClose(eDiag2.Covariance(0, 1), 0.0, "1d: Diag=Full Cov(0,1)=0");
            }
        }
    } else {
        const auto fullCov{MakeTestCov<K, CovarianceOption::Full>(dim)};
        auto eFull{MakeEstimate<K, CovarianceOption::Full>(v, fullCov)};
        auto eDiag{MakeEstimate<K, C>(v, cov)};
        eDiag = eFull;
        if constexpr (K == 1) {
            CheckClose(eDiag.Value(), 1.0, "1d: Diag=Full Value()");
            CheckClose(eDiag.Variance(), 0.4, "1d: Diag=Full Variance()");
        } else {
            CheckClose(eDiag.Value(0), 1.0, "1d: Diag=Full Value(0)");
            CheckClose(eDiag.Variance(0), 0.4, "1d: Diag=Full Variance(0)");
            if constexpr (dim >= 2) {
                CheckClose(eDiag.Covariance(0, 1), 0.0, "1d: Diag=Full Cov(0,1)=0");
            }
        }

        auto eFull2{MakeEstimate<K, CovarianceOption::Full>(v, fullCov)};
        eFull2 = MakeEstimate<K, C>(v, cov);
        if constexpr (K == 1) {
            CheckClose(eFull2.Value(), 1.0, "1d: Full=Diag Value()");
            CheckClose(eFull2.Variance(), 0.4, "1d: Full=Diag Variance()");
        } else {
            CheckClose(eFull2.Value(0), 1.0, "1d: Full=Diag Value(0)");
            if constexpr (dim >= 2) {
                CheckClose(eFull2.Variance(1), 0.3, "1d: Full=Diag Variance(1)");
                CheckClose(eFull2.Covariance(0, 1), 0.0, "1d: Full=Diag Cov(0,1)=0");
            }
        }
    }
}};

// =========================================================================
// Section 1: Cross-Dimension Assignment
// =========================================================================

constexpr auto sec1CrossDimAssignment{[] {
    Eigen::VectorXd xDyn{2};
    xDyn << 7.0, 8.0;
    Eigen::MatrixXd covDyn{2, 2};
    covDyn << 4.9, 0.0, 0.0, 6.4;
    Estimate<Eigen::Dynamic, CovarianceOption::Full> eDyn{xDyn, covDyn};

    Eigen::Vector2d xStat{1.0, 2.0};
    Eigen::Matrix2d covStat;
    covStat << 0.1, 0.0, 0.0, 0.4;
    Estimate<2, CovarianceOption::Full> eStat{xStat, covStat};

    eDyn = eStat;
    CheckClose(eDyn.Value(0), 1.0, "1e: Dyn=Stat Value(0)");
    CheckClose(eDyn.Covariance(1, 1), 0.4, "1e: Dyn=Stat Cov(1,1)");
}};

} // namespace TestEstimateSection

auto TestEstimate1::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 1: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec1Smoke);
    PrintLn("  1 smoke passed: assignment operators compilation smoke");

    sec1SmokeCrossType();
    PrintLn("  1 smoke cross-type passed: cross-type assignment compilation smoke");

    PrintLn("--- Section 1: Assignment Operators ---");
    RunOverAllDims<AllStaticDims>(sec1Assignment);
    PrintLn("  1 passed: assignment operators (K=1,2,3,5,10 Full/Diag + dynamic)");

    sec1CrossDimAssignment();
    PrintLn("  1e passed: cross-dimension assignment");

    PrintLn("All TestEstimate1 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
