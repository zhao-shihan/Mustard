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
#include "Mustard/Testing/TestStatistic/TestStatistic.h++"
#include "Mustard/Testing/TestStatistic/TestStatisticCommon.h++"

#include <cstdlib>

namespace Mustard::Testing {

TestStatistic10::TestStatistic10() :
    Subprogram{"TestStatistic10", "Test Mustard::Statistic (Section 10: Cross-Verification)."} {}

namespace TestStatisticSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestStatistic;

// =========================================================================
// Section 10 Smoke: Cross-Verification (from sec0SmokeCrossCov/CrossDim)
// =========================================================================

constexpr auto sec10Smoke{[] {
    // Cross-CovarianceOption: copy construct, assign, merge, combine
    {
        Statistic<1> sFull;
        sFull.Fill(1.0);
        Statistic<1, CovarianceOption::Diagonal> sDiag;
        sDiag.Fill(2.0);

        Statistic<1, CovarianceOption::Diagonal> sDiagFromFull{sFull};
        Statistic<1> sFullFromDiag{sDiag};
        sDiagFromFull = sFull;
        sFullFromDiag = sDiag;
        sFull += sDiag;
        sDiag += sFull;
        [[maybe_unused]] auto r1{sFull + sDiag};
        [[maybe_unused]] auto r2{sDiag + sFull};
    }

    // Vector cross
    {
        Statistic<2> sFull;
        sFull.Fill(Eigen::Vector2d{1.0, 2.0});
        Statistic<2, CovarianceOption::Diagonal> sDiag;
        sDiag.Fill(Eigen::Vector2d{3.0, 4.0});

        Statistic<2, CovarianceOption::Diagonal> sDiagFromFull{sFull};
        Statistic<2> sFullFromDiag{sDiag};
        sDiagFromFull = sFull;
        sFullFromDiag = sDiag;
        sFull += sDiag;
        sDiag += sFull;
        [[maybe_unused]] auto r1{sFull + sDiag};
        [[maybe_unused]] auto r2{sDiag + sFull};
    }

    // Cross-dimension: copy construct, assign, merge, combine
    {
        Statistic<Eigen::Dynamic> sDyn(3);
        sDyn.Fill(Eigen::Vector3d{1.0, 2.0, 3.0});
        Statistic<3> sStat{sDyn};
        sStat = sDyn;
        sStat += sDyn;
        [[maybe_unused]] auto r{sStat + sDyn};
    }
    {
        Statistic<3> sStat;
        sStat.Fill(Eigen::Vector3d{1.0, 2.0, 3.0});
        Statistic<Eigen::Dynamic> sDyn{sStat};
        sDyn = sStat;
        sDyn += sStat;
        [[maybe_unused]] auto r{sDyn + sStat};
    }

    // Rvalue cross-type operations
    {
        Statistic<2> sFull;
        sFull.Fill(Eigen::Vector2d{1.0, 2.0});
        Statistic<2, CovarianceOption::Diagonal> sDiag;
        sDiag.Fill(Eigen::Vector2d{3.0, 6.0});
        [[maybe_unused]] auto r1{sFull + Statistic<2, CovarianceOption::Diagonal>{sDiag}};
        [[maybe_unused]] auto r2{Statistic<2>{sFull} + sDiag};
    }

    {
        Statistic<3> sStat;
        sStat.Fill(Eigen::Vector3d{1.0, 2.0, 3.0});
        [[maybe_unused]] auto r{sStat + Statistic<Eigen::Dynamic>{sStat}};
    }
}};

// =========================================================================
// Section 10: Cross-CovarianceOption Copy Construction
// =========================================================================

constexpr auto sec10CrossCovCopy{[] {
    // Scalar
    {
        Statistic<1> sFull;
        sFull.Fill(2.0);
        sFull.Fill(4.0);
        sFull.Fill(4.0);
        sFull.Fill(4.0);
        sFull.Fill(5.0);
        sFull.Fill(5.0);
        sFull.Fill(7.0);
        sFull.Fill(9.0);

        // Diagonal from Full
        Statistic<1, CovarianceOption::Diagonal> sDiag{sFull};
        CheckClose(sDiag.Mean(), sFull.Mean(), "10: Diag from Full Mean");
        CheckClose(sDiag.Variance(), sFull.Variance(), "10: Diag from Full Variance");
        CheckEq(sDiag.SampleSize(), sFull.SampleSize(), "10: Diag from Full SampleSize");

        // Full from Diagonal
        Statistic<1> sFull2{sDiag};
        CheckClose(sFull2.Mean(), sFull.Mean(), "10: Full from Diag Mean");
        CheckClose(sFull2.Variance(), sFull.Variance(), "10: Full from Diag Variance");
        CheckEq(sFull2.SampleSize(), sFull.SampleSize(), "10: Full from Diag SampleSize");
    }

    // N=2
    {
        Statistic<2> sFull;
        sFull.Fill(Eigen::Vector2d{1.0, 2.0});
        sFull.Fill(Eigen::Vector2d{2.0, 4.0});
        sFull.Fill(Eigen::Vector2d{3.0, 6.0});
        sFull.Fill(Eigen::Vector2d{4.0, 8.0});
        sFull.Fill(Eigen::Vector2d{5.0, 10.0});

        // Diagonal from Full
        Statistic<2, CovarianceOption::Diagonal> sDiag{sFull};
        CheckClose(sDiag.Mean(), sFull.Mean(), "10: Diag from Full Mean");
        CheckClose(sDiag.Variance(), sFull.Variance(), "10: Diag from Full Variance");
        CheckClose(sDiag.Covariance(0, 1), 0.0, "10: Diag from Full Cov(0,1)=0");
        CheckEq(sDiag.SampleSize(), sFull.SampleSize(), "10: Diag from Full SampleSize");

        // Full from Diagonal
        Statistic<2> sFull2{sDiag};
        CheckClose(sFull2.Mean(), sFull.Mean(), "10: Full from Diag Mean");
        CheckClose(sFull2.Variance(), sFull.Variance(), "10: Full from Diag Variance");
        CheckClose(sFull2.Covariance(0, 1), 0.0, "10: Full from Diag Cov(0,1)=0");
    }
}};

// =========================================================================
// Section 10: Cross-CovarianceOption Assignment
// =========================================================================

constexpr auto sec10CrossCovAssign{[] {
    Statistic<2> sFullSrc;
    sFullSrc.Fill(Eigen::Vector2d{1.0, 2.0});
    sFullSrc.Fill(Eigen::Vector2d{2.0, 4.0});
    sFullSrc.Fill(Eigen::Vector2d{3.0, 6.0});

    Statistic<2, CovarianceOption::Diagonal> sDiagSrc;
    sDiagSrc.Fill(Eigen::Vector2d{4.0, 8.0});
    sDiagSrc.Fill(Eigen::Vector2d{5.0, 10.0});

    // Full = Diagonal
    Statistic<2> sFull;
    sFull = sDiagSrc;
    CheckClose(sFull.Mean(), sDiagSrc.Mean(), "10: Full=Diag Mean");
    CheckClose(sFull.Variance(), sDiagSrc.Variance(), "10: Full=Diag Variance");
    CheckClose(sFull.Covariance(0, 1), 0.0, "10: Full=Diag Cov(0,1)=0");

    // Diagonal = Full
    Statistic<2, CovarianceOption::Diagonal> sDiag;
    sDiag = sFullSrc;
    CheckClose(sDiag.Mean(), sFullSrc.Mean(), "10: Diag=Full Mean");
    CheckClose(sDiag.Variance(), sFullSrc.Variance(), "10: Diag=Full Variance");
    CheckClose(sDiag.Covariance(0, 1), 0.0, "10: Diag=Full Cov(0,1)=0");

    // Scalar: Full = Diagonal
    Statistic<1> s1Full;
    Statistic<1, CovarianceOption::Diagonal> s1Diag;
    s1Diag.Fill(3.0);
    s1Diag.Fill(7.0);
    s1Full = s1Diag;
    CheckClose(s1Full.Mean(), s1Diag.Mean(), "10: scalar Full=Diag Mean");
}};

// =========================================================================
// Section 10: Cross-CovarianceOption Merge (operator+=)
// =========================================================================

constexpr auto sec10CrossCovMerge{[] {
    // Full += Diagonal
    {
        Statistic<2> sFull;
        sFull.Fill(Eigen::Vector2d{1.0, 2.0});
        sFull.Fill(Eigen::Vector2d{2.0, 4.0});
        sFull.Fill(Eigen::Vector2d{3.0, 6.0});

        Statistic<2, CovarianceOption::Diagonal> sDiag;
        sDiag.Fill(Eigen::Vector2d{4.0, 8.0});
        sDiag.Fill(Eigen::Vector2d{5.0, 10.0});

        Statistic<2> sRef;
        sRef.Fill(Eigen::Vector2d{1.0, 2.0});
        sRef.Fill(Eigen::Vector2d{2.0, 4.0});
        sRef.Fill(Eigen::Vector2d{3.0, 6.0});
        sRef.Fill(Eigen::Vector2d{4.0, 8.0});
        sRef.Fill(Eigen::Vector2d{5.0, 10.0});

        sFull += sDiag;
        CheckClose(sFull.Mean(), sRef.Mean(), "10: Full+=Diag Mean");
        CheckClose(sFull.Variance(), sRef.Variance(), "10: Full+=Diag Variance");
        CheckClose(sFull.Covariance(0, 1), 4.75, "10: Full+=Diag Cov(0,1)");
        CheckEq(sFull.SampleSize(), sRef.SampleSize(), "10: Full+=Diag SampleSize");
    }

    // Diagonal += Full
    {
        Statistic<2, CovarianceOption::Diagonal> sDiag;
        sDiag.Fill(Eigen::Vector2d{1.0, 2.0});
        sDiag.Fill(Eigen::Vector2d{2.0, 4.0});

        Statistic<2> sFull;
        sFull.Fill(Eigen::Vector2d{3.0, 6.0});
        sFull.Fill(Eigen::Vector2d{4.0, 8.0});

        Statistic<2, CovarianceOption::Diagonal> sRef;
        sRef.Fill(Eigen::Vector2d{1.0, 2.0});
        sRef.Fill(Eigen::Vector2d{2.0, 4.0});
        sRef.Fill(Eigen::Vector2d{3.0, 6.0});
        sRef.Fill(Eigen::Vector2d{4.0, 8.0});

        sDiag += sFull;
        CheckClose(sDiag.Mean(), sRef.Mean(), "10: Diag+=Full Mean");
        CheckClose(sDiag.Variance(), sRef.Variance(), "10: Diag+=Full Variance");
        CheckClose(sDiag.Covariance(0, 1), 0.0, "10: Diag+=Full Cov(0,1)=0");
    }
}};

// =========================================================================
// Section 10: Cross-CovarianceOption operator+
// =========================================================================

constexpr auto sec10CrossCovAdd{[] {
    // Full + Diagonal
    {
        Statistic<2> sFull;
        sFull.Fill(Eigen::Vector2d{1.0, 2.0});
        sFull.Fill(Eigen::Vector2d{2.0, 4.0});

        Statistic<2, CovarianceOption::Diagonal> sDiag;
        sDiag.Fill(Eigen::Vector2d{3.0, 6.0});
        sDiag.Fill(Eigen::Vector2d{4.0, 8.0});
        sDiag.Fill(Eigen::Vector2d{5.0, 10.0});

        auto sResult{sFull + sDiag};

        Statistic<2> sRef;
        sRef.Fill(Eigen::Vector2d{1.0, 2.0});
        sRef.Fill(Eigen::Vector2d{2.0, 4.0});
        sRef.Fill(Eigen::Vector2d{3.0, 6.0});
        sRef.Fill(Eigen::Vector2d{4.0, 8.0});
        sRef.Fill(Eigen::Vector2d{5.0, 10.0});

        CheckClose(sResult.Mean(), sRef.Mean(), "10: Full+Diag Mean");
        CheckClose(sResult.Variance(), sRef.Variance(), "10: Full+Diag Variance");
        CheckClose(sResult.Covariance(0, 1), 4.0, "10: Full+Diag Cov(0,1)");
    }

    // Diagonal + Full
    {
        Statistic<2, CovarianceOption::Diagonal> sDiag;
        sDiag.Fill(Eigen::Vector2d{1.0, 2.0});
        sDiag.Fill(Eigen::Vector2d{2.0, 4.0});

        Statistic<2> sFull;
        sFull.Fill(Eigen::Vector2d{3.0, 6.0});
        sFull.Fill(Eigen::Vector2d{4.0, 8.0});
        sFull.Fill(Eigen::Vector2d{5.0, 10.0});

        auto sResult{sDiag + sFull};

        Statistic<2> sRef;
        sRef.Fill(Eigen::Vector2d{1.0, 2.0});
        sRef.Fill(Eigen::Vector2d{2.0, 4.0});
        sRef.Fill(Eigen::Vector2d{3.0, 6.0});
        sRef.Fill(Eigen::Vector2d{4.0, 8.0});
        sRef.Fill(Eigen::Vector2d{5.0, 10.0});

        CheckClose(sResult.Mean(), sRef.Mean(), "10: Diag+Full Mean");
        CheckClose(sResult.Variance(), sRef.Variance(), "10: Diag+Full Variance");
        CheckClose(sResult.Covariance(0, 1), 4.75, "10: Diag+Full Cov(0,1)");
    }

    // Scalar: Full + Diagonal
    {
        Statistic<1> s1Full;
        s1Full.Fill(1.0);
        s1Full.Fill(2.0);
        Statistic<1, CovarianceOption::Diagonal> s1Diag;
        s1Diag.Fill(3.0);
        s1Diag.Fill(4.0);

        auto sResult{s1Full + s1Diag};

        Statistic<1> sRef;
        sRef.Fill(1.0);
        sRef.Fill(2.0);
        sRef.Fill(3.0);
        sRef.Fill(4.0);

        CheckClose(sResult.Mean(), sRef.Mean(), "10: scalar Full+Diag Mean");
        CheckClose(sResult.Variance(), sRef.Variance(), "10: scalar Full+Diag Variance");
    }
}};

// =========================================================================
// Section 10: Cross-Dimension-Type Copy (Static <-> Dynamic)
// =========================================================================

constexpr auto sec10CrossDimCopy{[] {
    // Static from Dynamic
    {
        Statistic<Eigen::Dynamic> sDyn(3);
        sDyn.Fill(Eigen::Vector3d{1.0, 2.0, 3.0});
        sDyn.Fill(Eigen::Vector3d{4.0, 5.0, 6.0});

        Statistic<3> sStat{sDyn};
        CheckEq(sStat.Dimension(), 3, "10: Static from Dynamic Dimension");
        CheckClose(sStat.Mean(0), sDyn.Mean(0), "10: Static from Dynamic Mean(0)");
        CheckClose(sStat.Mean(1), sDyn.Mean(1), "10: Static from Dynamic Mean(1)");
        CheckClose(sStat.Mean(2), sDyn.Mean(2), "10: Static from Dynamic Mean(2)");
        CheckClose(sStat.Variance(0), sDyn.Variance(0), "10: Static from Dynamic Variance(0)");
        CheckEq(sStat.SampleSize(), sDyn.SampleSize(), "10: Static from Dynamic SampleSize");
    }

    // Dynamic from Static
    {
        Statistic<3> sStat;
        sStat.Fill(Eigen::Vector3d{1.0, 2.0, 3.0});
        sStat.Fill(Eigen::Vector3d{4.0, 5.0, 6.0});

        Statistic<Eigen::Dynamic> sDyn{sStat};
        CheckEq(sDyn.Dimension(), 3, "10: Dynamic from Static Dimension");
        CheckClose(sDyn.Mean(0), sStat.Mean(0), "10: Dynamic from Static Mean(0)");
        CheckClose(sDyn.Mean(1), sStat.Mean(1), "10: Dynamic from Static Mean(1)");
        CheckClose(sDyn.Variance(0), sStat.Variance(0), "10: Dynamic from Static Variance(0)");
        CheckEq(sDyn.SampleSize(), sStat.SampleSize(), "10: Dynamic from Static SampleSize");
    }
}};

// =========================================================================
// Section 10: Cross-Dimension-Type Merge (Static <-> Dynamic)
// =========================================================================

constexpr auto sec10CrossDimMerge{[] {
    // Static += Dynamic
    {
        Statistic<3> sStat;
        sStat.Fill(Eigen::Vector3d{1.0, 2.0, 3.0});
        sStat.Fill(Eigen::Vector3d{2.0, 3.0, 4.0});

        Statistic<Eigen::Dynamic> sDyn(3);
        sDyn.Fill(Eigen::Vector3d{3.0, 4.0, 5.0});

        Statistic<3> sRef;
        sRef.Fill(Eigen::Vector3d{1.0, 2.0, 3.0});
        sRef.Fill(Eigen::Vector3d{2.0, 3.0, 4.0});
        sRef.Fill(Eigen::Vector3d{3.0, 4.0, 5.0});

        sStat += sDyn;
        CheckClose(sStat.Mean(), sRef.Mean(), "10: Static+=Dynamic Mean");
        CheckClose(sStat.Variance(), sRef.Variance(), "10: Static+=Dynamic Variance");
        CheckEq(sStat.SampleSize(), sRef.SampleSize(), "10: Static+=Dynamic SampleSize");
    }

    // Dynamic += Static
    {
        Statistic<Eigen::Dynamic> sDyn(3);
        sDyn.Fill(Eigen::Vector3d{1.0, 2.0, 3.0});

        Statistic<3> sStat;
        sStat.Fill(Eigen::Vector3d{2.0, 3.0, 4.0});
        sStat.Fill(Eigen::Vector3d{3.0, 4.0, 5.0});

        Statistic<Eigen::Dynamic> sRef(3);
        sRef.Fill(Eigen::Vector3d{1.0, 2.0, 3.0});
        sRef.Fill(Eigen::Vector3d{2.0, 3.0, 4.0});
        sRef.Fill(Eigen::Vector3d{3.0, 4.0, 5.0});

        sDyn += sStat;
        CheckClose(sDyn.Mean(), sRef.Mean(), "10: Dynamic+=Static Mean");
        CheckClose(sDyn.Variance(), sRef.Variance(), "10: Dynamic+=Static Variance");
        CheckEq(sDyn.SampleSize(), sRef.SampleSize(), "10: Dynamic+=Static SampleSize");
    }

    // Static Full += Dynamic Diagonal (mixed everything)
    {
        Statistic<3> sStat;
        sStat.Fill(Eigen::Vector3d{1.0, 2.0, 3.0});

        Statistic<Eigen::Dynamic, CovarianceOption::Diagonal> sDyn(3);
        sDyn.Fill(Eigen::Vector3d{4.0, 5.0, 6.0});

        Statistic<3> sRef;
        sRef.Fill(Eigen::Vector3d{1.0, 2.0, 3.0});
        sRef.Fill(Eigen::Vector3d{4.0, 5.0, 6.0});

        sStat += sDyn;
        CheckClose(sStat.Mean(), sRef.Mean(), "10: Static+=DynamicDiag Mean");
        CheckClose(sStat.Variance(), sRef.Variance(), "10: Static+=DynamicDiag Variance");
        CheckEq(sStat.SampleSize(), sRef.SampleSize(), "10: Static+=DynamicDiag SampleSize");
    }
}};

// =========================================================================
// Section 10: Cross-CovarianceOption Rvalue operator+
// =========================================================================

constexpr auto sec10CrossCovRvalueAdd{[] {
    // Full + move(Diagonal)
    {
        Statistic<2> sFull;
        sFull.Fill(Eigen::Vector2d{1.0, 2.0});
        sFull.Fill(Eigen::Vector2d{2.0, 4.0});

        auto sDiagCopy{[] {
            Statistic<2, CovarianceOption::Diagonal> s;
            s.Fill(Eigen::Vector2d{3.0, 6.0});
            s.Fill(Eigen::Vector2d{4.0, 8.0});
            return s;
        }()};

        auto sResult{sFull + std::move(sDiagCopy)};

        Statistic<2> sRef;
        sRef.Fill(Eigen::Vector2d{1.0, 2.0});
        sRef.Fill(Eigen::Vector2d{2.0, 4.0});
        sRef.Fill(Eigen::Vector2d{3.0, 6.0});
        sRef.Fill(Eigen::Vector2d{4.0, 8.0});

        CheckClose(sResult.Mean(), sRef.Mean(), "10r: Full+move(Diag) Mean");
        CheckClose(sResult.Variance(), sRef.Variance(), "10r: Full+move(Diag) Variance");
    }
    // move(Full) + Diagonal
    {
        auto sFullCopy{[] {
            Statistic<2> s;
            s.Fill(Eigen::Vector2d{1.0, 2.0});
            s.Fill(Eigen::Vector2d{2.0, 4.0});
            return s;
        }()};

        Statistic<2, CovarianceOption::Diagonal> sDiag;
        sDiag.Fill(Eigen::Vector2d{3.0, 6.0});
        sDiag.Fill(Eigen::Vector2d{4.0, 8.0});

        auto sResult{std::move(sFullCopy) + sDiag};

        Statistic<2> sRef;
        sRef.Fill(Eigen::Vector2d{1.0, 2.0});
        sRef.Fill(Eigen::Vector2d{2.0, 4.0});
        sRef.Fill(Eigen::Vector2d{3.0, 6.0});
        sRef.Fill(Eigen::Vector2d{4.0, 8.0});

        CheckClose(sResult.Mean(), sRef.Mean(), "10r: move(Full)+Diag Mean");
        CheckClose(sResult.Variance(), sRef.Variance(), "10r: move(Full)+Diag Variance");
    }
}};

// =========================================================================
// Section 10: Cross-Dimension-Type Rvalue operator+
// =========================================================================

constexpr auto sec10CrossDimRvalueAdd{[] {
    // Static + move(Dynamic)
    {
        Statistic<3> sStat;
        sStat.Fill(Eigen::Vector3d{1.0, 2.0, 3.0});
        sStat.Fill(Eigen::Vector3d{2.0, 3.0, 4.0});

        auto sDynCopy{[] {
            Statistic<Eigen::Dynamic> s(3);
            s.Fill(Eigen::Vector3d{3.0, 4.0, 5.0});
            return s;
        }()};

        auto sResult{sStat + std::move(sDynCopy)};

        Statistic<3> sRef;
        sRef.Fill(Eigen::Vector3d{1.0, 2.0, 3.0});
        sRef.Fill(Eigen::Vector3d{2.0, 3.0, 4.0});
        sRef.Fill(Eigen::Vector3d{3.0, 4.0, 5.0});

        CheckClose(sResult.Mean(), sRef.Mean(), "10r: Static+move(Dynamic) Mean");
        CheckClose(sResult.Variance(), sRef.Variance(), "10r: Static+move(Dynamic) Variance");
    }
    // move(Dynamic) + Static
    {
        auto sDynCopy{[] {
            Statistic<Eigen::Dynamic> s(3);
            s.Fill(Eigen::Vector3d{1.0, 2.0, 3.0});
            return s;
        }()};

        Statistic<3> sStat;
        sStat.Fill(Eigen::Vector3d{2.0, 3.0, 4.0});
        sStat.Fill(Eigen::Vector3d{3.0, 4.0, 5.0});

        auto sResult{std::move(sDynCopy) + sStat};

        Statistic<3> sRef;
        sRef.Fill(Eigen::Vector3d{1.0, 2.0, 3.0});
        sRef.Fill(Eigen::Vector3d{2.0, 3.0, 4.0});
        sRef.Fill(Eigen::Vector3d{3.0, 4.0, 5.0});

        CheckClose(sResult.Mean(), sRef.Mean(), "10r: move(Dynamic)+Static Mean");
        CheckClose(sResult.Variance(), sRef.Variance(), "10r: move(Dynamic)+Static Variance");
    }
}};

} // namespace TestStatisticSection

auto TestStatistic10::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestStatisticSection;

    PrintLn("--- Section 10: Smoke Test ---");
    sec10Smoke();
    PrintLn("  10 smoke passed: cross-verification compilation smoke");

    PrintLn("--- Section 10: Cross-Verification ---");
    sec10CrossCovCopy();
    PrintLn("  10 passed: cross-CovarianceOption copy construction");
    sec10CrossCovAssign();
    PrintLn("  10 passed: cross-CovarianceOption assignment");
    sec10CrossCovMerge();
    PrintLn("  10 passed: cross-CovarianceOption merge");
    sec10CrossCovAdd();
    PrintLn("  10 passed: cross-CovarianceOption operator+");
    sec10CrossDimCopy();
    PrintLn("  10 passed: cross-dimension-type copy");
    sec10CrossDimMerge();
    PrintLn("  10 passed: cross-dimension-type merge");
    sec10CrossCovRvalueAdd();
    PrintLn("  10 passed: cross-CovarianceOption rvalue operator+");
    sec10CrossDimRvalueAdd();
    PrintLn("  10 passed: cross-dimension-type rvalue operator+");

    PrintLn("All TestStatistic10 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
