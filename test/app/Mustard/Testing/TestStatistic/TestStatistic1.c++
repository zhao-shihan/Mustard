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

TestStatistic1::TestStatistic1() :
    Subprogram{"TestStatistic1", "Test Mustard::Statistic (Section 1: Fill and Statistics)."} {}

namespace TestStatisticSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestStatistic;

// =========================================================================
// Section 1 Smoke: Fill and Accessors (from sec0Smoke)
// =========================================================================

constexpr auto sec1Smoke{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr auto dim{(K == Eigen::Dynamic) ? 3 : K};

    Stat s{MakeStatistic<K, C>(dim)};

    // Fill
    if constexpr (K == 1) {
        s.Fill(1.0);
        s.Fill(2.0, 0.5);
    } else {
        auto v{MakeSeqVector<K>(dim, 1.0)};
        s.Fill(v);
        s.Fill(2.0 * v, 0.5);
    }

    // Accessors — meta
    [[maybe_unused]] auto d{s.Dimension()};
    [[maybe_unused]] auto n{s.SampleSize()};
    [[maybe_unused]] auto ws{s.WeightSum()};
    [[maybe_unused]] auto w2{s.WeightSquareSum()};
    [[maybe_unused]] auto ess{s.EffectiveSampleSize()};

    // Accessors — bulk
    [[maybe_unused]] auto sum{s.Sum()};
    [[maybe_unused]] auto mean{s.Mean()};
    [[maybe_unused]] auto var{s.Variance()};
    [[maybe_unused]] auto std{s.StdDev()};
    if constexpr (K != 1) {
        [[maybe_unused]] auto cov{s.Covariance()};
        [[maybe_unused]] auto covMean{s.CovarianceOfMean()};
        [[maybe_unused]] auto corr{s.Correlation()};
        [[maybe_unused]] auto corrMean{s.CorrelationOfMean()};
    }
    [[maybe_unused]] auto varMean{s.VarianceOfMean()};
    [[maybe_unused]] auto stdMean{s.StdDevOfMean()};
    [[maybe_unused]] auto est{s.MeanEstimate()};

    // Accessors — per-component
    if constexpr (K == 1) {
        [[maybe_unused]] auto s0{s.Sum()};
        [[maybe_unused]] auto m0{s.Mean()};
        [[maybe_unused]] auto v0{s.Variance()};
        [[maybe_unused]] auto sd0{s.StdDev()};
        [[maybe_unused]] auto vm0{s.VarianceOfMean()};
        [[maybe_unused]] auto sm0{s.StdDevOfMean()};
        [[maybe_unused]] auto e0{s.MeanEstimate()};
    } else {
        for (auto i{0}; i < dim; ++i) {
            [[maybe_unused]] auto si{s.Sum(i)};
            [[maybe_unused]] auto mi{s.Mean(i)};
            [[maybe_unused]] auto vi{s.Variance(i)};
            [[maybe_unused]] auto sdi{s.StdDev(i)};
            [[maybe_unused]] auto vmi{s.VarianceOfMean(i)};
            [[maybe_unused]] auto smi{s.StdDevOfMean(i)};
            [[maybe_unused]] auto ei{s.MeanEstimate(i)};
            [[maybe_unused]] auto ci{s.Covariance(i, i)};
            [[maybe_unused]] auto cri{s.Correlation(i, i)};
            if constexpr (dim >= 2) {
                [[maybe_unused]] auto co{s.Covariance(i, (i + 1) % dim)};
                [[maybe_unused]] auto com{s.CovarianceOfMean(i, (i + 1) % dim)};
                [[maybe_unused]] auto cro{s.Correlation(i, (i + 1) % dim)};
                [[maybe_unused]] auto crom{s.CorrelationOfMean(i, (i + 1) % dim)};
            }
        }
    }
}};

// =========================================================================
// Section 1: Fill and Statistics (K>=1)
// Dataset: for each v in {1,2,3,4,5}, component i = (i+1)*v
// Derived (generic for dim components):
//   Mean(i) = 3*(i+1)
//   Variance(i) = 2.5*(i+1)²
//   Sum(i) = 15*(i+1)
//   Cov(i,j) = 2.5*(i+1)*(j+1) [Full], 0 [Diagonal for i!=j]
//   CovOfMean(i,j) = 0.5*(i+1)*(j+1)
//   VarOfMean(i) = 0.5*(i+1)²
// =========================================================================

constexpr auto sec1FillAndStatistics{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr auto dim{isDynamic ? 2 : K};

    Stat s{MakeStatistic<K, C>(dim)};

    // Fill 5 observations: component i = (i+1)*v for v in {1,2,3,4,5}
    if constexpr (K == 1) {
        for (auto v{1}; v <= 5; ++v) {
            s.Fill(static_cast<double>(v));
        }
    } else {
        for (auto v{1}; v <= 5; ++v) {
            s.Fill(v * MakeSeqVector<K>(dim, 1.0));
        }
    }

    CheckEq(s.SampleSize(), 5LL, "1: SampleSize");
    CheckClose(s.WeightSum(), 5.0, "1: WeightSum");
    CheckClose(s.WeightSquareSum(), 5.0, "1: WeightSquareSum");
    CheckClose(s.EffectiveSampleSize(), 5.0, "1: ESS");

    // Element-wise / scalar accessors
    if constexpr (K == 1) {
        CheckClose(s.Sum(), 15.0, "1: Sum()");
        CheckClose(s.Mean(), 3.0, "1: Mean()");
        CheckClose(s.Variance(), 2.5, "1: Variance()");
        CheckClose(s.StdDev(), std::sqrt(2.5), "1: StdDev()");
        CheckClose(s.VarianceOfMean(), 0.5, "1: VarianceOfMean()");
        CheckClose(s.StdDevOfMean(), std::sqrt(0.5), "1: StdDevOfMean()");
    } else {
        for (auto i{0}; i < dim; ++i) {
            const auto coeff{static_cast<double>(i + 1)};
            CheckClose(s.Sum(i), 15.0 * coeff,
                       fmt::format("1: Sum({})", i));
            CheckClose(s.Mean(i), 3.0 * coeff,
                       fmt::format("1: Mean({})", i));
            CheckClose(s.Variance(i), 2.5 * coeff * coeff,
                       fmt::format("1: Variance({})", i));
            CheckClose(s.StdDev(i), std::sqrt(2.5) * coeff,
                       fmt::format("1: StdDev({})", i));

            // Diagonal covariance
            CheckClose(s.Covariance(i, i), 2.5 * coeff * coeff,
                       fmt::format("1: Cov({},{})", i, i));

            // CovarianceOfMean
            CheckClose(s.VarianceOfMean(i), 0.5 * coeff * coeff,
                       fmt::format("1: VarianceOfMean({})", i));
            CheckClose(s.StdDevOfMean(i), std::sqrt(0.5) * coeff,
                       fmt::format("1: StdDevOfMean({})", i));
        }
    }

    // Off-diagonal covariance
    if constexpr (K != 1) {
        for (auto i{0}; i < dim; ++i) {
            for (auto j{i + 1}; j < dim; ++j) {
                const auto ci{static_cast<double>(i + 1)};
                const auto cj{static_cast<double>(j + 1)};
                if constexpr (C == CovarianceOption::Full) {
                    CheckClose(s.Covariance(i, j), 2.5 * ci * cj,
                               fmt::format("1: Cov({},{})", i, j));
                    CheckClose(s.Covariance(j, i), 2.5 * ci * cj,
                               fmt::format("1: Cov({},{})", j, i));
                    CheckClose(s.CovarianceOfMean(i, j), 0.5 * ci * cj,
                               fmt::format("1: CovarianceOfMean({},{})", i, j));
                } else {
                    CheckClose(s.Covariance(i, j), 0.0,
                               fmt::format("1: Cov({},{})", i, j));
                    CheckClose(s.Covariance(j, i), 0.0,
                               fmt::format("1: Cov({},{})", j, i));
                }
            }
        }
        // Correlation checks
        for (auto i{0}; i < dim; ++i) {
            CheckClose(s.Correlation(i, i), 1.0,
                       fmt::format("1: Corr({},{})", i, i));
            CheckClose(s.CorrelationOfMean(i, i), 1.0,
                       fmt::format("1: CorrOfMean({},{})", i, i));
            for (auto j{i + 1}; j < dim; ++j) {
                if constexpr (C == CovarianceOption::Full) {
                    CheckClose(s.Correlation(i, j), 1.0,
                               fmt::format("1: Corr({},{})", i, j));
                    CheckClose(s.CorrelationOfMean(i, j), 1.0,
                               fmt::format("1: CorrOfMean({},{})", i, j));
                } else {
                    CheckClose(s.Correlation(i, j), 0.0,
                               fmt::format("1: Corr({},{})", i, j));
                    CheckClose(s.CorrelationOfMean(i, j), 0.0,
                               fmt::format("1: CorrOfMean({},{})", i, j));
                }
            }
        }
    }

    // Bulk accessor sanity checks
    if constexpr (K != 1) {
        CheckEq(s.Mean().size(), dim, "1: Mean() size");
        CheckEq(s.Variance().size(), dim, "1: Variance() size");
        CheckEq(s.Covariance().rows(), dim, "1: Covariance() rows");
        CheckClose(s.Mean()(0), s.Mean(0), "1: Mean()(0) vs Mean(0)");
        CheckClose(s.Variance()(dim - 1), s.Variance(dim - 1),
                   "1: Variance()(last) vs Variance(last)");
    }

    // MeanEstimate
    if constexpr (K == 1) {
        const auto est{s.MeanEstimate()};
        CheckClose(est.Value(), 3.0, "1: MeanEstimate().value");
        CheckClose(est.Uncertainty(), std::sqrt(0.5), "1: MeanEstimate().uncertainty");
    } else {
        for (auto i{0}; i < dim; ++i) {
            const auto est{s.MeanEstimate(i)};
            const auto coeff{static_cast<double>(i + 1)};
            CheckClose(est.Value(), 3.0 * coeff,
                       fmt::format("1: MeanEstimate({}).value", i));
            CheckClose(est.Uncertainty(), std::sqrt(0.5) * coeff,
                       fmt::format("1: MeanEstimate({}).uncertainty", i));
        }

        // MeanEstimate vector form
        const auto estVec{s.MeanEstimate()};
        for (auto i{0}; i < dim; ++i) {
            const auto coeff{static_cast<double>(i + 1)};
            CheckClose(estVec.Value()(i), 3.0 * coeff,
                       fmt::format("1: MeanEstimate().value({})", i));
            CheckClose(estVec.Uncertainty()(i), std::sqrt(0.5) * coeff,
                       fmt::format("1: MeanEstimate().uncertainty({})", i));
            if constexpr (C == CovarianceOption::Full) {
                for (auto j{i + 1}; j < dim; ++j) {
                    const auto cj{static_cast<double>(j + 1)};
                    CheckClose(estVec.Covariance()(i, j), 0.5 * coeff * cj,
                               fmt::format("1: MeanEstimate().cov({},{})", i, j));
                }
            }
        }
    }
}};

// =========================================================================
// Section 1w: Weighted Statistics (K>=1)
// Fill with weighted observations (w = 2, 3, 1 for v = 1, 2, 3)
//   W = 6, W2 = 14, ESS = 36/14, M(i) = (i+1)*11, Mean(i) = (i+1)*11/6
// =========================================================================

constexpr auto sec1WeightedStatistics{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr auto dim{isDynamic ? 2 : K};

    Stat s{MakeStatistic<K, C>(dim)};
    if constexpr (K == 1) {
        s.Fill(1.0, 2.0);
        s.Fill(2.0, 3.0);
        s.Fill(3.0, 1.0);
    } else {
        s.Fill(1.0 * MakeSeqVector<K>(dim, 1.0), 2.0);
        s.Fill(2.0 * MakeSeqVector<K>(dim, 1.0), 3.0);
        s.Fill(3.0 * MakeSeqVector<K>(dim, 1.0), 1.0);
    }

    // Meta
    CheckEq(s.SampleSize(), 3LL, "1w: SampleSize");
    CheckClose(s.WeightSum(), 6.0, "1w: WeightSum");
    CheckClose(s.WeightSquareSum(), 14.0, "1w: WeightSquareSum");
    CheckClose(s.EffectiveSampleSize(), 36.0 / 14.0, "1w: ESS");

    // Sum and Mean
    if constexpr (K == 1) {
        CheckClose(s.Sum(), 11.0, "1w: Sum");
        CheckClose(s.Mean(), 11.0 / 6.0, "1w: Mean");
    } else {
        for (auto i{0}; i < dim; ++i) {
            const auto coeff{static_cast<double>(i + 1)};
            CheckClose(s.Sum(i), 11.0 * coeff, fmt::format("1w: Sum({})", i));
            CheckClose(s.Mean(i), 11.0 / 6.0 * coeff,
                       fmt::format("1w: Mean({})", i));
        }
    }

    // Exact Variance for scalar case
    if constexpr (K == 1) {
        CheckClose(s.Variance(), 17.0 / 22.0, "1w: Variance");
    }

    // Internal consistency
    if constexpr (K != 1) {
        for (auto i{0}; i < dim; ++i) {
            CheckClose(s.Variance(i), s.Covariance(i, i),
                       fmt::format("1w: Var({}) == Cov({},{})", i, i, i));
            CheckClose(s.StdDev(i), std::sqrt(s.Variance(i)),
                       fmt::format("1w: StdDev({}) consistency", i));
            CheckClose(s.VarianceOfMean(i), s.CovarianceOfMean(i, i),
                       fmt::format("1w: VarOfMean({}) == CovOfMean({},{})", i, i, i));
            CheckClose(s.StdDevOfMean(i), std::sqrt(s.VarianceOfMean(i)),
                       fmt::format("1w: StdDevOfMean({}) consistency", i));
        }
        // Relative scaling between components: Var(1) = 4 * Var(0)
        CheckClose(s.Variance(1), 4.0 * s.Variance(0), "1w: Var(1) == 4*Var(0)");
        if constexpr (C == CovarianceOption::Full) {
            CheckClose(s.Covariance(0, 1), s.Covariance(1, 0), "1w: Cov symmetry");
            CheckClose(s.CovarianceOfMean(0, 1), s.CovarianceOfMean(1, 0),
                       "1w: CovOfMean symmetry");
        }
    }
}};

} // namespace TestStatisticSection

auto TestStatistic1::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestStatisticSection;

    PrintLn("--- Section 1: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec1Smoke);
    PrintLn("  1 smoke passed: fill and accessors compilation smoke (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 1: Fill and Statistics ---");
    RunOverAllDims<AllStaticDims>(sec1FillAndStatistics);
    PrintLn("  1 passed: fill and statistics (K=1,2,3,5,10 Full/Diag + dynamic)");
    RunOverAllDims<AllStaticDims>(sec1WeightedStatistics);
    PrintLn("  1w passed: weighted statistics (K=1,2,3,5,10 Full/Diag + dynamic)");

    PrintLn("All TestStatistic1 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
