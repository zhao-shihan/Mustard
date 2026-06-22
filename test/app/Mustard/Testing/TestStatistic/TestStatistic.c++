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
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/IO/Print.h++"
#include "Mustard/Math/Statistic.h++"
#include "Mustard/Testing/TestStatistic/TestStatistic.h++"
#include "Mustard/Utility/Base64.h++"

#include "Eigen/Core"

#include "muc/numeric"

#include "fmt/format.h"
#include "fmt/ostream.h"

#include <array>
#include <cmath>
#include <cstdlib>
#include <initializer_list>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace Mustard::Testing {

TestStatistic::TestStatistic() :
    Subprogram{"TestStatistic", "Test Mustard::Math::Statistic."} {}

namespace TestStatisticSection {

using namespace Mustard::Math;

// =========================================================================
// Helpers: generate test data for arbitrary dimension
// =========================================================================

// Generate a vector [start, start+1, ..., start+dim-1]
template<int K>
auto MakeSeqVector(int dim, double start = 1.0) -> auto {
    if constexpr (K == Eigen::Dynamic) {
        Eigen::VectorXd v{dim};
        for (auto i{0}; i < dim; ++i) {
            v[i] = start + i;
        }
        return v;
    } else {
        Eigen::Vector<double, K> v;
        for (auto i{0}; i < dim; ++i) {
            v[i] = start + i;
        }
        return v;
    }
}

// Generate a constant vector [val, val, ..., val]
template<int K>
auto MakeConstVector(int dim, double val) -> auto {
    if constexpr (K == Eigen::Dynamic) {
        Eigen::VectorXd v{dim};
        for (auto i{0}; i < dim; ++i) {
            v[i] = val;
        }
        return v;
    } else {
        Eigen::Vector<double, K> v;
        for (auto i{0}; i < dim; ++i) {
            v[i] = val;
        }
        return v;
    }
}

// =========================================================================
// Helper: construct Statistic with correct dynamic/static behavior
// =========================================================================
template<int K, CovarianceOption C = CovarianceOption::Full>
auto MakeStatistic(int dim = 0) -> Statistic<K, C> {
    if constexpr (K == Eigen::Dynamic) {
        return Statistic<K, C>(dim);
    } else {
        return Statistic<K, C>{};
    }
}

// =========================================================================
// Iteration helpers
// =========================================================================

// All static K dimensions to test (K>=1)
using AllStaticDims = std::integer_sequence<int, 1, 2, 3, 5, 10>;

// Run a test function over all static K>=2 with both Full and Diagonal
template<int... Ks, typename Func, typename... Args>
auto RunOverStaticDims(Func&& func, Args&&... args) -> void {
    (std::forward<Func>(func).template operator()<Ks, CovarianceOption::Full>(std::forward<Args>(args)...), ...);
    (std::forward<Func>(func).template operator()<Ks, CovarianceOption::Diagonal>(std::forward<Args>(args)...), ...);
}

// Run a test function over dynamic dimension with both Full and Diagonal
template<typename Func, typename... Args>
auto RunOverDynamic(Func&& func, Args&&... args) -> void {
    std::forward<Func>(func).template operator()<Eigen::Dynamic, CovarianceOption::Full>(std::forward<Args>(args)...);
    std::forward<Func>(func).template operator()<Eigen::Dynamic, CovarianceOption::Diagonal>(std::forward<Args>(args)...);
}

// Run a test function over all K>=2 (static + dynamic) with both Full and Diagonal
template<int... Ks, typename Func, typename... Args>
auto RunOverAllDims(Func&& func, Args&&... args) -> void {
    RunOverStaticDims<Ks...>(std::forward<Func>(func), std::forward<Args>(args)...);
    RunOverDynamic(std::forward<Func>(func), std::forward<Args>(args)...);
}
// Wrappers that accept std::integer_sequence (AllStaticDims) and deduce the int pack
template<typename Seq, typename Func, typename... Args>
auto RunOverStaticDims(Func&& func, Args&&... args) -> void {
    [&]<int... Ks>(std::integer_sequence<int, Ks...>) {
        RunOverStaticDims<Ks...>(std::forward<Func>(func), std::forward<Args>(args)...);
    }(Seq{});
}

template<typename Seq, typename Func, typename... Args>
auto RunOverAllDims(Func&& func, Args&&... args) -> void {
    [&]<int... Ks>(std::integer_sequence<int, Ks...>) {
        RunOverAllDims<Ks...>(std::forward<Func>(func), std::forward<Args>(args)...);
    }(Seq{});
}

// =========================================================================
// Checker functions
// =========================================================================

template<typename T, typename U>
auto CheckClose(const T& actual, const U& expected, const std::string& ctx) -> void {
    if constexpr (std::is_arithmetic_v<T>) {
        if (not muc::isclose(actual, expected)) {
            Throw<std::runtime_error>(fmt::format("\n"
                                                  "{}: expected {}, got {}",
                                                  ctx, expected, actual));
        }
    } else {
        auto isCloseMat{[](const auto& a, const auto& b) {
            for (auto i{0}; i < a.rows(); ++i) {
                for (auto j{0}; j < a.cols(); ++j) {
                    if (not muc::isclose(a.coeff(i, j), b.coeff(i, j))) {
                        return false;
                    }
                }
            }
            return true;
        }};
        if (not isCloseMat(actual, expected)) {
            Throw<std::runtime_error>(fmt::format("\n"
                                                  "{}: expected approx\n"
                                                  "{},\n"
                                                  "got\n"
                                                  "{}",
                                                  ctx, fmt::streamed(expected), fmt::streamed(actual)));
        }
    }
}

auto CheckEq(auto actual, auto expected, const std::string& ctx) -> void {
    if (actual != expected) {
        Throw<std::runtime_error>(fmt::format("\n"
                                              "{}: expected {}, got {}",
                                              ctx, expected, actual));
    }
}

// =========================================================================
// Section 0: Smoke test — call every API to verify compilation
// =========================================================================

// 0.1 Static K>=1, both covariance options
constexpr auto sec0SmokeStatic{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    using POD = StatisticPOD<K, C>;
    constexpr auto dim{K};

    // Construction
    Stat s1;
    Stat s2{s1};
    Stat s3{std::move(s2)};
    POD pod{};
    Stat s4{pod};

    // Assignment
    s2 = s1;
    s1 = std::move(s2);

    // Fill
    if constexpr (K == 1) {
        s1.Fill(1.0);
        s1.Fill(2.0, 0.5);
    } else {
        auto v{MakeSeqVector<K>(dim, 1.0)};
        s1.Fill(v);
        s1.Fill(2.0 * v, 0.5);
    }

    // Accessors — meta
    s1.Dimension();
    s1.SampleSize();
    s1.WeightSum();
    s1.WeightSquareSum();
    s1.EffectiveSampleSize();

    // Accessors — bulk
    s1.Sum();
    s1.Mean();
    s1.Variance();
    s1.StdDev();
    if constexpr (K != 1) {
        s1.Covariance();
        s1.CovarianceOfMean();
        s1.Correlation();
        s1.CorrelationOfMean();
    }
    s1.VarianceOfMean();
    s1.StdDevOfMean();
    s1.MeanEstimate();

    // Accessors — per-component
    if constexpr (K == 1) {
        s1.Sum();
        s1.Mean();
        s1.Variance();
        s1.StdDev();
        s1.VarianceOfMean();
        s1.StdDevOfMean();
        s1.MeanEstimate();
    } else {
        for (auto i{0}; i < dim; ++i) {
            s1.Sum(i);
            s1.Mean(i);
            s1.Variance(i);
            s1.StdDev(i);
            s1.VarianceOfMean(i);
            s1.StdDevOfMean(i);
            s1.MeanEstimate(i);
            s1.Covariance(i, i);
            s1.Correlation(i, i);
            if constexpr (dim >= 2) {
                s1.Covariance(i, (i + 1) % dim);
                s1.CovarianceOfMean(i, (i + 1) % dim);
                s1.Correlation(i, (i + 1) % dim);
                s1.CorrelationOfMean(i, (i + 1) % dim);
            }
        }
    }

    // Merge
    s1 += s3;

    // Combine (all 4 ref-qualifier overloads)
    (s1 + s3);
    (s1 + Stat{s3});
    (Stat{s1} + s3);
    (Stat{s1} + Stat{s3});

    // ToPOD/FromPOD
    auto pod2{s1.ToPOD()};
    Stat s5{pod2};
    s5.FromPOD(pod2);
}};

// 0.4 Dynamic dimension, both covariance options
constexpr auto sec0SmokeDynamic{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    static_assert(K == Eigen::Dynamic);
    constexpr auto dim{3};

    // Construction with dim
    Stat s1(dim);
    Stat s2{s1};
    Stat s3{std::move(s2)};

    // Assignment
    s2 = s1;
    s1 = std::move(s2);

    // Fill (vector)
    auto v{MakeSeqVector<K>(dim, 1.0)};
    s1.Fill(v);
    s1.Fill(2.0 * v, 0.5);

    // Accessors — meta
    s1.Dimension();
    s1.SampleSize();
    s1.WeightSum();
    s1.WeightSquareSum();
    s1.EffectiveSampleSize();

    // Accessors — bulk
    s1.Sum();
    s1.Mean();
    s1.Variance();
    s1.StdDev();
    s1.Covariance();
    s1.CovarianceOfMean();
    s1.Correlation();
    s1.CorrelationOfMean();
    s1.VarianceOfMean();
    s1.StdDevOfMean();
    s1.MeanEstimate();

    // Accessors — per-component
    for (auto i{0}; i < dim; ++i) {
        s1.Sum(i);
        s1.Mean(i);
        s1.Variance(i);
        s1.StdDev(i);
        s1.VarianceOfMean(i);
        s1.StdDevOfMean(i);
        s1.MeanEstimate(i);
        s1.Covariance(i, i);
        s1.Correlation(i, i);
        s1.Covariance(i, (i + 1) % dim);
        s1.CovarianceOfMean(i, (i + 1) % dim);
        s1.Correlation(i, (i + 1) % dim);
        s1.CorrelationOfMean(i, (i + 1) % dim);
    }

    // Merge
    s1 += s3;

    // Combine (all 4 ref-qualifier overloads)
    (s1 + s3);
    (s1 + Stat{s3});
    (Stat{s1} + s3);
    (Stat{s1} + Stat{s3});
}};

// 0.5 Cross-CovarianceOption operations
constexpr auto sec0SmokeCrossCov{[] {
    // Scalar cross
    {
        Statistic<1> sFull;
        sFull.Fill(1.0);
        Statistic<1, CovarianceOption::Diagonal> sDiag;
        sDiag.Fill(2.0);

        // Copy construct
        Statistic<1, CovarianceOption::Diagonal> sDiagFromFull{sFull};
        Statistic<1> sFullFromDiag{sDiag};

        // Assign
        sDiagFromFull = sFull;
        sFullFromDiag = sDiag;

        // Merge
        sFull += sDiag;
        sDiag += sFull;

        // Combine
        (sFull + sDiag);
        (sDiag + sFull);
    }

    // Vector cross
    {
        Statistic<2> sFull;
        sFull.Fill(Eigen::Vector2d{1.0, 2.0});
        Statistic<2, CovarianceOption::Diagonal> sDiag;
        sDiag.Fill(Eigen::Vector2d{3.0, 4.0});

        // Copy construct
        Statistic<2, CovarianceOption::Diagonal> sDiagFromFull{sFull};
        Statistic<2> sFullFromDiag{sDiag};

        // Assign
        sDiagFromFull = sFull;
        sFullFromDiag = sDiag;

        // Merge
        sFull += sDiag;
        sDiag += sFull;

        // Combine
        (sFull + sDiag);
        (sDiag + sFull);
    }
}};

// 0.6 Cross-dimension-type operations (static <-> dynamic)
constexpr auto sec0SmokeCrossDim{[] {
    // Static from Dynamic
    {
        Statistic<Eigen::Dynamic> sDyn(3);
        sDyn.Fill(Eigen::Vector3d{1.0, 2.0, 3.0});

        Statistic<3> sStat{sDyn};
        sStat = sDyn;
        sStat += sDyn;
        (sStat + sDyn);
    }

    // Dynamic from Static
    {
        Statistic<3> sStat;
        sStat.Fill(Eigen::Vector3d{1.0, 2.0, 3.0});

        Statistic<Eigen::Dynamic> sDyn{sStat};
        sDyn = sStat;
        sDyn += sStat;
        (sDyn + sStat);
    }

    // Dynamic from Static (smaller dim)
    {
        Statistic<Eigen::Dynamic> sDyn(2);
        sDyn.Fill(Eigen::Vector2d{1.0, 2.0});

        Statistic<2> sStat{sDyn};
        sStat += sDyn;
        (sStat + sDyn);
    }
}};

// 0.7 StatisticPOD operations
constexpr auto sec0SmokePOD{[]<int K, CovarianceOption C>() {
    static_assert(K != Eigen::Dynamic);
    using Stat = Statistic<K, C>;
    using POD = StatisticPOD<K, C>;

    Stat s;
    if constexpr (K == 1) {
        s.Fill(5.0);
    } else {
        s.Fill(MakeSeqVector<K>(K, 1.0));
    }
    auto pod{s.ToPOD()};
    [[maybe_unused]] POD podCopy{pod};

    Stat s2;
    if constexpr (K == 1) {
        s2.Fill(6.0);
    } else {
        s2.Fill(MakeSeqVector<K>(K, 3.0));
    }
    auto pod2{s2.ToPOD()};
    (pod + pod2);

    // Base64 roundtrip
    auto b64{ToBase64(pod)};
    auto decoded{FromBase64<POD>(b64)};
    Stat s3{decoded};
}};

// =========================================================================
// Section 1: Construction (K>=2)
// =========================================================================

// K>=2 construction
constexpr auto sec1Construction{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr bool isDynamic{K == Eigen::Dynamic};

    // Static/default construction
    if constexpr (not isDynamic) {
        Stat s;
        CheckEq(s.Dimension(), K, "1: Dimension");
        CheckEq(s.SampleSize(), 0LL, "1: SampleSize");
        CheckClose(s.WeightSum(), 0.0, "1: WeightSum");
        CheckClose(s.WeightSquareSum(), 0.0, "1: WeightSquareSum");
        if constexpr (K == 1) {
            CheckClose(s.Sum(), 0.0, "1: Sum");
        } else {
            CheckClose(s.Sum(0), 0.0, "1: Sum(0)");
        }
    }

    // Dynamic construction with valid dimension
    if constexpr (isDynamic) {
        constexpr auto dim{3};
        {
            Stat s(dim);
            CheckEq(s.Dimension(), dim, "1: Dimension");
            CheckEq(s.SampleSize(), 0LL, "1: SampleSize");
            CheckClose(s.WeightSum(), 0.0, "1: WeightSum");
            CheckClose(s.WeightSquareSum(), 0.0, "1: WeightSquareSum");
            CheckClose(s.Sum(0), 0.0, "1: Sum(0)");
        }

        // dim <= 0 throws
        auto threwZero{false};
        try {
            Stat bad(0);
        } catch (const std::invalid_argument&) { threwZero = true; }
        if (not threwZero) {
            Throw<std::runtime_error>("1: dim=0 should throw std::invalid_argument");
        }

        auto threwNeg{false};
        try {
            Stat bad(-1);
        } catch (const std::invalid_argument&) { threwNeg = true; }
        if (not threwNeg) {
            Throw<std::runtime_error>("1: dim=-1 should throw std::invalid_argument");
        }
    }
}};

// =========================================================================
// Section 2: Fill and Statistics (K>=1)
// =========================================================================
// Fill and Statistics (K>=1)
// Dataset: for each v in {1,2,3,4,5}, component i = (i+1)*v
// Derived (generic for dim components):
//   Mean(i) = 3*(i+1)
//   Variance(i) = 2.5*(i+1)²
//   Sum(i) = 15*(i+1)
//   Cov(i,j) = 2.5*(i+1)*(j+1) [Full], 0 [Diagonal for i!=j]
//   CovOfMean(i,j) = 0.5*(i+1)*(j+1)
//   VarOfMean(i) = 0.5*(i+1)²
constexpr auto sec2FillAndStatistics{[]<int K, CovarianceOption C>() {
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

    CheckEq(s.SampleSize(), 5LL, "2: SampleSize");
    CheckClose(s.WeightSum(), 5.0, "2: WeightSum");
    CheckClose(s.WeightSquareSum(), 5.0, "2: WeightSquareSum");
    CheckClose(s.EffectiveSampleSize(), 5.0, "2: ESS");

    // Element-wise / scalar accessors
    if constexpr (K == 1) {
        CheckClose(s.Sum(), 15.0, "2: Sum()");
        CheckClose(s.Mean(), 3.0, "2: Mean()");
        CheckClose(s.Variance(), 2.5, "2: Variance()");
        CheckClose(s.StdDev(), std::sqrt(2.5), "2: StdDev()");
        CheckClose(s.VarianceOfMean(), 0.5, "2: VarianceOfMean()");
        CheckClose(s.StdDevOfMean(), std::sqrt(0.5), "2: StdDevOfMean()");
    } else {
        for (auto i{0}; i < dim; ++i) {
            const auto coeff{static_cast<double>(i + 1)};
            CheckClose(s.Sum(i), 15.0 * coeff,
                       fmt::format("2: Sum({})", i));
            CheckClose(s.Mean(i), 3.0 * coeff,
                       fmt::format("2: Mean({})", i));
            CheckClose(s.Variance(i), 2.5 * coeff * coeff,
                       fmt::format("2: Variance({})", i));
            CheckClose(s.StdDev(i), std::sqrt(2.5) * coeff,
                       fmt::format("2: StdDev({})", i));

            // Diagonal covariance
            CheckClose(s.Covariance(i, i), 2.5 * coeff * coeff,
                       fmt::format("2: Cov({},{})", i, i));

            // CovarianceOfMean
            CheckClose(s.VarianceOfMean(i), 0.5 * coeff * coeff,
                       fmt::format("2: VarianceOfMean({})", i));
            CheckClose(s.StdDevOfMean(i), std::sqrt(0.5) * coeff,
                       fmt::format("2: StdDevOfMean({})", i));
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
                               fmt::format("2: Cov({},{})", i, j));
                    CheckClose(s.Covariance(j, i), 2.5 * ci * cj,
                               fmt::format("2: Cov({},{})", j, i));
                    CheckClose(s.CovarianceOfMean(i, j), 0.5 * ci * cj,
                               fmt::format("2: CovarianceOfMean({},{})", i, j));
                } else {
                    CheckClose(s.Covariance(i, j), 0.0,
                               fmt::format("2: Cov({},{})", i, j));
                    CheckClose(s.Covariance(j, i), 0.0,
                               fmt::format("2: Cov({},{})", j, i));
                }
            }
        }
        // Correlation checks
        for (auto i{0}; i < dim; ++i) {
            CheckClose(s.Correlation(i, i), 1.0,
                       fmt::format("2: Corr({},{})", i, i));
            CheckClose(s.CorrelationOfMean(i, i), 1.0,
                       fmt::format("2: CorrOfMean({},{})", i, i));
            for (auto j{i + 1}; j < dim; ++j) {
                if constexpr (C == CovarianceOption::Full) {
                    CheckClose(s.Correlation(i, j), 1.0,
                               fmt::format("2: Corr({},{})", i, j));
                    CheckClose(s.CorrelationOfMean(i, j), 1.0,
                               fmt::format("2: CorrOfMean({},{})", i, j));
                } else {
                    CheckClose(s.Correlation(i, j), 0.0,
                               fmt::format("2: Corr({},{})", i, j));
                    CheckClose(s.CorrelationOfMean(i, j), 0.0,
                               fmt::format("2: CorrOfMean({},{})", i, j));
                }
            }
        }
    }

    // Bulk accessor sanity checks
    if constexpr (K != 1) {
        CheckEq(s.Mean().size(), dim, "2: Mean() size");
        CheckEq(s.Variance().size(), dim, "2: Variance() size");
        CheckEq(s.Covariance().rows(), dim, "2: Covariance() rows");
        CheckClose(s.Mean()(0), s.Mean(0), "2: Mean()(0) vs Mean(0)");
        CheckClose(s.Variance()(dim - 1), s.Variance(dim - 1),
                   "2: Variance()(last) vs Variance(last)");
    }

    // MeanEstimate
    if constexpr (K == 1) {
        const auto est{s.MeanEstimate()};
        CheckClose(est.Value(), 3.0, "2: MeanEstimate().value");
        CheckClose(est.Uncertainty(), std::sqrt(0.5), "2: MeanEstimate().uncertainty");
    } else {
        for (auto i{0}; i < dim; ++i) {
            const auto est{s.MeanEstimate(i)};
            const auto coeff{static_cast<double>(i + 1)};
            CheckClose(est.Value(), 3.0 * coeff,
                       fmt::format("2: MeanEstimate({}).value", i));
            CheckClose(est.Uncertainty(), std::sqrt(0.5) * coeff,
                       fmt::format("2: MeanEstimate({}).uncertainty", i));
        }

        // MeanEstimate vector form
        const auto estVec{s.MeanEstimate()};
        for (auto i{0}; i < dim; ++i) {
            const auto coeff{static_cast<double>(i + 1)};
            CheckClose(estVec.Value()(i), 3.0 * coeff,
                       fmt::format("2: MeanEstimate().value({})", i));
            CheckClose(estVec.Uncertainty()(i), std::sqrt(0.5) * coeff,
                       fmt::format("2: MeanEstimate().uncertainty({})", i));
            if constexpr (C == CovarianceOption::Full) {
                for (auto j{i + 1}; j < dim; ++j) {
                    const auto cj{static_cast<double>(j + 1)};
                    CheckClose(estVec.Covariance()(i, j), 0.5 * coeff * cj,
                               fmt::format("2: MeanEstimate().cov({},{})", i, j));
                }
            }
        }
    }
}};

// =========================================================================
// Section 2w: Weighted Statistics (K>=1)
// =========================================================================
// Fill with weighted observations (w = 2, 3, 1 for v = 1, 2, 3)
//   W = 6, W2 = 14, ESS = 36/14, M(i) = (i+1)*11, Mean(i) = (i+1)*11/6
//   For K=1: Var = 29/55 (analytically verified from incremental M2)
constexpr auto sec2WeightedStatistics{[]<int K, CovarianceOption C>() {
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
    CheckEq(s.SampleSize(), 3LL, "2w: SampleSize");
    CheckClose(s.WeightSum(), 6.0, "2w: WeightSum");
    CheckClose(s.WeightSquareSum(), 14.0, "2w: WeightSquareSum");
    CheckClose(s.EffectiveSampleSize(), 36.0 / 14.0, "2w: ESS");

    // Sum and Mean
    if constexpr (K == 1) {
        CheckClose(s.Sum(), 11.0, "2w: Sum");
        CheckClose(s.Mean(), 11.0 / 6.0, "2w: Mean");
    } else {
        for (auto i{0}; i < dim; ++i) {
            const auto coeff{static_cast<double>(i + 1)};
            CheckClose(s.Sum(i), 11.0 * coeff, fmt::format("2w: Sum({})", i));
            CheckClose(s.Mean(i), 11.0 / 6.0 * coeff,
                       fmt::format("2w: Mean({})", i));
        }
    }

    // Exact Variance for scalar case: Var = W/(W²-W₂) * M₂ = 6/22 * 17/6 = 17/22
    if constexpr (K == 1) {
        CheckClose(s.Variance(), 17.0 / 22.0, "2w: Variance");
    }

    // Internal consistency
    if constexpr (K != 1) {
        for (auto i{0}; i < dim; ++i) {
            CheckClose(s.Variance(i), s.Covariance(i, i),
                       fmt::format("2w: Var({}) == Cov({},{})", i, i, i));
            CheckClose(s.StdDev(i), std::sqrt(s.Variance(i)),
                       fmt::format("2w: StdDev({}) consistency", i));
            CheckClose(s.VarianceOfMean(i), s.CovarianceOfMean(i, i),
                       fmt::format("2w: VarOfMean({}) == CovOfMean({},{})", i, i, i));
            CheckClose(s.StdDevOfMean(i), std::sqrt(s.VarianceOfMean(i)),
                       fmt::format("2w: StdDevOfMean({}) consistency", i));
        }
        // Relative scaling between components: Var(1) = 4 * Var(0)
        CheckClose(s.Variance(1), 4.0 * s.Variance(0), "2w: Var(1) == 4*Var(0)");
        if constexpr (C == CovarianceOption::Full) {
            CheckClose(s.Covariance(0, 1), s.Covariance(1, 0), "2w: Cov symmetry");
            CheckClose(s.CovarianceOfMean(0, 1), s.CovarianceOfMean(1, 0),
                       "2w: CovOfMean symmetry");
        }
    }
}};

// =========================================================================
// Section 3: Merge Operations (K>=1)
// =========================================================================
// K>=1 merge
constexpr auto sec3MergeOperations{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr auto dim{isDynamic ? 2 : K};

    auto makeFullStat{[&](int startVal, int endVal) {
        Stat s{MakeStatistic<K, C>(dim)};
        if constexpr (K == 1) {
            for (auto v{startVal}; v <= endVal; ++v) {
                s.Fill(static_cast<double>(v));
            }
        } else {
            for (auto v{startVal}; v <= endVal; ++v) {
                s.Fill(v * MakeSeqVector<K>(dim, 1.0));
            }
        }
        return s;
    }};

    // Equal partition
    {
        auto s1{makeFullStat(1, 3)};
        auto s2{makeFullStat(4, 5)};
        auto sFull{makeFullStat(1, 5)};

        s1 += s2;
        CheckEq(s1.SampleSize(), sFull.SampleSize(), "3: SampleSize after merge");
        CheckClose(s1.WeightSum(), sFull.WeightSum(), "3: WeightSum after merge");
        if constexpr (K == 1) {
            CheckClose(s1.Mean(), sFull.Mean(), "3: Mean after merge");
            CheckClose(s1.Variance(), sFull.Variance(), "3: Variance after merge");
        } else {
            CheckClose(s1.Mean(0), sFull.Mean(0), "3: Mean(0) after merge");
            CheckClose(s1.Mean(dim - 1), sFull.Mean(dim - 1), "3: Mean(last) after merge");
            CheckClose(s1.Variance(0), sFull.Variance(0), "3: Variance(0) after merge");
            if constexpr (C == CovarianceOption::Full) {
                CheckClose(s1.Covariance(0, 1), sFull.Covariance(0, 1), "3: Cov(0,1) after merge");
            } else {
                CheckClose(s1.Covariance(0, 1), 0.0, "3: Cov(0,1)=0 after merge");
            }
        }
    }

    // operator+ does not mutate lhs
    {
        auto s1{makeFullStat(1, 2)};
        auto s2{makeFullStat(3, 4)};
        const auto s1CopyMean{s1.Mean()};

        auto s3{s1 + s2};

        // s1 unchanged
        if constexpr (K == 1) {
            CheckClose(s1.Mean(), s1CopyMean, "3: lhs Mean unchanged");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(s1.Mean(i), s1CopyMean[i], "3: lhs Mean unchanged");
            }
        }

        auto sFull{makeFullStat(1, 4)};
        if constexpr (K == 1) {
            CheckClose(s3.Mean(), sFull.Mean(), "3: result Mean");
        } else {
            for (auto i{0}; i < dim; ++i) {
                CheckClose(s3.Mean(i), sFull.Mean(i), "3: result Mean");
            }
        }
    }
}};

// =========================================================================
// Section 3s: Self-Merge (K>=1)
// =========================================================================
// s += s: self-merge — verifies no crash and basic invariants.
// Self-merge is not a physically meaningful operation (counting data twice)
// but the algorithm should handle it without UB.
constexpr auto secSelfMerge{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr auto dim{isDynamic ? 2 : K};

    Stat s{MakeStatistic<K, C>(dim)};
    if constexpr (K == 1) {
        s.Fill(1.0);
        s.Fill(2.0);
        s.Fill(3.0);
    } else {
        s.Fill(MakeSeqVector<K>(dim, 1.0));
        s.Fill(MakeSeqVector<K>(dim, 2.0));
        s.Fill(MakeSeqVector<K>(dim, 3.0));
    }

    // Record pre-merge state
    const auto nPrev{s.SampleSize()};
    const auto wPrev{s.WeightSum()};
    const auto w2Prev{s.WeightSquareSum()};
    const auto meanPrev{s.Mean()};
    const auto sumPrev{s.Sum()};

    // Self-merge — must not crash (no UB)
    s += s;
    CheckEq(s.SampleSize(), 2 * nPrev, "3s: SampleSize doubled");
    CheckClose(s.WeightSum(), 2.0 * wPrev, "3s: WeightSum doubled");
    CheckClose(s.WeightSquareSum(), 2.0 * w2Prev, "3s: WeightSquareSum doubled");
    CheckClose(s.Mean(), meanPrev, "3s: Mean unchanged");
    CheckClose(s.Sum(), 2 * sumPrev, "3s: Sum doubled");
    CheckClose(s.Variance(), s.Variance(), "3s: Variance is finite");
    if constexpr (K != 1) {
        CheckClose(s.Covariance(), s.Covariance(), "3s: Covariance is finite");
    }
}};

// =========================================================================
// Section 4: POD (static K only)
// =========================================================================

constexpr auto sec4POD{[]<int K, CovarianceOption C>() {
    static_assert(K != Eigen::Dynamic, "POD requires static dimension");
    using Stat = Statistic<K, C>;
    constexpr auto dim{K};

    Stat s1;
    if constexpr (K == 1) {
        for (auto v{1}; v <= 3; ++v) {
            s1.Fill(static_cast<double>(v));
        }
    } else {
        for (auto v{1}; v <= 3; ++v) {
            s1.Fill(v * MakeSeqVector<K>(dim, 1.0));
        }
    }

    auto pod{s1.ToPOD()};
    Stat s2{pod};

    CheckEq(s2.SampleSize(), s1.SampleSize(), "4: SampleSize roundtrip");
    CheckClose(s2.WeightSum(), s1.WeightSum(), "4: WeightSum roundtrip");
    if constexpr (K == 1) {
        CheckClose(s2.Mean(), s1.Mean(), "4: Mean() roundtrip");
        CheckClose(s2.Variance(), s1.Variance(), "4: Variance() roundtrip");
    } else {
        for (auto i{0}; i < dim; ++i) {
            CheckClose(s2.Mean(i), s1.Mean(i),
                       fmt::format("4: Mean({}) roundtrip", i));
            CheckClose(s2.Variance(i), s1.Variance(i),
                       fmt::format("4: Variance({}) roundtrip", i));
        }
        if constexpr (C == CovarianceOption::Full) {
            CheckClose(s2.Covariance(0, 1), s1.Covariance(0, 1), "4: Cov(0,1) roundtrip");
        }
    }
}};

// =========================================================================
// Section 5: Base64 Encoding (static K only)
// =========================================================================

// K>=1 Base64 roundtrip
constexpr auto sec5Base64Encoding{[]<int K, CovarianceOption C>() {
    static_assert(K != Eigen::Dynamic, "Base64 requires static dimension");
    using Stat = Statistic<K, C>;
    using POD = StatisticPOD<K, C>;

    Stat s1;
    if constexpr (K == 1) {
        for (auto v{1}; v <= 3; ++v) {
            s1.Fill(static_cast<double>(v));
        }
    } else {
        for (auto v{1}; v <= 3; ++v) {
            s1.Fill(v * MakeSeqVector<K>(K, 1.0));
        }
    }

    auto pod{s1.ToPOD()};
    auto b64{ToBase64(pod)};
    auto decoded{FromBase64<POD>(b64)};
    Stat s2{decoded};

    if constexpr (K == 1) {
        CheckClose(s2.Mean(), s1.Mean(), "5: Mean() Base64 roundtrip");
        CheckClose(s2.Variance(), s1.Variance(), "5: Variance() Base64 roundtrip");
    } else {
        CheckClose(s2.Mean(0), s1.Mean(0), "5: Mean(0) Base64 roundtrip");
        CheckClose(s2.Variance(0), s1.Variance(0), "5: Variance(0) Base64 roundtrip");
        if constexpr (C == CovarianceOption::Full) {
            CheckClose(s2.Covariance(0, 1), s1.Covariance(0, 1), "5: Cov(0,1) Base64 roundtrip");
        }
    }
}};

// Invalid Base64 string throws
constexpr auto sec6InvalidBase64{[] {
    StatisticPOD<1> data;
    auto threw{false};
    try {
        data = FromBase64<StatisticPOD<1>>("not_valid_base64!!!");
    } catch (const std::runtime_error&) {
        threw = true;
    }
    if (not threw) {
        Throw<std::runtime_error>("5: invalid Base64 should throw std::runtime_error");
    }
}};

// StatisticPOD operator+
constexpr auto sec6PODAdd{[] {
    Statistic<1> s1;
    s1.Fill(1.0);
    s1.Fill(2.0);
    Statistic<1> s2;
    s2.Fill(3.0);
    s2.Fill(4.0);

    auto pod1{s1.ToPOD()};
    auto pod2{s2.ToPOD()};
    auto pod3{pod1 + pod2};

    Statistic<1> sFull;
    sFull.Fill(1.0);
    sFull.Fill(2.0);
    sFull.Fill(3.0);
    sFull.Fill(4.0);

    Statistic<1> s3{pod3};
    CheckClose(s3.Mean(), sFull.Mean(), "5: Mean from StatisticPOD::operator+");
    CheckClose(s3.Variance(), sFull.Variance(), "5: Variance from StatisticPOD::operator+");
}};

// =========================================================================
// Section 6: Edge Cases and Numerical Stress
// =========================================================================

// K>=1 edge cases
constexpr auto sec6EdgeCases{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr auto dim{isDynamic ? 2 : K};

    // Construction helper
    auto makeStat{[&] {
        if constexpr (isDynamic) {
            return Stat(dim);
        } else {
            return Stat{};
        }
    }};

    // Fill helpers
    auto fillConst{[&](Stat& s, double val, double w = 1.0) {
        if constexpr (K == 1) {
            s.Fill(val, w);
        } else {
            s.Fill(MakeConstVector<K>(dim, val), w);
        }
    }};
    auto fillSeq{[&](Stat& s, double start = 1.0, double w = 1.0) {
        if constexpr (K == 1) {
            s.Fill(start, w);
        } else {
            s.Fill(MakeSeqVector<K>(dim, start), w);
        }
    }};
    auto checkMean{[&](const Stat& s, double expected, const std::string& ctx) {
        if constexpr (K == 1) {
            CheckClose(s.Mean(), expected, ctx);
        } else {
            CheckClose(s.Mean(0), expected, ctx);
        }
    }};
    auto checkVariance{[&](const Stat& s, double expected, const std::string& ctx) {
        if constexpr (K == 1) {
            CheckClose(s.Variance(), expected, ctx);
        } else {
            CheckClose(s.Variance(0), expected, ctx);
        }
    }};

    // Zero-weight fill is a no-op
    {
        Stat s{makeStat()};
        fillConst(s, 1.0, 0.0);
        CheckEq(s.SampleSize(), 0LL, "6: SampleSize (zero weight)");
        CheckClose(s.WeightSum(), 0.0, "6: WeightSum (zero weight)");

        // After a real fill, zero-weight should not change anything
        fillSeq(s);
        const auto n{s.SampleSize()};
        const auto w{s.WeightSum()};

        fillConst(s, 1.0, 0.0);
        CheckEq(s.SampleSize(), n, "6: SampleSize unchanged by zero weight");
        CheckClose(s.WeightSum(), w, "6: WeightSum unchanged by zero weight");
    }

    // Single sample (variance is NaN)
    {
        Stat s{makeStat()};
        fillSeq(s);
        CheckEq(s.SampleSize(), 1LL, "6: SampleSize");

        // CovCoeff = 1/(1-1) = inf, m2=0 => Variance = inf*0 = NaN
        if constexpr (K == 1) {
            if (not std::isnan(s.Variance())) {
                Throw<std::runtime_error>(fmt::format("6: Variance() should be NaN for single sample, got {}", s.Variance()));
            }
        } else {
            if (not std::isnan(s.Variance(0))) {
                Throw<std::runtime_error>(fmt::format("6: Variance(0) should be NaN for single sample, got {}", s.Variance(0)));
            }
        }
    }

    // Identical values (zero variance)
    {
        Stat s{makeStat()};
        for (auto j{0}; j < 10; ++j) {
            fillSeq(s);
        }
        checkVariance(s, 0.0, "6: Variance (identical values)");
    }

    // Large sample count
    {
        Stat s{makeStat()};
        constexpr long long kN{10'000};
        for (auto j{0}; j < kN; ++j) {
            fillConst(s, 1.0);
        }
        CheckEq(s.SampleSize(), kN, "6: SampleSize");
        checkMean(s, 1.0, "6: Mean");
        checkVariance(s, 0.0, "6: Variance");
    }

    // Empty statistic accessors
    if constexpr (std::numeric_limits<double>::is_iec559) {
        Stat s{makeStat()};
        if constexpr (K == 1) {
            if (not std::isnan(s.Mean())) {
                Throw<std::runtime_error>("6: Mean() of empty statistic should be NaN under IEEE 754");
            }
        } else {
            if (not std::isnan(s.Mean(0))) {
                Throw<std::runtime_error>("6: Mean(0) of empty statistic should be NaN under IEEE 754");
            }
        }
    }
}};

// Negative weights (K>=1)
constexpr auto sec7NegativeWeightVectors{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr auto dim{isDynamic ? 2 : K};

    // Fill helpers
    auto fillSeq{[&](Stat& s, double multiplier, double w = 1.0) {
        if constexpr (K == 1) {
            s.Fill(multiplier, w);
        } else {
            s.Fill(multiplier * MakeSeqVector<K>(dim, 1.0), w);
        }
    }};

    // Negative weight fill
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillSeq(s, 1.0, 5.0);
        fillSeq(s, 2.0, -2.0);

        CheckEq(s.SampleSize(), 2LL, "7: SampleSize (neg weight)");
        CheckClose(s.WeightSum(), 3.0, "7: WeightSum (5 + (-2))");
        CheckClose(s.WeightSquareSum(), 29.0, "7: WeightSquareSum (25 + 4)");
        // m[i] = 5*(i+1) + (-2)*2*(i+1) = (i+1)
        if constexpr (K == 1) {
            CheckClose(s.Sum(), 1.0, "7: Sum()");
            CheckClose(s.Mean(), 1.0 / 3.0, "7: Mean()");
        } else {
            for (auto i{0}; i < dim; ++i) {
                const auto coeff{static_cast<double>(i + 1)};
                CheckClose(s.Sum(i), coeff, fmt::format("7: Sum({})", i));
                CheckClose(s.Mean(i), coeff / 3.0, fmt::format("7: Mean({})", i));
            }
        }
    }

    // Mixed positive/negative weights
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillSeq(s, 1.0, 2.0);  // w=2
        fillSeq(s, 3.0, -1.0); // w=-1
        fillSeq(s, 5.0, 1.0);  // w=1

        CheckEq(s.SampleSize(), 3LL, "7: SampleSize (mixed signs)");
        CheckClose(s.WeightSum(), 2.0, "7: WeightSum (mixed signs)");
        CheckClose(s.WeightSquareSum(), 6.0, "7: WeightSquareSum (4+1+1)");
        // m[i] = 2*(i+1)*1 + (-1)*(i+1)*3 + 1*(i+1)*5 = (i+1)*(2 -3 +5) = 4*(i+1)
        if constexpr (K == 1) {
            CheckClose(s.Sum(), 4.0, "7: Sum() mixed");
            CheckClose(s.Mean(), 2.0, "7: Mean() mixed");
            // Should not crash
            s.Variance();
            s.StdDev();
        } else {
            for (auto i{0}; i < dim; ++i) {
                const auto coeff{static_cast<double>(i + 1)};
                CheckClose(s.Sum(i), 4.0 * coeff, fmt::format("7: Sum({}) mixed", i));
                CheckClose(s.Mean(i), 2.0 * coeff, fmt::format("7: Mean({}) mixed", i));
            }
            // Should not crash
            s.Variance(0);
            s.StdDev(0);
        }
    }

    // Weights summing to zero (cancellation)
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillSeq(s, 1.0, 1.0);
        fillSeq(s, 2.0, -1.0);

        CheckEq(s.SampleSize(), 2LL, "7: SampleSize (cancellation)");
        CheckClose(s.WeightSum(), 0.0, "7: WeightSum (cancellation)");
        CheckClose(s.WeightSquareSum(), 2.0, "7: WeightSquareSum (cancellation)");
        if constexpr (K == 1) {
            CheckClose(s.Variance(), 0.0, "7: Variance() (weight cancellation)");
        } else {
            CheckClose(s.Variance(0), 0.0, "7: Variance(0) (weight cancellation)");
        }

        if constexpr (std::numeric_limits<double>::is_iec559) {
            if constexpr (K == 1) {
                if (not std::isinf(s.Mean())) {
                    Throw<std::runtime_error>("7: Mean() with w=0 should be inf under IEEE 754");
                }
            } else {
                if (not std::isinf(s.Mean(0))) {
                    Throw<std::runtime_error>("7: Mean(0) with w=0 should be inf under IEEE 754");
                }
            }
        }
    }

    // Negative weight alone on single sample
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillSeq(s, 5.0, -1.0);
        CheckEq(s.SampleSize(), 1LL, "7: SampleSize (single neg)");
        CheckClose(s.WeightSum(), -1.0, "7: WeightSum (single neg)");
        CheckClose(s.WeightSquareSum(), 1.0, "7: WeightSquareSum (single neg)");
        if constexpr (K == 1) {
            CheckClose(s.Sum(), -5.0, "7: Sum() single neg");
            CheckClose(s.Mean(), 5.0, "7: Mean() single neg");
        } else {
            for (auto i{0}; i < dim; ++i) {
                const auto coeff{static_cast<double>(i + 1)};
                CheckClose(s.Sum(i), -5.0 * coeff, fmt::format("7: Sum({}) single neg", i));
                CheckClose(s.Mean(i), 5.0 * coeff, fmt::format("7: Mean({}) single neg", i));
            }
        }
    }
}};

// Numerical stress (K>=1)
constexpr auto sec8NumericalStressVectors{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr auto dim{isDynamic ? 2 : K};

    // Fill helper
    auto fillConst{[&](Stat& s, double val, double w = 1.0) {
        if constexpr (K == 1) {
            s.Fill(val, w);
        } else {
            s.Fill(MakeConstVector<K>(dim, val), w);
        }
    }};
    auto checkMean{[&](const Stat& s, double expected, const std::string& ctx) {
        if constexpr (K == 1) {
            CheckClose(s.Mean(), expected, ctx);
        } else {
            CheckClose(s.Mean(0), expected, ctx);
        }
    }};
    auto checkVariance{[&](const Stat& s, double expected, const std::string& ctx) {
        if constexpr (K == 1) {
            CheckClose(s.Variance(), expected, ctx);
        } else {
            CheckClose(s.Variance(0), expected, ctx);
        }
    }};

    // Very large values (~1e15)
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillConst(s, 1.0e15);
        fillConst(s, 1.0e15 + 1.0);
        fillConst(s, 1.0e15 + 2.0);

        checkMean(s, 1.0e15 + 1.0, "8: Mean (large)");
        // Variance for offset {0,1,2}: m2=2, CovCoeff=3/(9-3)=1/2, Var=1
        checkVariance(s, 1.0, "8: Variance (large)");
    }

    // Very small values (~1e-15)
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillConst(s, 1.0e-15);
        fillConst(s, 2.0e-15);
        fillConst(s, 3.0e-15);

        checkMean(s, 2.0e-15, "8: Mean (small)");
        checkVariance(s, 1.0e-30, "8: Variance (small)");
    }

    // Alternating large positive/negative values
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillConst(s, 1.0e10);
        fillConst(s, -1.0e10);
        fillConst(s, 1.0e10);
        fillConst(s, -1.0e10);

        checkMean(s, 0.0, "8: Mean (alternating)");
        checkVariance(s, 4.0e20 / 3.0, "8: Variance (alternating)");
    }

    // Values with large mean offset
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillConst(s, 1.0e9 + 1.0);
        fillConst(s, 1.0e9 + 2.0);
        fillConst(s, 1.0e9 + 3.0);

        checkMean(s, 1.0e9 + 2.0, "8: Mean (offset)");
        checkVariance(s, 1.0, "8: Variance (offset)");
    }

    // Very small weights
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillConst(s, 5.0, 1.0e-300);

        CheckEq(s.SampleSize(), 1LL, "8: SampleSize (tiny weight)");
        CheckClose(s.WeightSum(), 1.0e-300, "8: WeightSum (tiny weight)");
        checkMean(s, 5.0, "8: Mean (tiny weight)");
    }

    // Single large negative weight
    {
        Stat s{MakeStatistic<K, C>(dim)};
        fillConst(s, 1.0, -1.0e15);
        CheckEq(s.SampleSize(), 1LL, "8: SampleSize (large neg)");
        CheckClose(s.WeightSum(), -1.0e15, "8: WeightSum (large neg)");
        CheckClose(s.WeightSquareSum(), 1.0e30, "8: WeightSquareSum (large neg)");
        checkMean(s, 1.0, "8: Mean (large neg)");
    }
}};

// =========================================================================
// Section 6x: Dimension Mismatch (dynamic dimension only)
// =========================================================================
constexpr auto secDimMismatch{[] {
    // operator+= with mismatched dynamic dimensions
    // Use Statistic<Eigen::Dynamic, Full> explicitly
    {
        Statistic<Eigen::Dynamic, CovarianceOption::Full> s1(3);
        Statistic<Eigen::Dynamic, CovarianceOption::Full> s2(5);
        auto threw{false};
        try {
            s1 += s2;
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        if (not threw) {
            Throw<std::runtime_error>("dim: mismatch operator+= should throw");
        }
    }
    // operator+ with mismatched dynamic dimensions
    {
        Statistic<Eigen::Dynamic> s1(3);
        Statistic<Eigen::Dynamic> s2(5);
        auto threw{false};
        try {
            (void)(s1 + s2);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        if (not threw) {
            Throw<std::runtime_error>("dim: mismatch operator+ should throw");
        }
    }
    // Dynamic-dynamic assignment with different dimensions should succeed
    // (dynamic dimensions resize to match source via CopyFrom)
    {
        Statistic<Eigen::Dynamic> s1(3);
        Statistic<Eigen::Dynamic> s2(5);
        s1.Fill(Eigen::VectorXd::Ones(3));
        s2.Fill(Eigen::VectorXd::Ones(5));
        s1 = s2;
        CheckEq(s1.Dimension(), 5, "dim: dynamic assign resizes dimension");
        CheckEq(s1.SampleSize(), s2.SampleSize(), "dim: dynamic assign copies data");
    }
    // Static-from-dynamic with mismatched dimension should throw
    {
        Statistic<Eigen::Dynamic> sDyn(5);
        sDyn.Fill(Eigen::VectorXd::Ones(5));
        auto threw{false};
        try {
            Statistic<3> sStat(sDyn);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        if (not threw) {
            Throw<std::runtime_error>("dim: static-from-dynamic mismatch should throw");
        }
    }
}};

// =========================================================================
// Section 6x: Self-Assignment (K>=1)
// =========================================================================
constexpr auto secSelfAssignment{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr auto dim{isDynamic ? 2 : K};

    Stat s{MakeStatistic<K, C>(dim)};
    if constexpr (K == 1) {
        s.Fill(1.0);
        s.Fill(2.0);
    } else {
        s.Fill(MakeSeqVector<K>(dim, 1.0));
        s.Fill(MakeSeqVector<K>(dim, 2.0));
    }

    const auto n{s.SampleSize()};
    const auto w{s.WeightSum()};

    // Self copy-assign
    auto& sRef{s};
    s = sRef;
    CheckEq(s.SampleSize(), n, "sa: SampleSize after self copy-assign");
    CheckClose(s.WeightSum(), w, "sa: WeightSum after self copy-assign");

    // Self move-assign
    s = std::move(sRef);
    CheckEq(s.SampleSize(), n, "sa: SampleSize after self move-assign");
    CheckClose(s.WeightSum(), w, "sa: WeightSum after self move-assign");
}};

// =========================================================================
// Section 6x: Post-Move State (K>=1)
// =========================================================================
constexpr auto secPostMove{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr auto dim{isDynamic ? 2 : K};

    Stat s1{MakeStatistic<K, C>(dim)};
    if constexpr (K == 1) {
        s1.Fill(5.0);
    } else {
        s1.Fill(MakeSeqVector<K>(dim, 5.0));
    }

    const auto n{s1.SampleSize()};
    const auto w{s1.WeightSum()};

    // Move construct
    Stat s2{std::move(s1)};
    CheckEq(s2.SampleSize(), n, "pm: SampleSize after move");
    CheckClose(s2.WeightSum(), w, "pm: WeightSum after move");
    // s1 is in moved-from state — must be safely destructible (verified when
    // s1 goes out of scope at the end of this block).

    // Move assignment
    Stat s3{MakeStatistic<K, C>(dim)};
    s3 = std::move(s2);
    CheckEq(s3.SampleSize(), n, "pm: SampleSize after move assign");
    CheckClose(s3.WeightSum(), w, "pm: WeightSum after move assign");
    // s2 is in moved-from state — must be safely destructible.
}};

// =========================================================================
// Section 8: Cross-Verification — Basic Consistency
// =========================================================================

// N=1 component of Statistic<2> matches Statistic<1>
constexpr auto sec8ComponentConsistency{[] {
    Statistic<1> s1;
    Statistic<2> s2;
    const std::array values{2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0};
    for (auto v : values) {
        s1.Fill(v);
        s2.Fill(Eigen::Vector2d{v, 0.0});
    }

    CheckClose(s2.Mean(0), s1.Mean(), "10: Mean(0) matches scalar Statistic");
    CheckClose(s2.Variance(0), s1.Variance(), "10: Variance(0) matches scalar Statistic");
    CheckClose(s2.Mean(1), 0.0, "10: Mean(1) is zero");
    CheckClose(s2.Variance(1), 0.0, "10: Variance(1) is zero");
}};

// Partition consistency (K>=1)
constexpr auto sec9PartitionConsistency{[]<int K, CovarianceOption C>() {
    using Stat = Statistic<K, C>;
    constexpr bool isDynamic{K == Eigen::Dynamic};
    constexpr auto dim{isDynamic ? 2 : K};

    auto fillStat{[&](Stat& s, int startVal, int endVal) {
        if constexpr (K == 1) {
            for (auto v{startVal}; v <= endVal; ++v) {
                s.Fill(static_cast<double>(v));
            }
        } else {
            for (auto v{startVal}; v <= endVal; ++v) {
                s.Fill(v * MakeSeqVector<K>(dim, 1.0));
            }
        }
    }};

    Stat sFull{MakeStatistic<K, C>(dim)};
    fillStat(sFull, 1, 6);

    Stat sPart{MakeStatistic<K, C>(dim)};
    fillStat(sPart, 1, 3);
    Stat sOther{MakeStatistic<K, C>(dim)};
    fillStat(sOther, 4, 6);

    sPart += sOther;

    if constexpr (K == 1) {
        CheckClose(sPart.Mean(), sFull.Mean(), "9: partition consistency Mean()");
        CheckClose(sPart.Variance(), sFull.Variance(), "9: partition consistency Variance()");
    } else {
        CheckClose(sPart.Mean(0), sFull.Mean(0), "9: partition consistency Mean(0)");
        CheckClose(sPart.Variance(0), sFull.Variance(0), "9: partition consistency Variance(0)");
    }
    CheckEq(sPart.SampleSize(), sFull.SampleSize(), "9: partition consistency SampleSize");
}};

// =========================================================================
// Section 9: Cross-Verification — CovarianceOption
// =========================================================================

// Cross-CovarianceOption copy construction
constexpr auto sec9CrossCovCopy{[] {
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
        CheckClose(sDiag.Mean(), sFull.Mean(), "9: Diag from Full Mean");
        CheckClose(sDiag.Variance(), sFull.Variance(), "9: Diag from Full Variance");
        CheckEq(sDiag.SampleSize(), sFull.SampleSize(), "9: Diag from Full SampleSize");

        // Full from Diagonal
        Statistic<1> sFull2{sDiag};
        CheckClose(sFull2.Mean(), sFull.Mean(), "9: Full from Diag Mean");
        CheckClose(sFull2.Variance(), sFull.Variance(), "9: Full from Diag Variance");
        CheckEq(sFull2.SampleSize(), sFull.SampleSize(), "9: Full from Diag SampleSize");
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
        CheckClose(sDiag.Mean(), sFull.Mean(), "9: Diag from Full Mean");
        CheckClose(sDiag.Variance(), sFull.Variance(), "9: Diag from Full Variance");
        CheckClose(sDiag.Covariance(0, 1), 0.0, "9: Diag from Full Cov(0,1)=0");
        CheckEq(sDiag.SampleSize(), sFull.SampleSize(), "9: Diag from Full SampleSize");

        // Full from Diagonal
        Statistic<2> sFull2{sDiag};
        CheckClose(sFull2.Mean(), sFull.Mean(), "9: Full from Diag Mean");
        CheckClose(sFull2.Variance(), sFull.Variance(), "9: Full from Diag Variance");
        CheckClose(sFull2.Covariance(0, 1), 0.0, "9: Full from Diag Cov(0,1)=0");
    }
}};

// Cross-CovarianceOption assignment (operator=)
constexpr auto sec9CrossCovAssign{[] {
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
    CheckClose(sFull.Mean(), sDiagSrc.Mean(), "9: Full=Diag Mean");
    CheckClose(sFull.Variance(), sDiagSrc.Variance(), "9: Full=Diag Variance");
    CheckClose(sFull.Covariance(0, 1), 0.0, "9: Full=Diag Cov(0,1)=0");

    // Diagonal = Full
    Statistic<2, CovarianceOption::Diagonal> sDiag;
    sDiag = sFullSrc;
    CheckClose(sDiag.Mean(), sFullSrc.Mean(), "9: Diag=Full Mean");
    CheckClose(sDiag.Variance(), sFullSrc.Variance(), "9: Diag=Full Variance");
    CheckClose(sDiag.Covariance(0, 1), 0.0, "9: Diag=Full Cov(0,1)=0");

    // Scalar: Full = Diagonal
    Statistic<1> s1Full;
    Statistic<1, CovarianceOption::Diagonal> s1Diag;
    s1Diag.Fill(3.0);
    s1Diag.Fill(7.0);
    s1Full = s1Diag;
    CheckClose(s1Full.Mean(), s1Diag.Mean(), "9: scalar Full=Diag Mean");
}};

// Cross-CovarianceOption merge (operator+=)
constexpr auto sec9CrossCovMerge{[] {
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
        CheckClose(sFull.Mean(), sRef.Mean(), "9: Full+=Diag Mean");
        CheckClose(sFull.Variance(), sRef.Variance(), "9: Full+=Diag Variance");
        // Diagonal's within-dataset off-diagonal M2 is not stored, so merged
        // Cov(0,1) = (M2_Full(0,1)=4 + 0 + between=15) / (5-1) = 4.75
        CheckClose(sFull.Covariance(0, 1), 4.75, "9: Full+=Diag Cov(0,1)");
        CheckEq(sFull.SampleSize(), sRef.SampleSize(), "9: Full+=Diag SampleSize");
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
        CheckClose(sDiag.Mean(), sRef.Mean(), "9: Diag+=Full Mean");
        CheckClose(sDiag.Variance(), sRef.Variance(), "9: Diag+=Full Variance");
        CheckClose(sDiag.Covariance(0, 1), 0.0, "9: Diag+=Full Cov(0,1)=0");
    }
}};

// Cross-CovarianceOption operator+
constexpr auto sec9CrossCovAdd{[] {
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

        CheckClose(sResult.Mean(), sRef.Mean(), "9: Full+Diag Mean");
        CheckClose(sResult.Variance(), sRef.Variance(), "9: Full+Diag Variance");
        // Diagonal's within-dataset off-diagonal M2 is not stored, so merged
        // Cov(0,1) = (M2_Full(0,1)=1 + 0 + between=15) / (5-1) = 4.0
        CheckClose(sResult.Covariance(0, 1), 4.0, "9: Full+Diag Cov(0,1)");
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

        CheckClose(sResult.Mean(), sRef.Mean(), "9: Diag+Full Mean");
        CheckClose(sResult.Variance(), sRef.Variance(), "9: Diag+Full Variance");
        // When converting Diagonal to Full for the result, off-diagonal is zero;
        // merged Cov(0,1) = (0 + M2_Full(0,1)=4 + between=15) / (5-1) = 4.75
        CheckClose(sResult.Covariance(0, 1), 4.75, "9: Diag+Full Cov(0,1)");
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

        CheckClose(sResult.Mean(), sRef.Mean(), "9: scalar Full+Diag Mean");
        CheckClose(sResult.Variance(), sRef.Variance(), "9: scalar Full+Diag Variance");
    }
}};

// =========================================================================
// Section 10: Cross-Verification — Dimension Type
// =========================================================================

// Cross-dimension-type copy (Static ↔ Dynamic)
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

// Cross-dimension-type merge (Static ↔ Dynamic)
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
// Section 10x: Cross-Verification — Rvalue Overloads
// =========================================================================

// Cross-CovarianceOption operator+ with rvalue arguments
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

// Cross-dimension-type operator+ with rvalue arguments
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

// =========================================================================
// Section 9b: Correlation — zero-variance edge case
// =========================================================================
constexpr auto sec9bCorrZeroVar{[]<int K, CovarianceOption C>() {
    constexpr auto dim{(K == Eigen::Dynamic) ? 2 : K};
    constexpr bool isFull{C == CovarianceOption::Full};

    if constexpr (isFull and K != 1) {
        using Stat = Statistic<K, C>;
        Stat s{MakeStatistic<K, C>(dim)};
        // Feed the same observation twice → zero variance for all components
        auto v{MakeSeqVector<K>(dim, 1.0)};
        s.Fill(v);
        s.Fill(v);

        CheckClose(s.Correlation(0, 0), 1.0, "9b: Corr(0,0)=1 even Var=0");
        if constexpr (dim >= 2) {
            if (not std::isnan(s.Correlation(0, 1))) {
                Throw<std::runtime_error>("9b: Corr(0,1) should be NaN when Var=0");
            }
            if (not std::isnan(s.CorrelationOfMean(0, 1))) {
                Throw<std::runtime_error>("9b: CorrOfMean(0,1) should be NaN when Var=0");
            }
            CheckClose(s.Correlation(1, 1), 1.0, "9b: Corr(1,1)=1");
            CheckClose(s.CorrelationOfMean(0, 0), 1.0, "9b: CorrOfMean(0,0)=1");
            CheckClose(s.CorrelationOfMean(1, 1), 1.0, "9b: CorrOfMean(1,1)=1");
        }
    }
}};

} // namespace TestStatisticSection

// =========================================================================
// Main
// =========================================================================

auto TestStatistic::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestStatisticSection;

    // =====================================================================
    // Section 0: Smoke (compile check) — call every API, no result checking
    // =====================================================================
    PrintLn("--- Section 0: Smoke (compile check) ---");
    RunOverStaticDims<AllStaticDims>(sec0SmokeStatic);
    PrintLn("  0 passed: Statistic<K> (K=1,2,3,5,10 Full/Diag)");
    RunOverDynamic(sec0SmokeDynamic);
    PrintLn("  0 passed: Statistic<Dynamic> (Dynamic Full/Diag)");
    sec0SmokeCrossCov();
    PrintLn("  0 passed: cross-CovarianceOption");
    sec0SmokeCrossDim();
    PrintLn("  0 passed: cross-dimension-type");
    RunOverStaticDims<AllStaticDims>(sec0SmokePOD);
    PrintLn("  0 passed: StatisticPOD (K=1,2,3,5,10 Full/Diag)");

    // =====================================================================
    // Section 1: Construction (K>=1)
    // =====================================================================
    PrintLn("--- Section 1: Construction (K>=1) ---");
    RunOverAllDims<AllStaticDims>(sec1Construction);
    PrintLn("  1 passed: construction (K=1,2,3,5,10 Full/Diag + dynamic)");

    // =====================================================================
    // Section 2: Fill and Statistics (K>=1)
    // =====================================================================
    PrintLn("--- Section 2: Fill and Statistics (K>=1) ---");
    RunOverAllDims<AllStaticDims>(sec2FillAndStatistics);
    PrintLn("  2 passed: fill and statistics (K=1,2,3,5,10 Full/Diag + dynamic)");
    RunOverAllDims<AllStaticDims>(sec2WeightedStatistics);
    PrintLn("  2 passed: weighted statistics (K=1,2,3,5,10 Full/Diag + dynamic)");

    // =====================================================================
    // Section 3: Merge Operations (K>=1)
    // =====================================================================
    PrintLn("--- Section 3: Merge Operations (K>=1) ---");
    RunOverAllDims<AllStaticDims>(sec3MergeOperations);
    PrintLn("  3 passed: merge operations (K=1,2,3,5,10 Full/Diag + dynamic)");
    RunOverAllDims<AllStaticDims>(secSelfMerge);
    PrintLn("  3 passed: self-merge (K=1,2,3,5,10 Full/Diag + dynamic)");

    // =====================================================================
    // Section 4: POD
    // =====================================================================
    PrintLn("--- Section 4: ToPOD/FromPOD ---");
    RunOverStaticDims<AllStaticDims>(sec4POD);
    PrintLn("  4 passed: ToPOD/FromPOD Full/Diag (K=1,2,3,5,10)");

    // =====================================================================
    // Section 5: Base64 Encoding
    // =====================================================================
    PrintLn("--- Section 5: Base64 Encoding ---");
    RunOverStaticDims<AllStaticDims>(sec5Base64Encoding);
    PrintLn("  5 passed: Base64 Full/Diag (K=1,2,3,5,10)");
    sec6InvalidBase64();
    PrintLn("  5 passed: invalid Base64 throws");
    sec6PODAdd();
    PrintLn("  5 passed: StatisticPOD operator+");

    // =====================================================================
    // Section 6: Edge Cases (K>=1)
    // =====================================================================
    PrintLn("--- Section 6: Edge Cases (K>=1) ---");
    RunOverAllDims<AllStaticDims>(sec6EdgeCases);
    PrintLn("  6 passed: edge cases (K=1,2,3,5,10 Full/Diag + dynamic)");

    // =====================================================================
    // Section 7: Negative Weights (K>=1)
    // =====================================================================
    PrintLn("--- Section 7: Negative Weights (K>=1) ---");
    RunOverAllDims<AllStaticDims>(sec7NegativeWeightVectors);
    PrintLn("  7 passed: negative weights (K=1,2,3,5,10 Full/Diag + dynamic)");

    // =====================================================================
    // Section 8: Numerical Stress (K>=1)
    // =====================================================================
    PrintLn("--- Section 8: Numerical Stress (K>=1) ---");
    RunOverAllDims<AllStaticDims>(sec8NumericalStressVectors);
    PrintLn("  8 passed: numerical stress (K=1,2,3,5,10 Full/Diag + dynamic)");
    secDimMismatch();
    PrintLn("  8 passed: dimension mismatch");
    RunOverAllDims<AllStaticDims>(secSelfAssignment);
    PrintLn("  8 passed: self-assignment (K=1,2,3,5,10 Full/Diag + dynamic)");
    RunOverAllDims<AllStaticDims>(secPostMove);
    PrintLn("  8 passed: post-move state (K=1,2,3,5,10 Full/Diag + dynamic)");

    // =====================================================================
    // Section 9: Partition Consistency (K>=1)
    // =====================================================================
    PrintLn("--- Section 9: Partition Consistency (K>=1) ---");
    RunOverAllDims<AllStaticDims>(sec9PartitionConsistency);
    PrintLn("  9 passed: partition consistency (K=1,2,3,5,10 Full/Diag + dynamic)");
    RunOverAllDims<AllStaticDims>(sec9bCorrZeroVar);
    PrintLn("  9 passed: correlation zero-variance edge case (K=2,3,5,10 Full + dynamic)");

    // =====================================================================
    // Section 10: Cross-Verification
    // =====================================================================
    PrintLn("--- Section 10: Cross-Verification ---");
    sec8ComponentConsistency();
    PrintLn("  10 passed: component consistency (K=1 vs K=2)");
    sec9CrossCovCopy();
    PrintLn("  10 passed: cross-CovarianceOption copy construction");
    sec9CrossCovAssign();
    PrintLn("  10 passed: cross-CovarianceOption assignment");
    sec9CrossCovMerge();
    PrintLn("  10 passed: cross-CovarianceOption merge");
    sec9CrossCovAdd();
    PrintLn("  10 passed: cross-CovarianceOption operator+");
    sec10CrossDimCopy();
    PrintLn("  10 passed: cross-dimension-type copy");
    sec10CrossDimMerge();
    PrintLn("  10 passed: cross-dimension-type merge");
    sec10CrossCovRvalueAdd();
    PrintLn("  10 passed: cross-CovarianceOption rvalue operator+");
    sec10CrossDimRvalueAdd();
    PrintLn("  10 passed: cross-dimension-type rvalue operator+");

    // =====================================================================
    // All tests passed
    // =====================================================================

    PrintLn("All TestStatistic tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
