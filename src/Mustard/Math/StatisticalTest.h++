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

#pragma once

#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Math/Estimate.h++"
#include "Mustard/Math/Statistic.h++"
#include "Mustard/Utility/SemanticBase.h++"

#include "Eigen/Cholesky"
#include "Eigen/Core"

#include "muc/math"

#include "fmt/format.h"

#include <stdexcept>

namespace Mustard::inline Math {

/// @brief Base class for statistical hypothesis tests.
///
/// Provides the interface for computing test statistics.
///
/// @see UnivariateStatisticalTest, MultivariateStatisticalTest
class StatisticalTest : public MovableVirtualBase {
public:
    /// @brief Return the test statistic value.
    virtual auto Value() const -> double = 0;

protected:
    /// @brief Validate that a p-value lies in the valid range @f$[0, 1]@f$.
    /// @param p The p-value to validate
    /// @throws std::domain_error if @p p is outside @f$[0, 1]@f$
    static auto CheckPValue(double p) -> void;
    /// @brief Convert a p-value to a significance (normal quantile).
    /// @param p The p-value to convert
    /// @return The corresponding significance (normal quantile)
    static auto PValueToSignificance(double p) -> double;
};

/// @brief Base class for univariate (scalar) statistical tests.
///
/// Extends StatisticalTest with tail-probability queries
/// for one-tailed and two-tailed hypothesis tests.
/// Significance values are computed by converting the
/// tail probabilities to equivalent normal deviates.
///
/// @see StatisticalTTest, StatisticalZTest
class UnivariateStatisticalTest : public StatisticalTest {
public:
    /// @brief Left-tail p-value (probability that the test statistic
    ///        is less than or equal to the observed value under the null hypothesis).
    virtual auto LeftTailP() const -> double = 0;
    /// @brief Left-tail significance (normal quantile of the left-tail p-value).
    virtual auto LeftTailSignificance() const -> double { return PValueToSignificance(LeftTailP()); }
    /// @brief Right-tail p-value (probability that the test statistic
    ///        is greater than or equal to the observed value under the null hypothesis).
    virtual auto RightTailP() const -> double = 0;
    /// @brief Right-tail significance (normal quantile of the right-tail p-value).
    virtual auto RightTailSignificance() const -> double { return PValueToSignificance(RightTailP()); }
    /// @brief Two-sided p-value (probability that the absolute value of the test
    ///        statistic exceeds the absolute observed value under the null hypothesis).
    virtual auto TwoSidedP() const -> double = 0;
    /// @brief Two-sided significance (normal quantile of the two-sided p-value).
    virtual auto TwoSidedSignificance() const -> double { return PValueToSignificance(TwoSidedP()); }
};

/// @brief Welch's t-test.
///
/// Performs either a two-sample Welch's t-test or a one-sample t-test.
/// The two-sample test does not assume equal variances.
///
/// The test statistic is
/// @f[
/// t = \frac{\bar{x}_1 - \bar{x}_2}{\sqrt{s_1^2/n_1 + s_2^2/n_2}}
/// @f]
/// with degrees of freedom approximated by the Welch--Satterthwaite equation:
/// @f[
/// \nu = \frac{(s_1^2/n_1 + s_2^2/n_2)^2}
///            {\frac{(s_1^2/n_1)^2}{n_1-1} + \frac{(s_2^2/n_2)^2}{n_2-1}}
/// @f]
///
/// For the one-sample test (@f$\mu_0@f$ is the null-hypothesis mean):
/// @f[
/// t = \frac{\bar{x} - \mu_0}{s / \sqrt{n}}, \quad \nu = n - 1
/// @f]
///
/// @note As the sample size(s) go to infinity, the t-test becomes
///       asymptotically equivalent to the Z-test.
/// @see StatisticalZTest
class StatisticalTTest : public UnivariateStatisticalTest {
public:
    /// @name Two-sample constructors
    /// @{

    /// @brief Two-sample t-test from running statistics.
    /// @tparam C Covariance option of both statistics
    /// @param stat1 First sample's running statistic
    /// @param stat2 Second sample's running statistic
    template<CovarianceOption C>
    StatisticalTTest(const Statistic<1, C>& stat1, const Statistic<1, C>& stat2);
    /// @brief Two-sample t-test from estimates with explicit sample sizes.
    /// @tparam C Covariance option of both estimates
    /// @param est1 First sample's mean estimate
    /// @param n1  First sample's size (must be > 1)
    /// @param est2 Second sample's mean estimate
    /// @param n2  Second sample's size (must be > 1)
    /// @throws std::invalid_argument if @p n1 <= 1 or @p n2 <= 1
    template<CovarianceOption C>
    StatisticalTTest(const Estimate<1, C>& est1, double n1, const Estimate<1, C>& est2, double n2);

    /// @}

    /// @name One-sample constructors
    /// @{

    /// @brief One-sample t-test from a running statistic against a known mean @p mu.
    /// @tparam C Covariance option of the statistic
    /// @param stat Running statistic
    /// @param mu   Null-hypothesis mean
    template<CovarianceOption C>
    StatisticalTTest(const Statistic<1, C>& stat, double mu);
    /// @brief One-sample t-test from an estimate against a known mean @p mu.
    /// @tparam C Covariance option of the estimate
    /// @param est Mean estimate
    /// @param n   Sample size (must be > 1)
    /// @param mu  Null-hypothesis mean
    /// @throws std::invalid_argument if @p n <= 1
    template<CovarianceOption C>
    StatisticalTTest(const Estimate<1, C>& est, double n, double mu);

    /// @}

    /// @brief Degrees of freedom, @f$\nu@f$.
    auto NDF() const -> auto { return fNDF; }

    /// @brief t-statistic, @f$t@f$.
    auto Value() const -> double override { return fT; }
    /// @brief Left-tail p-value, @f$P(T \le t \mid \nu)@f$.
    auto LeftTailP() const -> double override;
    /// @brief Right-tail p-value, @f$P(T \ge t \mid \nu)@f$.
    auto RightTailP() const -> double override;
    /// @brief Two-sided p-value, @f$2 \cdot P(T \ge |t| \mid \nu)@f$.
    auto TwoSidedP() const -> double override;

private:
    double fNDF; ///< Degrees of freedom
    double fT;   ///< t-statistic
};

/// @brief Z-test.
///
/// Performs either a two-sample Z-test or a one-sample Z-test.
/// Unlike the t-test, the Z-test assumes the population variances
/// are known exactly (not estimated from finite samples), so the
/// test statistic follows a standard normal distribution.
///
/// The test statistic for the two-sample case is
/// @f[
/// z = \frac{\bar{x}_1 - \bar{x}_2}{\sqrt{\sigma_1^2 + \sigma_2^2}}
/// @f]
///
/// For the one-sample case (@f$\mu_0@f$ is the null-hypothesis mean):
/// @f[
/// z = \frac{\bar{x} - \mu_0}{\sigma}
/// @f]
///
/// @note As the sample size(s) go to infinity, the t-test becomes
///       asymptotically equivalent to the Z-test.
/// @see StatisticalTTest
class StatisticalZTest : public UnivariateStatisticalTest {
public:
    /// @name Two-sample constructors
    /// @{

    /// @brief Two-sample Z-test from running statistics.
    /// @tparam C Covariance option of both statistics
    /// @param stat1 First sample's statistic
    /// @param stat2 Second sample's statistic
    template<CovarianceOption C>
    StatisticalZTest(const Statistic<1, C>& stat1, const Statistic<1, C>& stat2);
    /// @brief Two-sample Z-test from estimates.
    /// @tparam C Covariance option of both estimates
    /// @param est1 First sample's mean estimate
    /// @param est2 Second sample's mean estimate
    template<CovarianceOption C>
    StatisticalZTest(const Estimate<1, C>& est1, const Estimate<1, C>& est2);

    /// @}

    /// @name One-sample constructors
    /// @{

    /// @brief One-sample Z-test from a running statistic against a known mean @p mu.
    /// @tparam C Covariance option of the statistic
    /// @param stat Running statistic
    /// @param mu   Null-hypothesis mean
    template<CovarianceOption C>
    StatisticalZTest(const Statistic<1, C>& stat, double mu);
    /// @brief One-sample Z-test from an estimate against a known mean @p mu.
    /// @tparam C Covariance option of the estimate
    /// @param est Mean estimate
    /// @param mu  Null-hypothesis mean
    template<CovarianceOption C>
    StatisticalZTest(const Estimate<1, C>& est, double mu);

    /// @}

    /// @brief Z-statistic, @f$z@f$.
    auto Value() const -> double override { return fZ; }
    /// @brief Left-tail p-value, @f$\Phi(z)@f$.
    auto LeftTailP() const -> double override;
    /// @brief Left-tail significance, @f$-z@f$.
    auto LeftTailSignificance() const -> double override { return -fZ; }
    /// @brief Right-tail p-value, @f$1 - \Phi(z)@f$.
    auto RightTailP() const -> double override;
    /// @brief Right-tail significance, @f$z@f$.
    auto RightTailSignificance() const -> double override { return fZ; }
    /// @brief Two-sided p-value, @f$2 \cdot (1 - \Phi(|z|))@f$.
    auto TwoSidedP() const -> double override;
    /// @brief Two-sided significance, @f$|z|@f$.
    auto TwoSidedSignificance() const -> double override { return std::abs(fZ); }

private:
    double fZ; ///< Z-statistic
};

/// @brief Base class for multivariate statistical tests.
///
/// Extends StatisticalTest with p-value and significance queries
/// for multidimensional hypothesis tests. The significance is
/// computed by converting the p-value to an equivalent normal
/// deviate (one-tailed).
///
/// Provides the static helper `InverseBilinearForm` for computing
/// the quadratic form @f$u^\mathsf{T} A^{-1} u@f$ efficiently,
/// which is the core operation in Hotelling's @f$T^2@f$ and
/// chi-squared tests.
///
/// @see StatisticalTSquaredTest, StatisticalChiSquaredTest
class MultivariateStatisticalTest : public StatisticalTest {
protected:
    /// @brief Construct from two estimates (two-sample test basis).
    /// @tparam K Dimension of the first estimate
    /// @tparam C Covariance option of the first estimate
    /// @tparam L Dimension of the second estimate
    /// @tparam D Covariance option of the second estimate
    /// @param est1 First estimate
    /// @param est2 Second estimate
    /// @throws std::invalid_argument if @p est1 and @p est2 have different dimensions
    template<int K, CovarianceOption C, int L, CovarianceOption D>
    MultivariateStatisticalTest(const Estimate<K, C>& est1, const Estimate<L, D>& est2);
    /// @brief Construct from an estimate and a null-hypothesis mean vector
    ///        (one-sample test basis).
    /// @tparam K Dimension of the estimate
    /// @tparam C Covariance option of the estimate
    /// @tparam AVec Eigen vector type for the null-hypothesis mean
    /// @param est Estimate
    /// @param mu  Null-hypothesis mean vector
    /// @param nConstraint Number of constraints that reduce the effective
    ///                    degrees of freedom (default: 0)
    /// @throws std::invalid_argument if @p est and @p mu have different dimensions,
    ///         or if @p nConstraint is negative or not less than the dimension
    template<int K, CovarianceOption C, typename AVec>
    MultivariateStatisticalTest(const Estimate<K, C>& est, const Eigen::MatrixBase<AVec>& mu, int nConstraint);

public:
    /// @brief Dimension of the test (number of components being tested).
    auto Dimension() const -> auto { return fDim; }

    /// @brief P-value of the test statistic.
    virtual auto PValue() const -> double = 0;
    /// @brief Significance (normal quantile of the p-value).
    virtual auto Significance() const -> double { return PValueToSignificance(PValue()); }

protected:
    /// @brief Compute the quadratic form @f$u^\mathsf{T} A^{-1} u@f$
    ///        for a symmetric positive-definite matrix @f$A@f$.
    ///
    /// Uses an LDLT decomposition of @f$A@f$ for efficient solution.
    ///
    /// @tparam AMat Eigen matrix expression type for @f$A@f$
    /// @tparam AVec Eigen vector type for @f$u@f$
    /// @param aXpr The matrix @f$A@f$
    /// @param uXpr The vector @f$u@f$
    /// @return @f$u^\mathsf{T} A^{-1} u@f$
    template<typename AMat, typename AVec>
        requires(AVec::ColsAtCompileTime == 1)
    static auto InverseBilinearForm(const Eigen::MatrixBase<AMat>& aXpr, const Eigen::MatrixBase<AVec>& uXpr) -> double;
    /// @brief Compute the quadratic form @f$u^\mathsf{T} A^{-1} u@f$
    ///        for a diagonal matrix @f$A@f$.
    ///
    /// Uses element-wise inverse of the diagonal, which is more efficient
    /// than a full decomposition.
    ///
    /// @tparam AMat Eigen diagonal matrix expression type
    /// @tparam AVec Eigen vector type for @f$u@f$
    /// @param aXpr The diagonal matrix @f$A@f$
    /// @param uXpr The vector @f$u@f$
    /// @return @f$u^\mathsf{T} A^{-1} u@f$
    template<typename AMat, typename AVec>
        requires(AVec::ColsAtCompileTime == 1)
    static auto InverseBilinearForm(const Eigen::DiagonalBase<AMat>& aXpr, const Eigen::MatrixBase<AVec>& uXpr) -> double;

private:
    int fDim; ///< Dimension of the test
};

/// @brief Multivariate @f$T^2@f$ test.
///
/// A multivariate generalization of the t-test. The one-sample test is
/// Hotelling's @f$T^2@f$ test. The two-sample test extends it with the
/// Behrens--Fisher problem addressed by Nel--Van der Merwe's approximation
/// for the second degrees of freedom.
///
/// The test statistic for the two-sample case is
/// @f[
/// T^2 = (\bar{x}_1 - \bar{x}_2)^\mathsf{T}
///        \left(\frac{S_1}{n_1} + \frac{S_2}{n_2}\right)^{-1}
///        (\bar{x}_1 - \bar{x}_2)
/// @f]
///
/// The p-value is obtained from the F-distribution via:
/// @f[
/// \frac{\nu_2 - \nu_1 + 1}{\nu_1 \nu_2} T^2 \sim F(\nu_1, \nu_2 - \nu_1 + 1)
/// @f]
///
/// For the one-sample case, @f$\nu_1 = p - n_\text{Constraint}@f$ and
/// @f$\nu_2 = n - 1@f$, corresponding to Hotelling's original @f$T^2@f$
/// distribution.
///
/// @note As the sample size(s) go to infinity, the @f$T^2@f$ test
///       becomes asymptotically equivalent to the chi-squared test.
/// @see StatisticalChiSquaredTest
class StatisticalTSquaredTest : public MultivariateStatisticalTest {
public:
    /// @name Two-sample constructors
    /// @{

    /// @brief Two-sample @f$T^2@f$ test from running statistics.
    /// @tparam K Dimension of the first statistic
    /// @tparam C Covariance option of the first statistic
    /// @tparam L Dimension of the second statistic
    /// @tparam D Covariance option of the second statistic
    /// @param stat1 First sample's statistic
    /// @param stat2 Second sample's statistic
    template<int K, CovarianceOption C, int L, CovarianceOption D>
    StatisticalTSquaredTest(const Statistic<K, C>& stat1, const Statistic<L, D>& stat2);
    /// @brief Two-sample @f$T^2@f$ test from estimates with explicit sample sizes.
    /// @tparam K Dimension of the first estimate
    /// @tparam C Covariance option of the first estimate
    /// @tparam L Dimension of the second estimate
    /// @tparam D Covariance option of the second estimate
    /// @param est1 First sample's mean estimate
    /// @param n1   First sample's size (must be > 1)
    /// @param est2 Second sample's mean estimate
    /// @param n2   Second sample's size (must be > 1)
    /// @throws std::invalid_argument if @p n1 <= 1 or @p n2 <= 1,
    ///         or if dimensions mismatch
    template<int K, CovarianceOption C, int L, CovarianceOption D>
    StatisticalTSquaredTest(const Estimate<K, C>& est1, double n1, const Estimate<L, D>& est2, double n2);

    /// @}

    /// @name One-sample constructors (Hotelling's @f$T^2@f$)
    /// @{

    /// @brief One-sample Hotelling's @f$T^2@f$ test from a running statistic
    ///        against a null mean vector.
    /// @tparam K Dimension of the statistic
    /// @tparam C Covariance option of the statistic
    /// @tparam AVec Eigen vector type for the null-hypothesis mean
    /// @param stat Running statistic
    /// @param mu   Null-hypothesis mean vector
    /// @param nConstraint Number of constraints (default: 0)
    template<int K, CovarianceOption C, typename AVec>
        requires((AVec::RowsAtCompileTime == K or
                  K == Eigen::Dynamic or AVec::RowsAtCompileTime == Eigen::Dynamic) and
                 AVec::ColsAtCompileTime == 1)
    StatisticalTSquaredTest(const Statistic<K, C>& stat, const Eigen::MatrixBase<AVec>& mu, int nConstraint = 0);
    /// @brief One-sample Hotelling's @f$T^2@f$ test from an estimate
    ///        against a null mean vector.
    /// @tparam K Dimension of the estimate
    /// @tparam C Covariance option of the estimate
    /// @tparam AVec Eigen vector type for the null-hypothesis mean
    /// @param est Mean estimate
    /// @param n   Sample size (must be > 1)
    /// @param mu  Null-hypothesis mean vector
    /// @param nConstraint Number of constraints (default: 0)
    /// @throws std::invalid_argument if @p n <= 1, or if dimensions mismatch,
    ///         or if @p nConstraint is invalid
    template<int K, CovarianceOption C, typename AVec>
        requires((AVec::RowsAtCompileTime == K or
                  K == Eigen::Dynamic or AVec::RowsAtCompileTime == Eigen::Dynamic) and
                 AVec::ColsAtCompileTime == 1)
    StatisticalTSquaredTest(const Estimate<K, C>& est, int n, const Eigen::MatrixBase<AVec>& mu, int nConstraint = 0);

    /// @}

    /// @brief First degrees of freedom, @f$\nu_1 = p - n_\text{Constraint}@f$.
    auto NDF1() const -> auto { return fNDF1; }
    /// @brief Second degrees of freedom, @f$\nu_2@f$.
    auto NDF2() const -> auto { return fNDF2; }

    /// @brief @f$T^2@f$ statistic.
    auto Value() const -> double override { return fTSq; }
    /// @brief P-value, @f$P(\T^2(\nu) \ge \T^2_{\mathrm{obs}})@f$.
    auto PValue() const -> double override;

private:
    double fNDF1; ///< First degrees of freedom
    double fNDF2; ///< Second degrees of freedom
    double fTSq;  ///< @f$T^2@f$ statistic
};

/// @brief Chi-squared test.
///
/// A multivariate test assuming known (population) covariance matrices.
/// The test statistic follows a chi-squared distribution under
/// the null hypothesis.
///
/// The test statistic for the two-sample case is
/// @f[
/// \chi^2 = (\bar{x}_1 - \bar{x}_2)^\mathsf{T}
///           (\Sigma_1 + \Sigma_2)^{-1}
///           (\bar{x}_1 - \bar{x}_2)
/// @f]
///
/// For the one-sample case (@f$\mu_0@f$ is the null-hypothesis mean):
/// @f[
/// \chi^2 = (\bar{x} - \mu_0)^\mathsf{T} \Sigma^{-1} (\bar{x} - \mu_0)
/// @f]
///
/// @note As the sample size(s) go to infinity, the @f$T^2@f$ test
///       becomes asymptotically equivalent to the chi-squared test.
/// @see StatisticalTSquaredTest
class StatisticalChiSquaredTest : public MultivariateStatisticalTest {
public:
    /// @name Two-sample constructors
    /// @{

    /// @brief Two-sample chi-squared test from running statistics.
    /// @tparam K Dimension of the first statistic
    /// @tparam C Covariance option of the first statistic
    /// @tparam L Dimension of the second statistic
    /// @tparam D Covariance option of the second statistic
    /// @param stat1 First sample's statistic
    /// @param stat2 Second sample's statistic
    template<int K, CovarianceOption C, int L, CovarianceOption D>
    StatisticalChiSquaredTest(const Statistic<K, C>& stat1, const Statistic<L, D>& stat2);
    /// @brief Two-sample chi-squared test from estimates.
    /// @tparam K Dimension of the first estimate
    /// @tparam C Covariance option of the first estimate
    /// @tparam L Dimension of the second estimate
    /// @tparam D Covariance option of the second estimate
    /// @param est1 First sample's mean estimate
    /// @param est2 Second sample's mean estimate
    template<int K, CovarianceOption C, int L, CovarianceOption D>
    StatisticalChiSquaredTest(const Estimate<K, C>& est1, const Estimate<L, D>& est2);

    /// @}

    /// @name One-sample constructors
    /// @{

    /// @brief One-sample chi-squared test from a running statistic against a null mean vector.
    /// @tparam K Dimension of the statistic
    /// @tparam C Covariance option of the statistic
    /// @tparam AVec Eigen vector type for the null-hypothesis mean
    /// @param stat Running statistic
    /// @param mu   Null-hypothesis mean vector
    /// @param nConstraint Number of constraints (default: 0)
    template<int K, CovarianceOption C, typename AVec>
        requires((AVec::RowsAtCompileTime == K or
                  K == Eigen::Dynamic or AVec::RowsAtCompileTime == Eigen::Dynamic) and
                 AVec::ColsAtCompileTime == 1)
    StatisticalChiSquaredTest(const Statistic<K, C>& stat, const Eigen::MatrixBase<AVec>& mu, int nConstraint = 0);
    /// @brief One-sample chi-squared test from an estimate against a null mean vector.
    /// @tparam K Dimension of the estimate
    /// @tparam C Covariance option of the estimate
    /// @tparam AVec Eigen vector type for the null-hypothesis mean
    /// @param est Mean estimate
    /// @param mu  Null-hypothesis mean vector
    /// @param nConstraint Number of constraints (default: 0)
    /// @throws std::invalid_argument if dimensions mismatch or if @p nConstraint is invalid
    template<int K, CovarianceOption C, typename AVec>
        requires((AVec::RowsAtCompileTime == K or
                  K == Eigen::Dynamic or AVec::RowsAtCompileTime == Eigen::Dynamic) and
                 AVec::ColsAtCompileTime == 1)
    StatisticalChiSquaredTest(const Estimate<K, C>& est, const Eigen::MatrixBase<AVec>& mu, int nConstraint = 0);

    /// @}

    /// @brief Degrees of freedom, @f$\nu_1 = p - n_\text{Constraint}@f$.
    auto NDF() const -> auto { return fNDF; }

    /// @brief Chi-squared statistic, @f$\chi^2@f$.
    auto Value() const -> double override { return fChiSq; }
    /// @brief P-value, @f$P(\chi^2(\nu) \ge \chi^2_{\mathrm{obs}})@f$.
    auto PValue() const -> double override;

private:
    double fNDF;   ///< Degrees of freedom
    double fChiSq; ///< Chi-squared statistic
};

} // namespace Mustard::inline Math

#include "Mustard/Math/StatisticalTest.inl"
