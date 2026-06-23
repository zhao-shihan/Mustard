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

#include "Eigen/Core"

#include "mplr/mplr.hpp"

#include "muc/math"

#include "fmt/format.h"

#include <cmath>
#include <concepts>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace Mustard::inline Math {

// Forward declaration. See definitions below.
template<int K, CovarianceOption C = CovarianceOption::Full>
class Statistic;

// Dummy definition. See partial specialization below.
template<int K, CovarianceOption C = CovarianceOption::Full>
struct StatisticPOD : std::monostate {};

namespace impl {

/// @brief Running statistic for weighted @f$K@f$-dimensional data.
///
/// Implements a stable online algorithm for computing running mean,
/// variance and covariance in a single pass over weighted data.
/// The algorithm is numerically stable and supports merging two accumulators
/// (reduction) via `operator+=` and `operator+`.
///
/// Given a sequence of weighted observations @f$(x_i, w_i)@f$, the class maintains
/// five running aggregates:
///
/// @f[
/// \begin{equation}
/// n   = \text{observation count} \quad
/// W   = \sum w_i \quad
/// W_2 = \sum w_i^2 \quad
/// M   = \sum w_i x_i
/// \end{equation}
/// @f]
///
/// The @f$M_2@f$ matrix is updated incrementally. Denoting the accumulated weight
/// before and after the @f$i@f$-th observation as @f$W_{i-1}@f$ and
/// @f$W_i = W_{i-1} + w_i@f$, and the updated mean as @f$\mu_i@f$, each step applies
/// @f[
/// M_2 \mathrel{+}= \frac{w_i W_i}{W_{i-1}}
///                  (x_i - \mu_i)(x_i - \mu_i)^\mathsf{T}
/// @f]
/// This is equivalent to the more familiar West--Chan form
/// @f$\frac{w_i W_{i-1}}{W_i}(x_i - \mu_{i-1})(x_i - \mu_{i-1})^\mathsf{T}@f$
/// but uses the updated mean for numerical convenience.
///
/// From these the sample covariance and covariance of the mean are recovered as:
///
/// @f[
/// \begin{equation}
/// \operatorname{Cov}(x) = \frac{W}{W^2 - W_2} \; M_2 \quad
/// \operatorname{Cov}(\bar{x}) = \frac{W_2}{W\,(W^2 - W_2)} \; M_2
/// \end{equation}
/// @f]
///
/// The effective sample size (Kish's formula) is @f$W^2 / W_2@f$.
///
/// When `C == CovarianceOption::Diagonal`, only the diagonal elements of
/// @f$M_2@f$ are stored (variances only), reducing memory from @f$O(K^2)@f$ to @f$O(K)@f$.
///
/// @tparam K Dimension of the observation space. Use `Eigen::Dynamic`
///           for runtime-determined dimension.
/// @tparam C Covariance option (`CovarianceOption::Full` or `Diagonal`)
///
/// @note This class resides in `namespace impl` and is the internal base.
///       Public users should instantiate `Statistic<K, C>` or its specializations.
/// @see Statistic, Estimate
template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
class StatisticBase {
    template<typename AOther, int L, CovarianceOption D>
        requires GoodStatisticDimension<L>::value
    friend class StatisticBase;

public:
    /// @brief Whether the dimension is known at compile time.
    using StaticDimension = std::bool_constant<K != Eigen::Dynamic>;
    /// @brief Whether the full covariance matrix is stored.
    using FullCovariance = std::bool_constant<C == CovarianceOption::Full>;
    /// @brief The value type (@f$\mathbb{R}^K@f$).
    using ValueType = Eigen::Vector<double, K>;
    /// @brief The Eigen matrix type for the @f$M_2@f$ statistic.
    ///        Full @f$K\times K@f$ matrix for `CovarianceOption::Full`, diagonal-only otherwise.
    using CovarianceType = std::conditional_t<FullCovariance{}, Eigen::Matrix<double, K, K>, Eigen::DiagonalMatrix<double, K>>;
    /// @brief The POD type used for MPI operation and Base64.
    using PODType = StatisticPOD<K, C>;

private:
    /// @brief Construct with uninitialized matrices.
    StatisticBase(std::monostate);

public:
    /// @brief Default constructor. Zero-initializes all accumulators.
    StatisticBase()
        requires(K != Eigen::Dynamic);
    /// @brief Move constructor.
    ///
    /// The source object is left in an undefined state.
    /// @warning Accessing any data or calling any operation (including
    /// `Fill`, `operator+=`) on a moved-from object is undefined behavior.
    /// @note A moved-from object is safe to assign a new value via
    /// assignment operators or `FromPOD()`.
    StatisticBase(StatisticBase&&) noexcept = default;
    /// @brief Copy constructor.
    StatisticBase(const StatisticBase& other);
    /// @brief Move assignment.
    ///
    /// The source object is left in an undefined state.
    /// @warning Accessing any data or calling any operation (including
    /// `Fill`, `operator+=`) on a moved-from object is undefined behavior.
    /// @note A moved-from object is safe to assign a new value via
    /// assignment operators or `FromPOD()`.
    auto operator=(StatisticBase&&) noexcept -> StatisticBase& = default;
    /// @brief Copy assignment.
    auto operator=(const StatisticBase& other) -> StatisticBase& { return CopyFrom(other); }

    /// @brief Construct with a given dimension.
    /// @param dim Number of components (must be positive)
    /// @throws std::invalid_argument if `dim <= 0`
    explicit StatisticBase(int dim)
        requires(K == Eigen::Dynamic);
    /// @brief Construct by copying state from another StatisticBase with possibly different CovarianceOption.
    /// @tparam L Dimension of the source
    /// @tparam D Covariance option of the source
    /// @param other Source statistic to copy from
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    explicit StatisticBase(const Statistic<L, D>& other);
    /// @brief Construct from POD data (e.g., from a Base64 string).
    /// @param data Previous statistic state as a plain-old-data struct
    explicit StatisticBase(const PODType& data)
        requires(K != Eigen::Dynamic);

    /// @brief Assign state from another StatisticBase with possibly different CovarianceOption.
    /// @tparam L Dimension of the source
    /// @tparam D Covariance option of the source
    /// @param other Source statistic to copy from
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto operator=(const Statistic<L, D>& other) -> ADerived& { return CopyFrom(other); }

    /// @brief Return the dimension of the observation space.
    auto Dimension() const -> int;

    /// @brief Feed one weighted observation into the statistic.
    ///
    /// Updates all running aggregates in @f$O(K^2)@f$ time (or @f$O(K)@f$ for
    /// `CovarianceOption::Diagonal`). Weights of zero are silently ignored.
    ///
    /// @tparam T An Eigen vector
    /// @param xXpr  The observation vector
    /// @param w  The observation weight (default: 1)
    template<typename AVec>
    auto Fill(const Eigen::MatrixBase<AVec>& xXpr, double w = 1) -> void;

    /// @brief Number of observations fed to the statistic.
    auto SampleSize() const -> auto { return fN; }
    /// @brief Sum of all observation weights, @f$W = \sum w_i@f$.
    auto WeightSum() const -> auto { return fW; }
    /// @brief Sum of squared observation weights, @f$W_2 = \sum w_i^2@f$.
    auto WeightSquareSum() const -> auto { return fW2; }

    /// @brief Weighted sum of the i-th component, @f$M_i = \sum w_k x_{k,i}@f$.
    /// @param i Component index (0-based)
    auto Sum(int i) const -> auto { return fM.coeff(i); }
    /// @brief Arithmetic mean of the i-th component, @f$\bar{x}_i = M_i / W@f$.
    /// @param i Component index (0-based)
    auto Mean(int i) const -> auto { return fM.coeff(i) / fW; }
    /// @brief Sample covariance between components i and j,
    ///        @f$\operatorname{Cov}_{ij} = \frac{W}{W^2 - W_2} M_{2,ij}@f$.
    /// @param i First component index (0-based)
    /// @param j Second component index (0-based)
    auto Covariance(int i, int j) const -> auto { return CovCoeff() * fM2.coeff(i, j); }
    /// @brief Sample correlation coefficient between components i and j,
    ///        @f$\rho_{ij} = \operatorname{Cov}_{ij} / \sqrt{\operatorname{Var}_i \operatorname{Var}_j}@f$.
    /// @param i First component index (0-based)
    /// @param j Second component index (0-based)
    auto Correlation(int i, int j) const -> double;
    /// @brief Sample variance of the i-th component, @f$\sigma_i^2 = \operatorname{Cov}_{ii}@f$.
    /// @param i Component index (0-based)
    auto Variance(int i) const -> auto { return Covariance(i, i); }
    /// @brief Sample standard deviation of the i-th component, @f$\sigma_i = \sqrt{\operatorname{Var}_i}@f$.
    /// @param i Component index (0-based)
    auto StdDev(int i) const -> auto { return std::sqrt(Variance(i)); }
    /// @brief Weighted sum vector, @f$M \in \mathbb{R}^K@f$.
    auto Sum() const -> const auto& { return fM; }
    /// @brief Arithmetic mean vector, @f$\bar{x} = M / W@f$.
    auto Mean() const -> auto { return MeanXpr().eval(); }
    /// @brief Full sample covariance matrix (or diagonal matrix for `CovarianceOption::Diagonal`).
    auto Covariance() const -> CovarianceType { return CovXpr(); }
    /// @brief Full sample correlation matrix (or identity diagonal matrix if `C == CovarianceOption::Diagonal`).
    auto Correlation() const -> CovarianceType { return Corr(&StatisticBase::CovXpr, &StatisticBase::VarXpr); }
    /// @brief Variance vector (diagonal of the covariance matrix).
    auto Variance() const -> auto { return VarXpr().eval(); }
    /// @brief Standard deviation vector, @f$\sigma = \sqrt{\operatorname{Var}}@f$.
    auto StdDev() const -> auto { return VarXpr().cwiseSqrt().eval(); }

    /// @brief Kish's effective sample size, @f$W^2 / W_2@f$.
    ///
    /// Returns 0 when no weights have been recorded. This quantity measures the
    /// equivalent number of equally-weighted observations.
    auto EffectiveSampleSize() const -> auto { return fW2 ? muc::pow(fW, 2) / fW2 : 0; }
    /// @brief Covariance of the sample mean between components i and j,
    ///        @f$\operatorname{Cov}(\bar{x})_{ij} = \frac{W_2}{W(W^2 - W_2)} M_{2,ij}@f$.
    /// @param i First component index (0-based)
    /// @param j Second component index (0-based)
    auto CovarianceOfMean(int i, int j) const -> auto { return CovMeanCoeff() * fM2.coeff(i, j); }
    /// @brief Correlation of the sample mean between components i and j,
    ///        @f$\rho_{\bar{x},ij} = \operatorname{Cov}(\bar{x})_{ij} / \sqrt{\operatorname{Var}(\bar{x})_i \operatorname{Var}(\bar{x})_j}@f$.
    /// @param i First component index (0-based)
    /// @param j Second component index (0-based)
    auto CorrelationOfMean(int i, int j) const -> double;
    /// @brief Variance of the sample mean for the i-th component.
    /// @param i Component index (0-based)
    auto VarianceOfMean(int i) const -> auto { return CovarianceOfMean(i, i); }
    /// @brief Standard deviation of the sample mean for the i-th component.
    /// @param i Component index (0-based)
    auto StdDevOfMean(int i) const -> auto { return std::sqrt(VarianceOfMean(i)); }
    /// @brief Value with variance for the i-th component.
    /// @param i Component index (0-based)
    /// @return An `Estimate` containing `Mean(i)` as value and `VarianceOfMean(i)` as variance.
    /// @see Estimate
    auto MeanEstimate(int i) const -> Estimate<1, C> { return {Mean(i), VarianceOfMean(i)}; }
    /// @brief Full covariance matrix of the sample mean.
    auto CovarianceOfMean() const -> CovarianceType { return CovMeanXpr(); }
    /// @brief Full correlation matrix of the sample mean (or identity diagonal matrix if `C == CovarianceOption::Diagonal`).
    auto CorrelationOfMean() const -> CovarianceType { return Corr(&StatisticBase::CovMeanXpr, &StatisticBase::VarMeanXpr); }
    /// @brief Variance vector of the sample mean.
    auto VarianceOfMean() const -> auto { return VarMeanXpr().eval(); }
    /// @brief Standard deviation vector of the sample mean.
    auto StdDevOfMean() const -> auto { return VarMeanXpr().cwiseSqrt().eval(); }
    /// @brief Full value vector with covariance matrix as an `Estimate`.
    /// @return `Estimate<K, C>{Mean(), CovarianceOfMean()}`
    /// @see Estimate
    auto MeanEstimate() const -> Estimate<K, C> { return {MeanXpr(), CovMeanXpr()}; }

    /// @brief Merge another statistic.
    ///
    /// Both accumulators must have the same dimension. The result is identical
    /// to having processed two sets of observations sequentially. The other
    /// statistic may have a different `CovarianceOption`, but the resulting
    /// covariance will be computed according to this statistic's option.
    ///
    /// @note When either operand is `CovarianceOption::Diagonal`, off-diagonal
    ///       elements from any `Full` operand are discarded — only the diagonal
    ///       (variances) participates in the merge. Full off-diagonal data is
    ///       only preserved when both sides are `CovarianceOption::Full`. The
    ///       result type of the free-function `operator+` is determined by
    ///       `StatisticCombineResultType`.
    ///
    /// @param other Another statistic, possibly with different `CovarianceOption`
    /// @return Reference to `*this`
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto operator+=(const Statistic<L, D>& other) -> ADerived&;

    /// @brief Convert the current state to a plain-old-data struct for MPI reduction and Base64 encoding.
    /// @return A `StatisticPOD<K, C>` containing copies of all accumulators.
    /// @note Only available when the dimension is known at compile time.
    ///       The `StatisticPOD` type is introspected by `MPLR_REFLECTION_TEMPLATE`
    ///       for use with MPI collective operations.
    auto ToPOD() const -> PODType
        requires(K != Eigen::Dynamic);
    /// @brief Restore state from a previously saved POD snapshot (e.g., from MPI reduction or Base64 decoding).
    /// @param data The POD data to restore from
    /// @note Only available when the dimension is known at compile time.
    auto FromPOD(const PODType& data) -> void
        requires(K != Eigen::Dynamic);

protected:
    /// @brief Computed mean vector (unevaluated expression), @f$\bar{x} = M / W@f$.
    auto MeanXpr() const -> auto { return fM / fW; }
    /// @brief Coefficient for sample covariance, @f$W / (W^2 - W_2)@f$.
    auto CovCoeff() const -> auto { return fW / (muc::pow(fW, 2) - fW2); }
    /// @brief Variance vector expression (unevaluated expression).
    auto VarXpr() const -> auto { return CovCoeff() * fM2.diagonal(); }
    /// @brief Covariance matrix expression (unevaluated expression).
    auto CovXpr() const -> auto { return CovCoeff() * fM2; }

    /// @brief Coefficient for covariance of the mean, @f$W_2 / (W (W^2 - W_2))@f$.
    auto CovMeanCoeff() const -> auto { return fW2 / (fW * (muc::pow(fW, 2) - fW2)); }
    /// @brief Variance-of-mean vector expression (unevaluated expression).
    auto VarMeanXpr() const -> auto { return CovMeanCoeff() * fM2.diagonal(); }
    /// @brief Covariance-of-mean matrix expression (unevaluated expression).
    auto CovMeanXpr() const -> auto { return CovMeanCoeff() * fM2; }

    /// @brief Sample correlation helper
    auto Corr(int i, int j, auto cov, auto var) const -> double;
    /// @brief Sample correlation helper
    auto Corr(auto covXpr, auto varXpr) const -> CovarianceType;

    /// @brief Accumulate the @f$M_2@f$ cross-term from a weighted deviation.
    /// @param otherW  Weight of the incoming data (@f$w_i@f$ or @f$W_B@f$)
    /// @param prevW  Total weight before the update (@f$W_{i-1}@f$ or @f$W_A@f$)
    /// @param deltaXpr  Deviation vector
    template<typename AVec>
    auto AppendM2CrossTerm(double otherW, double prevW, const Eigen::MatrixBase<AVec>& deltaXpr) -> void;

    /// @brief Check that a plain vector has matching dimension.
    /// @throws std::invalid_argument if dimensions differ (only checked for dynamic dimension)
    template<typename AVec>
        requires((AVec::RowsAtCompileTime == K or
                  K == Eigen::Dynamic or AVec::RowsAtCompileTime == Eigen::Dynamic) and
                 AVec::ColsAtCompileTime == 1)
    auto CheckDimensionMatch(const Eigen::MatrixBase<AVec>& y) const -> void;

private:
    /// @brief Copy statistic state from another statistic.
    ///
    /// The other statistic may have a different `CovarianceOption`,
    /// but the resulting covariance will be computed according to
    /// this statistic's option.
    ///
    /// @note When the source has `CovarianceOption::Diagonal` and the
    ///       destination has `CovarianceOption::Full`, the source's
    ///       diagonal is expanded into a full diagonal matrix.
    ///       When the source has `CovarianceOption::Full` and the
    ///       destination has `CovarianceOption::Diagonal`, only
    ///       the diagonal (variances) is copied.
    ///
    /// @tparam L Dimension of the source
    /// @tparam D Covariance option of the source
    /// @param other Source statistic to copy from
    template<typename AOther, int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto CopyFrom(const StatisticBase<AOther, L, D>& other) & -> ADerived&;

    auto Self() & -> auto&& { return static_cast<ADerived&>(*this); }
    auto Self() const& -> auto&& { return static_cast<const ADerived&>(*this); }
    auto Self() && -> auto&& { return static_cast<ADerived&&>(*this); }
    auto Self() const&& -> auto&& { return static_cast<const ADerived&&>(*this); }

private:
    long long fN;       ///< Observation count
    double fW;          ///< Sum of weights, @f$W = \sum w_i@f$
    double fW2;         ///< Sum of squared weights, @f$W_2 = \sum w_i^2@f$
    ValueType fM;       ///< Weighted sum vector, @f$M = \sum w_i x_i@f$
    CovarianceType fM2; ///< Weighted sum-of-squared-deviations matrix
};

} // namespace impl

/// @brief Scalar (1-dimensional) specialization of Statistic.
///
/// Provides scalar `double` accessors instead of Eigen vector/matrix types,
/// making the interface more convenient for single-variable statistics.
///
/// @tparam C Covariance option (`CovarianceOption::Full` or `Diagonal`)
///
/// @see StatisticBase, StatisticPOD, Estimate
template<CovarianceOption C>
class Statistic<1, C> : protected impl::StatisticBase<Statistic<1, C>, 1, C> {
    template<typename AOther, int L, CovarianceOption D>
        requires impl::GoodStatisticDimension<L>::value
    friend class impl::StatisticBase;

private:
    using Base = impl::StatisticBase<Statistic<1, C>, 1, C>;

public: // clang-format off
    /// @brief Whether the dimension is static (always true for this specialization).
    using typename Base::StaticDimension; // clang-format on
    /// @brief Whether full covariance matrix is stored.
    using typename Base::FullCovariance;
    /// @brief Value type is plain `double` for the scalar case.
    using ValueType = double;
    /// @brief Covariance type is plain `double` for the scalar case.
    using CovarianceType = double;
    /// @brief POD type for serialization.
    using typename Base::PODType;

public:
    /// @brief Inherit constructors from the base class.
    using Base::Base;

    /// @brief Inherit assignment operators from the base class.
    using Base::operator=;

    /// @brief Return the dimension of the observation space. Always returns 1.
    using Base::Dimension;

    /// @brief Feed a scalar observation with optional weight.
    /// @param x Observation value
    /// @param w Weight (default: 1)
    auto Fill(double x, double w = 1) -> void { Base::Fill(Eigen::Vector<double, 1>{x}, w); }

    /// @brief Number of observations.
    using Base::SampleSize;
    /// @brief Sum of weights.
    using Base::WeightSum;
    /// @brief Sum of squared weights.
    using Base::WeightSquareSum;

    /// @brief Weighted sum, @f$M = \sum w_i x_i@f$.
    auto Sum() const -> auto { return Base::Sum(0); }
    /// @brief Arithmetic mean, @f$\bar{x} = M / W@f$.
    auto Mean() const -> auto { return Base::Mean(0); }
    /// @brief Sample variance, @f$\sigma^2@f$.
    auto Variance() const -> auto { return Base::Variance(0); }
    /// @brief Sample standard deviation, @f$\sigma@f$.
    auto StdDev() const -> auto { return Base::StdDev(0); }

    /// @brief Kish's effective sample size.
    using Base::EffectiveSampleSize;
    /// @brief Variance of the sample mean, @f$\operatorname{Var}(\bar{x})@f$.
    auto VarianceOfMean() const -> auto { return Base::VarianceOfMean(0); }
    /// @brief Standard deviation of the sample mean.
    auto StdDevOfMean() const -> auto { return Base::StdDevOfMean(0); }
    /// @brief Value and variance as an `Estimate`.
    /// @return `Estimate{Mean(), VarianceOfMean()}`
    /// @see Estimate
    auto MeanEstimate() const -> auto { return Base::MeanEstimate(0); }

    /// @brief Merge another statistic with possibly different CovarianceOption.
    using Base::operator+=;

    /// @brief Inherit POD conversion from the base class.
    using Base::ToPOD;
    /// @brief Inherit POD restoration from the base class.
    using Base::FromPOD;
};

/// @brief @f$K@f$-dimensional static-dimension Statistic.
///
/// All Eigen vectors and matrices are fixed-size at compile time, giving
/// optimal performance when the dimension is known in advance.
///
/// @tparam K Dimension (positive integer)
/// @tparam C Covariance option
///
/// @see StatisticBase, StatisticPOD, Estimate
template<int K, CovarianceOption C>
class Statistic : public impl::StatisticBase<Statistic<K, C>, K, C> {
private:
    using Base = impl::StatisticBase<Statistic<K, C>, K, C>;

public:
    /// @brief Inherit constructors from the base class.
    using Base::Base;

    /// @brief Inherit assignment operators from the base class.
    using Base::operator=;
};

namespace impl {

/// @brief Determines the result type of merging two `Statistic` accumulators.
///
/// The result dimension is the compile-time K of the left operand when it is static,
/// otherwise the compile-time L of the right operand when it is static,
/// otherwise `Eigen::Dynamic` (runtime-determined).
///
/// The result covariance option is `Full` if either operand stores the full covariance
/// matrix, and `Diagonal` only if both are diagonal.
///
/// @tparam K Left-hand dimension
/// @tparam C Left-hand covariance option
/// @tparam L Right-hand dimension
/// @tparam D Right-hand covariance option
template<int K, CovarianceOption C, int L, CovarianceOption D>
using StatisticCombineResultType = Statistic<
    K != Eigen::Dynamic ?
        K :
        (L != Eigen::Dynamic ?
             L :
             Eigen::Dynamic),
    C == CovarianceOption::Full or
            D == CovarianceOption::Full ?
        CovarianceOption::Full :
        CovarianceOption::Diagonal>;

} // namespace impl

/// @name operator+
/// @brief Non-member addition merges two `Statistic` accumulators whose content is assumed independent.
///
/// The result type's dimension and covariance option are determined by
/// `StatisticCombineResultType`: the dimension is the compile-time constant
/// when available, otherwise runtime-determined; the covariance option is
/// `Full` if either operand stores the full covariance matrix.
///
/// @note When one operand is `Diagonal`, only its diagonal contributes to the
///       merged @f$M_2@f$ statistic.
/// @see StatisticCombineResultType
/// @{

template<int K, CovarianceOption C, int L, CovarianceOption D>
auto operator+(const Statistic<K, C>& lhs, const Statistic<L, D>& rhs) -> impl::StatisticCombineResultType<K, C, L, D>;

template<int K, CovarianceOption C, int L, CovarianceOption D>
auto operator+(const Statistic<K, C>& lhs, Statistic<L, D>&& rhs) -> impl::StatisticCombineResultType<K, C, L, D>;

template<int K, CovarianceOption C, int L, CovarianceOption D>
auto operator+(Statistic<K, C>&& lhs, const Statistic<L, D>& rhs) -> impl::StatisticCombineResultType<K, C, L, D>;

template<int K, CovarianceOption C, int L, CovarianceOption D>
auto operator+(Statistic<K, C>&& lhs, Statistic<L, D>&& rhs) -> impl::StatisticCombineResultType<K, C, L, D>;

/// @}

/// @brief POD (plain-old-data) struct holding a snapshot of `Statistic` state.
///
/// The primary purpose of this struct is MPI operation (enabled by the
/// `MPLR_REFLECTION_TEMPLATE` introspection at the bottom of this header)
/// and Base64 encoding for storage or network transfer. The contiguous
/// memory layout allows direct `memcpy` and use with MPI collective operations.
///
/// @tparam K Dimension (must be a compile-time constant, not `Eigen::Dynamic`)
/// @tparam C Covariance option
///
/// @see StatisticBase::ToPOD, StatisticBase::FromPOD, MPLR_REFLECTION_TEMPLATE
template<int K, CovarianceOption C>
    requires(impl::GoodStatisticDimension<K>{}() and K != Eigen::Dynamic)
struct StatisticPOD<K, C> {
    long long n;                            ///< Observation count
    double w;                               ///< Sum of weights
    double w2;                              ///< Sum of squared weights
    double m[K];                            ///< Weighted sum vector
    double m2[C == CovarianceOption::Full ? ///< @f$M_2@f$ statistic (flattened)
                  K * K :
                  K];

    /// @brief Merge two statistics.
    template<int L, CovarianceOption D>
    auto operator+(const StatisticPOD<L, D>& other) -> auto { return (Statistic<K, C>{*this} + Statistic<L, D>{other}).ToPOD(); }
};

} // namespace Mustard::inline Math

MPLR_REFLECTION_TEMPLATE((int K, Mustard::CovarianceOption C),
                         (Mustard::StatisticPOD<K, C>),
                         n, w, w2, m, m2)

#include "Mustard/Math/Statistic.inl"
