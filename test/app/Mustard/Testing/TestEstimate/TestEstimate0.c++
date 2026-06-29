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

TestEstimate0::TestEstimate0() :
    Subprogram{"TestEstimate0", "Test Mustard::Math::Estimate (Section 0: Construction)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Section 0 Smoke: Construction and Basic Accessors (from sec0Smoke)
// =========================================================================

constexpr auto sec0Smoke{[]<int K, CovarianceOption C>() {
    using Est = Estimate<K, C>;
    constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr bool isFull{C == CovarianceOption::Full};
    constexpr CovarianceOption oppositeC{isFull ? CovarianceOption::Diagonal : CovarianceOption::Full};
    using OppositeEst = Estimate<K, oppositeC>;

    const auto x{MakeTestValue<K, C>(dim)};
    const auto cov{MakeTestCov<K, C>(dim)};
    const auto xOpp{MakeTestValue<K, oppositeC>(dim)};
    const auto covOpp{MakeTestCov<K, oppositeC>(dim)};

    // Three reusable result variables: scalar, vector, matrix
    [[maybe_unused]] double scalar{};
    [[maybe_unused]] typename Est::ValueType vec{};
    [[maybe_unused]] typename Est::CovarianceType mat{};

    // Fresh-Estimate helpers (K=1 uses scalar ctor; K!=1 uses Eigen ctor)
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

    // ---- constructors ----
    {
        if constexpr (not isDynamic) {
            Est e0;
            vec = e0.Value();
            typename Est::PODType pod{};
            Est e5{pod};
            vec = e5.Value();
        }
        if constexpr (isDynamic) {
            Est e0d{dim};
            vec = e0d.Value();
        }
        Est e1 = newEst();
        if constexpr (K == 1) {
            Estimate<1, C> e1s{1.0, 0.5};
            scalar = e1s.Value();
        }
        Est e2{std::move(e1)};
        Est e3{e2};
        OppositeEst eOpp = newOpp();
        Est e4{eOpp};
    }

    Est e3 = newEst();

    // ---- per-component accessors (K!=1 only; hidden by no-arg overloads for K=1) ----
    if constexpr (K != 1) {
        scalar = e3.Dimension();
        scalar = e3.Value(0);
        scalar = e3.Covariance(0, 0);
        scalar = e3.Variance(0);
        scalar = e3.StdDev(0);
        scalar = e3.Uncertainty(0);
        scalar = e3.RelativeUncertainty(0);
        scalar = e3.Correlation(0, 0);
        if constexpr (dim >= 2) {
            scalar = e3.Correlation(0, 1);
        }
    }

    // ---- full vector/matrix accessors ----
    {
        scalar = e3.Dimension();
        vec = e3.Value();
        if constexpr (K != 1) {
            mat = e3.Covariance();
            mat = e3.Correlation();
        }
        vec = e3.Variance();
        vec = e3.StdDev();
        vec = e3.Uncertainty();
        vec = e3.RelativeUncertainty();
    }
}};

// =========================================================================
// Section 0 Smoke CrossType A: Same K, different CovarianceOption (K=3, Full ↔ Diagonal)
// =========================================================================

constexpr auto sec0SmokeCrossTypeA{[] {
    [[maybe_unused]] double scalar{};
    constexpr auto full{CovarianceOption::Full};
    constexpr auto diag{CovarianceOption::Diagonal};

    {
        const auto x3{MakeTestValue<3, full>(3)};
        const auto cov3f{MakeTestCov<3, full>(3)};
        const auto cov3d{MakeTestCov<3, diag>(3)};
        Estimate<3, full> ef3{x3, cov3f};
        Estimate<3, diag> ed3{x3, cov3d};

        // cross construction
        [[maybe_unused]] Estimate<3, full> efFromDiag{ed3};
        [[maybe_unused]] Estimate<3, diag> edFromFull{ef3};
    }
}};

// =========================================================================
// Section 0 Smoke CrossType B: Static ↔ Dynamic, same dimension, same CovarianceOption
// =========================================================================

constexpr auto sec0SmokeCrossTypeB{[] {
    [[maybe_unused]] double scalar{};
    constexpr auto full{CovarianceOption::Full};
    constexpr auto diag{CovarianceOption::Diagonal};
    constexpr int dyn{Eigen::Dynamic};

    {
        const auto x3{MakeTestValue<3, full>(3)};
        const auto cov3f{MakeTestCov<3, full>(3)};
        const auto cov3d{MakeTestCov<3, diag>(3)};
        const auto covDynF3{MakeTestCov<dyn, full>(3)};
        const auto covDynD3{MakeTestCov<dyn, diag>(3)};
        const auto xDyn3{MakeTestValue<dyn, full>(3)};

        // --- Static Full ↔ Dynamic Full ---
        Estimate<3, full> e3f{x3, cov3f};
        Estimate<dyn, full> edf3{xDyn3, covDynF3};

        // cross construction
        [[maybe_unused]] Estimate<3, full> e3fFromDyn{edf3};
        [[maybe_unused]] Estimate<dyn, full> edfFromStat{e3f};

        // --- Static Diag ↔ Dynamic Diag ---
        Estimate<3, diag> e3d{MakeTestValue<3, diag>(3), cov3d};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};

        // cross construction
        [[maybe_unused]] Estimate<3, diag> e3dFromDyn{edd3};
        [[maybe_unused]] Estimate<dyn, diag> eddFromStat{e3d};
    }
}};

// =========================================================================
// Section 0 Smoke CrossType C: Dynamic ↔ Dynamic, same dimension
// =========================================================================

constexpr auto sec0SmokeCrossTypeC{[] {
    [[maybe_unused]] double scalar{};
    constexpr auto full{CovarianceOption::Full};
    constexpr auto diag{CovarianceOption::Diagonal};
    constexpr int dyn{Eigen::Dynamic};

    {
        const auto xDyn3{MakeTestValue<dyn, full>(3)};
        const auto covDynF3{MakeTestCov<dyn, full>(3)};
        const auto covDynD3{MakeTestCov<dyn, diag>(3)};

        // --- Dyn Full ↔ Dyn Diag ---
        Estimate<dyn, full> edf3{xDyn3, covDynF3};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};

        // cross construction
        [[maybe_unused]] Estimate<dyn, full> edfFromDiag{edd3};
        [[maybe_unused]] Estimate<dyn, diag> eddFromFull{edf3};
    }
}};

// =========================================================================
// Section 0 Smoke CrossType D: Static ↔ Dynamic, different CovarianceOption
// =========================================================================

constexpr auto sec0SmokeCrossTypeD{[] {
    [[maybe_unused]] double scalar{};
    constexpr auto full{CovarianceOption::Full};
    constexpr auto diag{CovarianceOption::Diagonal};
    constexpr int dyn{Eigen::Dynamic};

    {
        const auto x3{MakeTestValue<3, full>(3)};
        const auto cov3f{MakeTestCov<3, full>(3)};
        const auto cov3d{MakeTestCov<3, diag>(3)};
        const auto xDyn3{MakeTestValue<dyn, full>(3)};
        const auto covDynF3{MakeTestCov<dyn, full>(3)};
        const auto covDynD3{MakeTestCov<dyn, diag>(3)};

        // --- Static Full ↔ Dynamic Diag ---
        Estimate<3, full> e3f{x3, cov3f};
        Estimate<dyn, diag> edd3{MakeTestValue<dyn, diag>(3), covDynD3};

        // cross construction
        [[maybe_unused]] Estimate<3, full> e3fFromDD{edd3};
        [[maybe_unused]] Estimate<dyn, diag> eddFromSF{e3f};

        // --- Static Diag ↔ Dynamic Full ---
        Estimate<3, diag> e3d{MakeTestValue<3, diag>(3), cov3d};
        Estimate<dyn, full> edf3{xDyn3, covDynF3};

        // cross construction
        [[maybe_unused]] Estimate<3, diag> e3dFromDF{edf3};
        [[maybe_unused]] Estimate<dyn, full> edfFromSD{e3d};
    }
}};

// =========================================================================
// Section 0: Construction and Basic Accessors
// =========================================================================

constexpr auto sec0Construction{[]<int K, CovarianceOption C>() {
    using Est = Estimate<K, C>;
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr bool isFull{C == CovarianceOption::Full};

    // Default construction (static only)
    if constexpr (not isDynamic) {
        Est e;
        CheckEq(e.Dimension(), K, "0: Dimension");
        if constexpr (K == 1) {
            CheckClose(e.Value(), 0.0, "0: Value()");
            CheckClose(e.Variance(), 0.0, "0: Variance()");
            CheckClose(e.StdDev(), 0.0, "0: StdDev()");
            CheckClose(e.Uncertainty(), 0.0, "0: Uncertainty()");
        } else {
            for (auto i{0}; i < K; ++i) {
                CheckClose(e.Value(i), 0.0, fmt::format("0: Value({})", i));
                CheckClose(e.Variance(i), 0.0, fmt::format("0: Variance({})", i));
                CheckClose(e.StdDev(i), 0.0, fmt::format("0: StdDev({})", i));
                CheckClose(e.Uncertainty(i), 0.0, fmt::format("0: Uncertainty({})", i));
            }
        }
    } else {
        {
            Est e{2};
            CheckEq(e.Dimension(), 2, "0: Dimension");
            CheckClose(e.Value(0), 0.0, "0: Value(0)");
            CheckClose(e.Value(1), 0.0, "0: Value(1)");
        }
        {
            auto threwZero{false};
            try {
                Est e{0};
            } catch (const std::invalid_argument&) {
                threwZero = true;
            }
            if (not threwZero) {
                Throw<std::runtime_error>("0: dim=0 should throw");
            }

            auto threwNeg{false};
            try {
                Est e{-1};
            } catch (const std::invalid_argument&) {
                threwNeg = true;
            }
            if (not threwNeg) {
                Throw<std::runtime_error>("0: dim=-1 should throw");
            }
        }
    }

    // Value+cov constructor
    {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        auto e{MakeEstimate<K, C>(x, cov)};

        if constexpr (K == 1) {
            CheckClose(e.Value(), 1.0, "0: Value()");
            CheckClose(e.Variance(), 0.4, "0: Variance()");
            CheckClose(e.StdDev(), std::sqrt(0.4), "0: StdDev()");
            CheckClose(e.Uncertainty(), std::sqrt(0.4), "0: Uncertainty()");
            CheckClose(e.RelativeUncertainty(), std::sqrt(0.4), "0: RelUnc()");
        } else {
            CheckClose(e.Value(0), 1.0, "0: Value(0)");
            if constexpr (dim >= 2) {
                CheckClose(e.Value(1), 2.0, "0: Value(1)");
            }
            if constexpr (isFull) {
                CheckClose(e.Covariance(0, 0), 0.4, "0: Cov(0,0)");
                if constexpr (dim >= 2) {
                    CheckClose(e.Covariance(1, 1), 0.3, "0: Cov(1,1)");
                    CheckClose(e.Covariance(0, 1), 0.1, "0: Cov(0,1)");
                    CheckClose(e.Covariance(1, 0), 0.1, "0: Cov(1,0)");
                }
            }
            CheckClose(e.Variance(0), 0.4, "0: Variance(0)");
            if constexpr (dim >= 2) {
                CheckClose(e.Variance(1), 0.3, "0: Variance(1)");
                CheckClose(e.StdDev(1), std::sqrt(0.3), "0: StdDev(1)");
                CheckClose(e.RelativeUncertainty(1), std::sqrt(0.3) / 2.0, "0: RelUnc(1)");
            }
            CheckClose(e.StdDev(0), std::sqrt(0.4), "0: StdDev(0)");
            CheckClose(e.Uncertainty(0), std::sqrt(0.4), "0: Uncertainty(0)");
            CheckClose(e.RelativeUncertainty(0), std::sqrt(0.4), "0: RelUnc(0)");
        }

        if constexpr (K == 1) {
            CheckClose(e.Value(), x(0), "0: Value()");
            CheckClose(e.Variance(), cov.diagonal()(0), "0: Variance()");
            CheckClose(e.StdDev(), std::sqrt(cov.diagonal()(0)), "0: StdDev()");
            CheckClose(e.Uncertainty(), std::sqrt(cov.diagonal()(0)), "0: Uncertainty()");
        } else {
            CheckClose(e.Value(), x, "0: Value()");
            for (auto i{0}; i < dim; ++i) {
                CheckClose(e.Variance()(i), cov.diagonal()(i), fmt::format("0: Variance()({})", i));
                CheckClose(e.StdDev()(i), std::sqrt(cov.diagonal()(i)), fmt::format("0: StdDev()({})", i));
                CheckClose(e.Uncertainty()(i), std::sqrt(cov.diagonal()(i)), fmt::format("0: Uncertainty()({})", i));
            }
        }
        if constexpr (isFull and K != 1) {
            CheckClose(e.Covariance(), cov, "0: Covariance()");
        }

        // Correlation checks
        if constexpr (isFull and K != 1) {
            CheckClose(e.Correlation(0, 0), 1.0, "0: Corr(0,0)=1");
            if constexpr (dim >= 2) {
                CheckClose(e.Correlation(1, 1), 1.0, "0: Corr(1,1)=1");
                CheckClose(e.Correlation(0, 1), 0.1 / std::sqrt(0.12), "0: Corr(0,1)");
                CheckClose(e.Correlation(1, 0), 0.1 / std::sqrt(0.12), "0: Corr(1,0) sym");
            }
            const auto corrMat{e.Correlation()};
            CheckClose(corrMat(0, 0), 1.0, "0: Correlation()(0,0)");
            if constexpr (dim >= 2) {
                CheckClose(corrMat(0, 1), 0.1 / std::sqrt(0.12), "0: Correlation()(0,1)");
            }
        }
        if constexpr (not isFull and K != 1 and dim >= 2) {
            CheckClose(e.Correlation(0, 0), 1.0, "0: Corr(0,0)=1 diag");
            CheckClose(e.Correlation(0, 1), 0.0, "0: Corr(0,1)=0 diag");
        }

        Est e2{e};
        if constexpr (K == 1) {
            CheckClose(e2.Value(), e.Value(), "0: Copy Value()");
            CheckClose(e2.Variance(), e.Variance(), "0: Copy Variance()");
        } else {
            CheckClose(e2.Value(0), e.Value(0), "0: Copy Value(0)");
            if constexpr (isFull and dim >= 2) {
                CheckClose(e2.Covariance(0, 1), e.Covariance(0, 1), "0: Copy Cov(0,1)");
            }
        }

        Est e3{std::move(e2)};
        if constexpr (K == 1) {
            CheckClose(e3.Value(), 1.0, "0: Move Value()");
            CheckClose(e3.Variance(), 0.4, "0: Move Variance()");
        } else {
            CheckClose(e3.Value(0), 1.0, "0: Move Value(0)");
            if constexpr (isFull and dim >= 2) {
                CheckClose(e3.Covariance(0, 1), 0.1, "0: Move Cov(0,1)");
            }
        }
    }

    // Cross-CovarianceOption copy
    if constexpr (isFull) {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, CovarianceOption::Full>(dim)};
        auto eFull{MakeEstimate<K, CovarianceOption::Full>(x, cov)};
        Estimate<K, CovarianceOption::Diagonal> eDiag{eFull};
        if constexpr (K == 1) {
            CheckClose(eDiag.Value(), 1.0, "0: DiagFromFull Value()");
            CheckClose(eDiag.Variance(), 0.4, "0: DiagFromFull Variance()");
        } else {
            CheckClose(eDiag.Value(0), 1.0, "0: DiagFromFull Value(0)");
            CheckClose(eDiag.Variance(0), 0.4, "0: DiagFromFull Variance(0)");
            if constexpr (dim >= 2) {
                CheckClose(eDiag.Variance(1), 0.3, "0: DiagFromFull Variance(1)");
                CheckClose(eDiag.Covariance(0, 1), 0.0, "0: DiagFromFull Cov(0,1)=0");
            }
        }
    } else {
        const auto x{MakeTestValue<K, C>(dim)};
        const auto diagCov{MakeTestCov<K, CovarianceOption::Diagonal>(dim)};
        auto eDiag{MakeEstimate<K, CovarianceOption::Diagonal>(x, diagCov)};
        Estimate<K, CovarianceOption::Full> eFull{eDiag};
        if constexpr (K == 1) {
            CheckClose(eFull.Value(), 1.0, "0: FullFromDiag Value()");
            CheckClose(eFull.Variance(), 0.4, "0: FullFromDiag Variance()");
        } else {
            CheckClose(eFull.Value(0), 1.0, "0: FullFromDiag Value(0)");
            CheckClose(eFull.Variance(0), 0.4, "0: FullFromDiag Variance(0)");
            if constexpr (dim >= 2) {
                CheckClose(eFull.Covariance(0, 1), 0.0, "0: FullFromDiag Cov(0,1)=0");
            }
        }
    }

    // Dimension mismatch throws (dynamic only)
    if constexpr (isDynamic) {
        Est e1{2};
        auto threw{false};
        try {
            Est eOther{3};
            e1 += eOther;
        } catch (const std::invalid_argument&) { threw = true; }
        if (not threw) {
            Throw<std::runtime_error>("0: dimension mismatch should throw");
        }
    }
}};

// =========================================================================
// Section 0: Cross-Dimension Copy (Static <-> Dynamic)
// =========================================================================

constexpr auto sec0CrossDimCopy{[] {
    // Static from Dynamic
    Eigen::VectorXd xDyn{2};
    xDyn << 5.0, 6.0;
    Eigen::MatrixXd covDyn{2, 2};
    covDyn << 0.9, 0.2, 0.2, 0.4;
    Estimate<Eigen::Dynamic, CovarianceOption::Full> eDyn{xDyn, covDyn};
    Estimate<2, CovarianceOption::Full> eStat{eDyn};
    CheckEq(eStat.Dimension(), 2, "0j: Static from Dynamic Dimension");
    CheckClose(eStat.Value(0), 5.0, "0j: Static from Dynamic Value(0)");
    CheckClose(eStat.Covariance(0, 1), 0.2, "0j: Static from Dynamic Cov(0,1)");

    // Dynamic from Static
    Eigen::Vector2d xStat{7.0, 8.0};
    Eigen::Matrix2d covStat;
    covStat << 1.6, 0.3, 0.3, 2.5;
    Estimate<2, CovarianceOption::Full> eStat2{xStat, covStat};
    Estimate<Eigen::Dynamic, CovarianceOption::Full> eDyn2{eStat2};
    CheckEq(eDyn2.Dimension(), 2, "0j: Dynamic from Static Dimension");
    CheckClose(eDyn2.Value(0), 7.0, "0j: Dynamic from Static Value(0)");
    CheckClose(eDyn2.Covariance(0, 1), 0.3, "0j: Dynamic from Static Cov(0,1)");
}};

} // namespace TestEstimateSection

auto TestEstimate0::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 0: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec0Smoke);
    sec0SmokeCrossTypeA();
    sec0SmokeCrossTypeB();
    sec0SmokeCrossTypeC();
    sec0SmokeCrossTypeD();
    PrintLn("  0 smoke passed: compilation smoke (K=1,2,3,5,10 Full/Diag + dynamic + cross-type)");

    PrintLn("--- Section 0: Construction and Basic Accessors ---");
    RunOverAllDims<AllStaticDims>(sec0Construction);
    PrintLn("  0 passed: construction and basic accessors (K=1,2,3,5,10 Full/Diag + dynamic)");

    sec0CrossDimCopy();
    PrintLn("  0j passed: cross-dimension copy");

    PrintLn("All TestEstimate0 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
