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

namespace Mustard::inline Math {

// ===========================================================================
// Welch's t-test
// ===========================================================================

template<CovarianceOption C>
StatisticalTTest::StatisticalTTest(const Statistic<1, C>& stat1, const Statistic<1, C>& stat2) :
    StatisticalTTest{stat1.MeanEstimate(), stat1.EffectiveSampleSize(),
                     stat2.MeanEstimate(), stat2.EffectiveSampleSize()} {}

template<CovarianceOption C>
StatisticalTTest::StatisticalTTest(const Estimate<1, C>& est1, double n1, const Estimate<1, C>& est2, double n2) :
    UnivariateStatisticalTest{},
    fNDF{},
    fT{} {
    if (n1 <= 1 or n2 <= 1) {
        Throw<std::invalid_argument>(fmt::format("Sample sizes must be greater than 1, got n1={} and n2={}", n1, n2));
    }
    const auto delta{est1 - est2};
    fNDF = muc::pow(delta.Variance(), 2) /
           (muc::pow(est1.Variance(), 2) / (n1 - 1) +
            muc::pow(est2.Variance(), 2) / (n2 - 1));
    fT = delta.Value() / delta.StdDev();
}

template<CovarianceOption C>
StatisticalTTest::StatisticalTTest(const Statistic<1, C>& stat, double mu) :
    StatisticalTTest{stat.MeanEstimate(), stat.EffectiveSampleSize(), mu} {}

template<CovarianceOption C>
StatisticalTTest::StatisticalTTest(const Estimate<1, C>& est, double n, double mu) :
    UnivariateStatisticalTest{},
    fNDF{},
    fT{} {
    if (n <= 1) {
        Throw<std::invalid_argument>(fmt::format("Sample size must be greater than 1, got n={}", n));
    }
    const auto delta{est - mu};
    fNDF = n - 1;
    fT = delta.Value() / delta.StdDev();
}

// ===========================================================================
// Z-test
// ===========================================================================

template<CovarianceOption C>
StatisticalZTest::StatisticalZTest(const Statistic<1, C>& stat1, const Statistic<1, C>& stat2) :
    StatisticalZTest{stat1.MeanEstimate(), stat2.MeanEstimate()} {}

template<CovarianceOption C>
StatisticalZTest::StatisticalZTest(const Estimate<1, C>& est1, const Estimate<1, C>& est2) :
    UnivariateStatisticalTest{},
    fZ{} {
    const auto delta{est1 - est2};
    fZ = delta.Value() / delta.StdDev();
}

template<CovarianceOption C>
StatisticalZTest::StatisticalZTest(const Statistic<1, C>& stat, double mu) :
    StatisticalZTest{stat.MeanEstimate(), mu} {}

template<CovarianceOption C>
StatisticalZTest::StatisticalZTest(const Estimate<1, C>& est, double mu) :
    UnivariateStatisticalTest{},
    fZ{} {
    const auto delta{est - mu};
    fZ = delta.Value() / delta.StdDev();
}

// ============================================================================
// Multivariate test base class
// ============================================================================

template<int K, CovarianceOption C, int L, CovarianceOption D>
MultivariateStatisticalTest::MultivariateStatisticalTest(const Estimate<K, C>& est1, const Estimate<L, D>& est2) :
    StatisticalTest{},
    fDim{est1.Dimension()} {
    if (fDim != est2.Dimension()) {
        Throw<std::invalid_argument>(fmt::format(
            "Dimension mismatch: est1 has dimension {} but est2 has dimension {}", fDim, est2.Dimension()));
    }
}

template<int K, CovarianceOption C, typename AVec>
MultivariateStatisticalTest::MultivariateStatisticalTest(const Estimate<K, C>& est, const Eigen::MatrixBase<AVec>& mu, int nConstraint) :
    StatisticalTest{},
    fDim{est.Dimension()} {
    if (fDim != mu.size()) {
        Throw<std::invalid_argument>(
            fmt::format("Dimension mismatch: est has dimension {} but mu has dimension {}", fDim, mu.size()));
    }
    if (nConstraint < 0 or nConstraint >= fDim) {
        Throw<std::invalid_argument>(fmt::format(
            "Number of constraints must be in [0, {}), got nConstraint={}", fDim, nConstraint));
    }
}

template<typename AMat, typename AVec>
    requires(AVec::ColsAtCompileTime == 1)
auto MultivariateStatisticalTest::InverseBilinearForm(const Eigen::MatrixBase<AMat>& aXpr, const Eigen::MatrixBase<AVec>& uXpr) -> double {
    const auto& u{uXpr.eval()};
    return u.dot(aXpr.ldlt().solve(u));
}

template<typename AMat, typename AVec>
    requires(AVec::ColsAtCompileTime == 1)
auto MultivariateStatisticalTest::InverseBilinearForm(const Eigen::DiagonalBase<AMat>& aXpr, const Eigen::MatrixBase<AVec>& uXpr) -> double {
    return aXpr.diagonal().cwiseInverse().dot(uXpr.cwiseSquare());
}

// ===========================================================================
// T-squared test
// ===========================================================================

template<int K, CovarianceOption C, int L, CovarianceOption D>
StatisticalTSquaredTest::StatisticalTSquaredTest(const Statistic<K, C>& stat1, const Statistic<L, D>& stat2) :
    StatisticalTSquaredTest{stat1.MeanEstimate(), stat1.EffectiveSampleSize(),
                            stat2.MeanEstimate(), stat2.EffectiveSampleSize()} {}

template<int K, CovarianceOption C, int L, CovarianceOption D>
StatisticalTSquaredTest::StatisticalTSquaredTest(const Estimate<K, C>& est1, double n1, const Estimate<L, D>& est2, double n2) :
    MultivariateStatisticalTest{est1, est2},
    fNDF1{},
    fNDF2{},
    fTSq{} {
    if (n1 <= 1 or n2 <= 1) {
        Throw<std::invalid_argument>(fmt::format("Sample sizes must be greater than 1, got n1={} and n2={}", n1, n2));
    }
    // Reference: K. Krishnamoorthy, Jianqi Yu, Modified Nel and Van der Merwe test for the multivariate Behrens-Fisher problem,
    // Statistics & Probability Letters, Volume 66, Issue 2, 2004, doi:10.1016/j.spl.2003.10.012.
    fNDF1 = Dimension();
    const auto delta{est1 - est2};
    const auto& deltaX{delta.Value()};
    const auto& deltaCov{delta.Covariance()};
    constexpr auto denom{[](const auto& ss, auto n) {
        // ss can be diagonal matrix which does not have .trace() ... so use .diagonal().sum() instead
        return ((ss * ss).diagonal().sum() + muc::pow(ss.diagonal().sum(), 2)) / (n - 1);
    }};
    if constexpr (typename decltype(delta)::FullCovariance{}) {
        const auto sLDLT{deltaCov.ldlt()};
        using MatType = typename decltype(delta)::CovarianceType;
        MatType ss;
        if constexpr (C == CovarianceOption::Full) {
            ss = sLDLT.solve(est1.Covariance());
        } else {
            ss = sLDLT.solve(MatType{est1.Covariance()});
        }
        fNDF2 = denom(ss, n1);
        if constexpr (D == CovarianceOption::Full) {
            ss = sLDLT.solve(est2.Covariance());
        } else {
            ss = sLDLT.solve(MatType{est2.Covariance()});
        }
        fNDF2 = fNDF1 * (1 + fNDF1) / (fNDF2 + denom(ss, n2));
        fTSq = deltaX.dot(sLDLT.solve(deltaX));
    } else {
        const auto ss1{deltaCov.inverse() * est1.Covariance()};
        const auto ss2{deltaCov.inverse() * est2.Covariance()};
        fNDF2 = fNDF1 * (1 + fNDF1) / (denom(ss1, n1) + denom(ss2, n2));
        fTSq = InverseBilinearForm(deltaCov, deltaX);
    }
}

template<int K, CovarianceOption C, typename AVec>
    requires((AVec::RowsAtCompileTime == K or
              K == Eigen::Dynamic or AVec::RowsAtCompileTime == Eigen::Dynamic) and
             AVec::ColsAtCompileTime == 1)
StatisticalTSquaredTest::StatisticalTSquaredTest(const Statistic<K, C>& stat, const Eigen::MatrixBase<AVec>& mu, int nConstraint) :
    StatisticalTSquaredTest{stat.MeanEstimate(), stat.EffectiveSampleSize(), mu, nConstraint} {}

template<int K, CovarianceOption C, typename AVec>
    requires((AVec::RowsAtCompileTime == K or
              K == Eigen::Dynamic or AVec::RowsAtCompileTime == Eigen::Dynamic) and
             AVec::ColsAtCompileTime == 1)
StatisticalTSquaredTest::StatisticalTSquaredTest(const Estimate<K, C>& est, int n, const Eigen::MatrixBase<AVec>& mu, int nConstraint) :
    MultivariateStatisticalTest{est, mu, nConstraint},
    fNDF1{},
    fNDF2{},
    fTSq{} {
    if (n <= 1) {
        Throw<std::invalid_argument>(fmt::format("Sample size must be greater than 1, got n={}", n));
    }
    const auto delta{est - mu};
    fNDF1 = Dimension() - nConstraint;
    fNDF2 = n - 1;
    fTSq = InverseBilinearForm(delta.Covariance(), delta.Value()); // standard Hotelling's T-squared
}

// ===========================================================================
// Chi-squared test
// ===========================================================================

template<int K, CovarianceOption C, int L, CovarianceOption D>
StatisticalChiSquaredTest::StatisticalChiSquaredTest(const Statistic<K, C>& stat1, const Statistic<L, D>& stat2) :
    StatisticalChiSquaredTest{stat1.MeanEstimate(), stat2.MeanEstimate()} {}

template<int K, CovarianceOption C, int L, CovarianceOption D>
StatisticalChiSquaredTest::StatisticalChiSquaredTest(const Estimate<K, C>& est1, const Estimate<L, D>& est2) :
    MultivariateStatisticalTest{est1, est2},
    fNDF{},
    fChiSq{} {
    const auto delta{est1 - est2};
    fNDF = Dimension();
    fChiSq = InverseBilinearForm(delta.Covariance(), delta.Value());
}

template<int K, CovarianceOption C, typename AVec>
    requires((AVec::RowsAtCompileTime == K or
              K == Eigen::Dynamic or AVec::RowsAtCompileTime == Eigen::Dynamic) and
             AVec::ColsAtCompileTime == 1)
StatisticalChiSquaredTest::StatisticalChiSquaredTest(const Statistic<K, C>& stat, const Eigen::MatrixBase<AVec>& mu, int nConstraint) :
    StatisticalChiSquaredTest{stat.MeanEstimate(), mu, nConstraint} {}

template<int K, CovarianceOption C, typename AVec>
    requires((AVec::RowsAtCompileTime == K or
              K == Eigen::Dynamic or AVec::RowsAtCompileTime == Eigen::Dynamic) and
             AVec::ColsAtCompileTime == 1)
StatisticalChiSquaredTest::StatisticalChiSquaredTest(const Estimate<K, C>& est, const Eigen::MatrixBase<AVec>& mu, int nConstraint) :
    MultivariateStatisticalTest{est, mu, nConstraint},
    fNDF{},
    fChiSq{} {
    const auto delta{est - mu};
    fNDF = Dimension() - nConstraint;
    fChiSq = InverseBilinearForm(delta.Covariance(), delta.Value());
}

} // namespace Mustard::inline Math
