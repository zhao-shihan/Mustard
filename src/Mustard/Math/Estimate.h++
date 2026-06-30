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

#include "Eigen/Cholesky"
#include "Eigen/Core"
#include "unsupported/Eigen/SpecialFunctions"

#include "mplr/mplr.hpp"

#include "muc/math"
#include "muc/numeric"

#include "fmt/format.h"

#include <cmath>
#include <cstring>
#include <numbers>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace Mustard::inline Math {

/// @brief Controls how the covariance matrix is represented.
enum struct CovarianceOption {
    Full,    ///< Store the full @f$K\times K@f$ covariance matrix
    Diagonal ///< Store only the diagonal elements of the covariance matrix (i.e., the variances)
};

namespace impl {

/// @brief Compile-time constraint validating the dimension parameter @f$K@f$.
///
/// The dimension must be either `Eigen::Dynamic` (runtime-determined)
/// or a positive integer satisfying
/// @f$K \cdot K \cdot \mathtt{sizeof(double)} \le \mathtt{EIGEN\_STACK\_ALLOCATION\_LIMIT}@f$,
/// so that fixed-size Eigen matrices do not overflow the stack.
///
/// @tparam K The dimension (static or `Eigen::Dynamic`)
///
/// @note This is an internal implementation detail. Prefer using the public
///       `Estimate<K, C>` or `Statistic<K, C>` class templates, which already
///       enforce the constraint via their `requires` clauses.
template<int K>
struct GoodStatisticDimension
    : std::bool_constant<
          K == Eigen::Dynamic or
          (K > 0 and sizeof(double) * K * K <= EIGEN_STACK_ALLOCATION_LIMIT)> {};

} // namespace impl

// Forward declaration. See definitions below.
template<int K, CovarianceOption C = CovarianceOption::Full>
class Estimate;

// Dummy definition. See partial specialization below.
template<int K, CovarianceOption C = CovarianceOption::Full>
struct EstimatePOD : std::monostate {};

namespace impl {

/// @brief Helper type to select a fixed-size vector whenever possible.
///
/// In binary operations, evaluate to this type instead of .eval() directly
/// to avoid unnecessary heap allocation when result dimension is known at compile time.
template<int K, int L>
using VecType = std::conditional_t<
    L == Eigen::Dynamic,
    Eigen::Vector<double, K>,
    Eigen::Vector<double, L>>;

/// @brief Helper type to select a fixed-size array whenever possible.
///
/// In binary operations, evaluate to this type instead of .eval() directly
/// to avoid unnecessary heap allocation when result dimension is known at compile time.
template<int K, int L>
using ArrType = std::conditional_t<
    L == Eigen::Dynamic,
    Eigen::Array<double, K, 1>,
    Eigen::Array<double, L, 1>>;

/// @brief Helper type to select a fixed-size matrix whenever possible.
///
/// In binary operations, evaluate to this type instead of .eval() directly
/// to avoid unnecessary heap allocation when result dimension is known at compile time.
template<int K, int L>
using MatType = std::conditional_t<
    L == Eigen::Dynamic,
    Eigen::Matrix<double, K, K>,
    Eigen::Matrix<double, L, L>>;

/// @brief Determines the result type of `Concat` between two `Estimate` objects.
///
/// The result dimension is @f$K+L@f$ when both @f$K@f$ and @f$L@f$ are compile-time
/// constants and @f$K+L@f$ fits within the Eigen stack allocation limit; otherwise
/// it is `Eigen::Dynamic` (runtime-determined).
///
/// The result covariance option is `Full` if either operand stores the full covariance
/// matrix, and `Diagonal` only if both are diagonal.
///
/// @tparam K Left-hand dimension
/// @tparam C Left-hand covariance option
/// @tparam L Right-hand dimension
/// @tparam D Right-hand covariance option
template<int K, CovarianceOption C, int L, CovarianceOption D>
using EstimateConcatResult = Estimate<
    (K != Eigen::Dynamic and L != Eigen::Dynamic and
     GoodStatisticDimension<K + L>::value) ?
        K + L :
        Eigen::Dynamic,
    (C == CovarianceOption::Full or D == CovarianceOption::Full) ?
        CovarianceOption::Full :
        CovarianceOption::Diagonal>;

/// @brief Core implementation for a value with associated covariance (uncertainty).
///
/// Stores an @f$K@f$-dimensional value vector and its covariance matrix.
/// Provides accessors for the stored quantities and derived statistics
/// (variance, standard deviation, relative uncertainty).
///
/// Supports element-wise binary operations between two independent
/// estimates, scalar/vector operations, and a variety of unary
/// element-wise transformations. All operations propagate uncertainties
/// via first-order Taylor expansion.
///
/// When `C == CovarianceOption::Diagonal`, only the diagonal elements of
/// the covariance matrix are stored (variances only), reducing memory
/// from @f$O(K^2)@f$ to @f$O(K)@f$.
///
/// @tparam K Dimension of the value space. Use `Eigen::Dynamic`
///           for runtime-determined dimension.
/// @tparam C Covariance option (`CovarianceOption::Full` or `Diagonal`)
///
/// @note When two `Estimate` objects with different `CovarianceOption` interact,
///       a `Diagonal` operand is treated as a covariance matrix whose off-diagonal
///       elements are zero. The full covariance propagation formula is applied
///       using this representation, and off-diagonal elements are discarded from
///       the result when the result type is `Diagonal` (see `EstimateBinaryOpResult`).
///
/// @note All binary operations between two `Estimate` objects (including
///       `operator+=`, `operator-=`, `operator*=`, `operator/=`, `Dot`,
///       `Pow`, `Exp`, `NegateAdd`, `Divide`, and the corresponding free
///       functions) assume the two operands are **independent**.
///       Self-operations (e.g., `a += a`) are syntactically supported, but
///       the covariance is propagated as if operating on two independent
///       estimates that happen to have the same value and covariance — the
///       result does **not** account for any correlation that would arise
///       from using the same underlying data twice.
///
/// @note This class resides in `namespace impl` and is the internal base.
///       Public users should instantiate `Estimate<K, C>` or its specializations.
/// @see Estimate, EstimatePOD, StatisticBase
template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
class EstimateBase {
    template<typename AOther, int L, CovarianceOption D>
        requires GoodStatisticDimension<L>::value
    friend class EstimateBase;

public:
    /// @brief Whether the dimension is known at compile time.
    using StaticDimension = std::bool_constant<K != Eigen::Dynamic>;
    /// @brief Whether the full covariance matrix is stored.
    using FullCovariance = std::bool_constant<C == CovarianceOption::Full>;
    /// @brief The Eigen vector type for the value (@f$\mathbb{R}^K@f$).
    using ValueType = Eigen::Vector<double, K>;
    /// @brief The Eigen matrix type for the covariance.
    ///        Full @f$K\times K@f$ matrix when covariance is enabled, diagonal-only otherwise.
    using CovarianceType = std::conditional_t<FullCovariance{}, Eigen::Matrix<double, K, K>, Eigen::DiagonalMatrix<double, K>>;
    /// @brief The POD type used for MPI operation and Base64.
    using PODType = EstimatePOD<K, C>;

private:
    /// @brief Construct with uninitialized matrices.
    EstimateBase(std::monostate);

protected:
    /// @brief Construct from value vector and covariance matrix.
    /// @param x Value vector
    /// @param cov Covariance matrix
    template<typename AVec, typename ACov>
        requires(std::assignable_from<ValueType&, const AVec&> and
                 std::assignable_from<CovarianceType&, const ACov&>)
    EstimateBase(const Eigen::MatrixBase<AVec>& x, const Eigen::EigenBase<ACov>& cov);

public:
    /// @brief Default constructor. Zero-initializes value and covariance.
    EstimateBase()
        requires(K != Eigen::Dynamic);
    /// @brief Move constructor.
    ///
    /// The source object is left in an undefined state.
    /// @warning Accessing any data or calling any operation (including
    /// `Fill`, `operator+=`) on a moved-from object is undefined behavior.
    /// @note A moved-from object is safe to assign a new value via
    /// assignment operators or `FromPOD()`.
    EstimateBase(EstimateBase&&) noexcept = default;
    /// @brief Copy constructor.
    EstimateBase(const EstimateBase& other);
    /// @brief Move assignment.
    ///
    /// The source object is left in an undefined state.
    /// @warning Accessing any data or calling any operation (including
    /// `Fill`, `operator+=`) on a moved-from object is undefined behavior.
    /// @note A moved-from object is safe to assign a new value via
    /// assignment operators or `FromPOD()`.
    auto operator=(EstimateBase&&) noexcept -> EstimateBase& = default;
    /// @brief Copy assignment.
    auto operator=(const EstimateBase& other) -> EstimateBase& { return CopyFrom(other); }

    /// @brief Construct with a given dimension.
    /// @param dim Number of components (must be positive)
    /// @throws std::invalid_argument if `dim <= 0`
    explicit EstimateBase(int dim)
        requires(K == Eigen::Dynamic);
    /// @brief Construct by copying state from another EstimateBase with possibly different CovarianceOption.
    /// @tparam L Dimension of the source
    /// @tparam D Covariance option of the source
    /// @param other Source estimate to copy from
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    explicit EstimateBase(const Estimate<L, D>& other);
    /// @brief Construct from POD data (e.g., from a Base64 string).
    /// @param data Previous estimate state as a plain-old-data struct
    explicit EstimateBase(const PODType& data)
        requires(K != Eigen::Dynamic);

    /// @brief Assign state from another EstimateBase with possibly different CovarianceOption.
    /// @tparam L Dimension of the source
    /// @tparam D Covariance option of the source
    /// @param other Source estimate to copy from
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto operator=(const Estimate<L, D>& other) -> ADerived& { return CopyFrom(other); }

    /// @brief Return the dimension of the value space.
    auto Dimension() const -> int;

    /// @brief i-th component of the value vector.
    /// @param i Component index (0-based)
    auto Value(int i) const -> auto { return fX[i]; }
    /// @brief Covariance between components i and j.
    /// @param i First component index (0-based)
    /// @param j Second component index (0-based)
    auto Covariance(int i, int j) const -> auto { return fCov.coeff(i, j); }
    /// @brief Correlation coefficient between components i and j.
    /// @param i First component index (0-based)
    /// @param j Second component index (0-based)
    auto Correlation(int i, int j) const -> double;
    /// @brief Variance of the i-th component, @f$\operatorname{Cov}_{ii}@f$.
    /// @param i Component index (0-based)
    auto Variance(int i) const -> auto { return Covariance(i, i); }
    /// @brief Standard deviation of the i-th component, @f$\sqrt{\operatorname{Var}_i}@f$.
    /// @param i Component index (0-based)
    auto StdDev(int i) const -> auto { return std::sqrt(Variance(i)); }
    /// @brief Uncertainty (standard deviation) of the i-th component.
    /// @param i Component index (0-based)
    auto Uncertainty(int i) const -> auto { return StdDev(i); }
    /// @brief Relative uncertainty of the i-th component, @f$\sigma_i / |v_i|@f$.
    /// @param i Component index (0-based)
    auto RelativeUncertainty(int i) const -> auto { return Uncertainty(i) / std::abs(Value(i)); }
    /// @brief Full value vector.
    auto Value() const -> const auto& { return fX; }
    /// @brief Full covariance matrix (or diagonal matrix if `C == CovarianceOption::Diagonal`).
    auto Covariance() const -> const auto& { return fCov; }
    /// @brief Full correlation matrix (or identity diagonal matrix if `C == CovarianceOption::Diagonal`).
    auto Correlation() const -> CovarianceType;
    /// @brief Variance vector.
    auto Variance() const -> auto { return VarXpr().eval(); }
    /// @brief Standard deviation vector.
    auto StdDev() const -> auto { return StdDevXpr().eval(); }
    /// @brief Per-component uncertainty (standard deviation) vector.
    auto Uncertainty() const -> auto { return StdDev(); }
    /// @brief Per-component relative uncertainty vector.
    auto RelativeUncertainty() const -> auto { return StdDevXpr().cwiseQuotient(fX.cwiseAbs()).eval(); }

    /// @name Statistical operations
    /// @{

    /// @brief In-place combination of two independent estimates using inverse-covariance weighting.
    ///
    /// Combines another estimate into @c *this:
    /// @f[
    /// \Sigma = (\Sigma_{\mathrm{self}}^{-1} + \Sigma_{\mathrm{other}}^{-1})^{-1}
    /// @f]
    /// @f[
    /// \mu = \Sigma \cdot (\Sigma_{\mathrm{self}}^{-1} \mu_{\mathrm{self}} + \Sigma_{\mathrm{other}}^{-1} \mu_{\mathrm{other}})
    /// @f]
    ///
    /// @param other Another independent estimate of the same quantity
    /// @return Reference to @c *this
    /// @note The operands are assumed independent.
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto CombineInPlace(const Estimate<L, D>& other) & -> ADerived&;

    /// @}
    /// @name `operator+=`
    /// @{

    /// @brief Addition with another estimate.
    /// @param other Another estimate
    /// @return Reference to `*this`
    /// @note The operands are assumed independent.
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto operator+=(const Estimate<L, D>& other) -> ADerived&;
    /// @brief Element-wise addition of a scalar estimate to the value.
    /// @note The operands are assumed independent.
    template<CovarianceOption D>
    auto operator+=(const Estimate<1, D>& c) -> ADerived&;
    /// @brief Element-wise addition of a vector to the value (covariance unchanged).
    template<typename AVec>
        requires(K != 1)
    auto operator+=(const Eigen::MatrixBase<AVec>& yXpr) -> ADerived&;
    /// @brief Element-wise addition of a scalar to the value (covariance unchanged).
    auto operator+=(double c) -> ADerived&;

    /// @}
    /// @name `operator-=`
    /// @{

    /// @brief Subtraction of another estimate.
    /// @param other Another estimate
    /// @return Reference to `*this`
    /// @note The operands are assumed independent.
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto operator-=(const Estimate<L, D>& other) -> ADerived&;
    /// @brief Element-wise subtraction of a scalar estimate from the value.
    /// @note The operands are assumed independent.
    template<CovarianceOption D>
    auto operator-=(const Estimate<1, D>& c) -> ADerived&;
    /// @brief Element-wise subtraction of a vector from the value (covariance unchanged).
    template<typename AVec>
        requires(K != 1)
    auto operator-=(const Eigen::MatrixBase<AVec>& yXpr) -> ADerived&;
    /// @brief Element-wise subtraction of a scalar from the value (covariance unchanged).
    auto operator-=(double c) -> ADerived&;

    /// @}
    /// @name `operator*=`
    /// @{

    /// @brief Element-wise multiplication by another estimate.
    /// @note The operands are assumed independent.
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto operator*=(const Estimate<L, D>& other) -> ADerived&;
    /// @brief Element-wise multiplication by a scalar estimate.
    /// @note The operands are assumed independent.
    template<CovarianceOption D>
    auto operator*=(const Estimate<1, D>& c) -> ADerived&;
    /// @brief Element-wise multiplication by a plain vector.
    template<typename AVec>
        requires(K != 1)
    auto operator*=(const Eigen::MatrixBase<AVec>& yXpr) -> ADerived&;
    /// @brief Multiply value by scalar (covariance scaled by @f$c^2@f$).
    /// @param c Scalar multiplier
    auto operator*=(double c) -> ADerived&;

    /// @}
    /// @name `operator/=`
    /// @{

    /// @brief Element-wise division by another estimate.
    /// @note The operands are assumed independent.
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto operator/=(const Estimate<L, D>& other) -> ADerived&;
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto operator/=(Estimate<L, D>&& other) -> ADerived&;
    /// @brief Element-wise division by a scalar estimate.
    /// @note The operands are assumed independent.
    template<CovarianceOption D>
    auto operator/=(const Estimate<1, D>& c) -> auto& { return Self() *= c.Inverse(); }
    template<CovarianceOption D>
    auto operator/=(Estimate<1, D>&& c) -> auto& { return Self() *= std::move(c).Inverse(); }
    /// @brief Element-wise division by a plain vector.
    template<typename AVec>
        requires(K != 1)
    auto operator/=(const Eigen::MatrixBase<AVec>& yXpr) -> auto& { return Self() *= yXpr.cwiseInverse(); }
    /// @brief Divide value by scalar (covariance scaled by @f$1/c^2@f$).
    /// @param c Scalar divisor
    auto operator/=(double c) -> auto& { return Self() *= 1 / c; }

    /// @}
    /// @name Negation and related operations
    /// @{

    /// @brief In-place negate: @f$x \to -x@f$ (covariance unchanged).
    auto NegateInPlace() & -> ADerived&;
    /// @brief Create a new negated estimate: @f$x \to -x@f$ (covariance unchanged).
    auto Negate() const& -> ADerived;
    auto Negate() && -> ADerived;

    /// @brief In-place negate-and-add from another estimate: @f$x \to \mathit{other}.x - x@f$.
    /// @note The operands are assumed independent.
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto NegateAddInPlace(const Estimate<L, D>& other) & -> ADerived&;
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto NegateAdd(const Estimate<L, D>& other) const& -> ADerived;
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto NegateAdd(const Estimate<L, D>& other) && -> ADerived;

    /// @brief In-place negate-and-add from a scalar estimate: @f$x \to \mathit{c}.x - x@f$.
    /// @note The operands are assumed independent.
    template<CovarianceOption D>
    auto NegateAddInPlace(const Estimate<1, D>& c) & -> ADerived&;
    template<CovarianceOption D>
    auto NegateAdd(const Estimate<1, D>& c) const& -> ADerived;
    template<CovarianceOption D>
    auto NegateAdd(const Estimate<1, D>& c) && -> ADerived;

    /// @brief In-place negate-and-add: @f$x_i \to v_i - x_i@f$ (covariance unchanged).
    template<typename AVec>
        requires(K != 1)
    auto NegateAddInPlace(const Eigen::MatrixBase<AVec>& yXpr) & -> ADerived&;
    template<typename AVec>
        requires(K != 1)
    auto NegateAdd(const Eigen::MatrixBase<AVec>& yXpr) const& -> ADerived;
    template<typename AVec>
        requires(K != 1)
    auto NegateAdd(const Eigen::MatrixBase<AVec>& yXpr) && -> ADerived;

    /// @brief In-place negate-and-add: @f$x \to c - x@f$ (covariance unchanged).
    auto NegateAddInPlace(double c) & -> ADerived&;
    auto NegateAdd(double c) const& -> ADerived;
    auto NegateAdd(double c) && -> ADerived;

    /// @}
    /// @name Reciprocal and related operations
    /// @{

    /// @brief In-place divide from another estimate: @f$x_i \to \mathit{other}.x_i / x_i@f$.
    /// @note The operands are assumed independent.
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto DivideInPlace(const Estimate<L, D>& other) & -> ADerived&;
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto Divide(const Estimate<L, D>& other) const& -> ADerived;
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto Divide(const Estimate<L, D>& other) && -> ADerived;

    /// @brief In-place divide from a scalar estimate: @f$x_i \to \mathit{c}.x / x_i@f$.
    /// @note The operands are assumed independent.
    /// @note Rvalue overloads are intended to resolve ambiguity from Estimate<L, D> overloads.
    template<CovarianceOption D>
    auto DivideInPlace(const Estimate<1, D>& c) & -> ADerived&;
    template<CovarianceOption D>
    auto Divide(const Estimate<1, D>& c) const& -> ADerived;
    template<CovarianceOption D>
    auto Divide(const Estimate<1, D>& c) && -> ADerived;

    /// @brief In-place divide: @f$x_i \to v_i / x_i@f$.
    template<typename AVec>
        requires(K != 1)
    auto DivideInPlace(const Eigen::MatrixBase<AVec>& yXpr) & -> ADerived&;
    template<typename AVec>
        requires(K != 1)
    auto Divide(const Eigen::MatrixBase<AVec>& yXpr) const& -> ADerived;
    template<typename AVec>
        requires(K != 1)
    auto Divide(const Eigen::MatrixBase<AVec>& yXpr) && -> ADerived;

    /// @brief In-place divide: @f$x \to c / x@f$.
    auto DivideInPlace(double c) & -> ADerived&;
    auto Divide(double c) const& -> ADerived;
    auto Divide(double c) && -> ADerived;

    /// @}
    /// @name Unary math functions: Algebraic / Power
    /// @{

    /// @brief In-place square: @f$x \to x^2@f$.
    auto SquareInPlace() & -> ADerived&;
    auto Square() const& -> ADerived;
    auto Square() && -> ADerived;

    /// @brief In-place cube: @f$x \to x^3@f$.
    auto CubeInPlace() & -> ADerived&;
    auto Cube() const& -> ADerived;
    auto Cube() && -> ADerived;

    /// @brief In-place sqrt: @f$x \to \sqrt{x}@f$.
    auto SqrtInPlace() & -> ADerived&;
    auto Sqrt() const& -> ADerived;
    auto Sqrt() && -> ADerived;

    /// @brief In-place cbrt: @f$x \to \sqrt[3]{x}@f$.
    auto CbrtInPlace() & -> ADerived&;
    auto Cbrt() const& -> ADerived;
    auto Cbrt() && -> ADerived;

    /// @brief In-place reciprocal square root: @f$x \to 1/\sqrt{x}@f$.
    auto RsqrtInPlace() & -> ADerived&;
    auto Rsqrt() const& -> ADerived;
    auto Rsqrt() && -> ADerived;

    /// @brief In-place element-wise reciprocal: @f$x \to 1/x@f$.
    auto InverseInPlace() & -> ADerived&;
    auto Inverse() const& -> ADerived;
    auto Inverse() && -> ADerived;

    /// @}
    /// @name Unary math functions: Piecewise
    /// @{

    /// @brief In-place element-wise absolute value: @f$x \to |x|@f$.
    auto AbsInPlace() & -> ADerived&;
    auto Abs() const& -> ADerived;
    auto Abs() && -> ADerived;

    /// @}
    /// @name Unary math functions: Exponential
    /// @{

    /// @brief In-place exp: @f$x \to \exp(x)@f$.
    auto ExpInPlace() & -> ADerived&;
    auto Exp() const& -> ADerived;
    auto Exp() && -> ADerived;

    /// @brief In-place exp2: @f$x \to 2^x@f$.
    auto Exp2InPlace() & -> ADerived&;
    auto Exp2() const& -> ADerived;
    auto Exp2() && -> ADerived;

    /// @brief In-place expm1: @f$x \to \exp(x)-1@f$.
    auto Expm1InPlace() & -> ADerived&;
    auto Expm1() const& -> ADerived;
    auto Expm1() && -> ADerived;

    /// @brief In-place logistic (sigmoid): @f$x \to 1/(1+\exp(-x))@f$.
    auto LogisticInPlace() & -> ADerived&;
    auto Logistic() const& -> ADerived;
    auto Logistic() && -> ADerived;

    /// @}
    /// @name Unary math functions: Logarithmic
    /// @{

    /// @brief In-place log: @f$x \to \ln(x)@f$.
    auto LogInPlace() & -> ADerived&;
    auto Log() const& -> ADerived;
    auto Log() && -> ADerived;

    /// @brief In-place log10: @f$x \to \log_{10}(x)@f$.
    auto Log10InPlace() & -> ADerived&;
    auto Log10() const& -> ADerived;
    auto Log10() && -> ADerived;

    /// @brief In-place log2: @f$x \to \log_{2}(x)@f$.
    auto Log2InPlace() & -> ADerived&;
    auto Log2() const& -> ADerived;
    auto Log2() && -> ADerived;

    /// @brief In-place log1p: @f$x \to \ln(1+x)@f$.
    auto Log1pInPlace() & -> ADerived&;
    auto Log1p() const& -> ADerived;
    auto Log1p() && -> ADerived;

    /// @}
    /// @name Unary math functions: Trigonometric
    /// @{

    /// @brief In-place sin: @f$x \to \sin(x)@f$.
    auto SinInPlace() & -> ADerived&;
    auto Sin() const& -> ADerived;
    auto Sin() && -> ADerived;

    /// @brief In-place cos: @f$x \to \cos(x)@f$.
    auto CosInPlace() & -> ADerived&;
    auto Cos() const& -> ADerived;
    auto Cos() && -> ADerived;

    /// @brief In-place tan: @f$x \to \tan(x)@f$.
    auto TanInPlace() & -> ADerived&;
    auto Tan() const& -> ADerived;
    auto Tan() && -> ADerived;

    /// @}
    /// @name Unary math functions: Inverse Trigonometric
    /// @{

    /// @brief In-place asin: @f$x \to \arcsin(x)@f$.
    auto AsinInPlace() & -> ADerived&;
    auto Asin() const& -> ADerived;
    auto Asin() && -> ADerived;

    /// @brief In-place acos: @f$x \to \arccos(x)@f$.
    auto AcosInPlace() & -> ADerived&;
    auto Acos() const& -> ADerived;
    auto Acos() && -> ADerived;

    /// @brief In-place atan: @f$x \to \arctan(x)@f$.
    auto AtanInPlace() & -> ADerived&;
    auto Atan() const& -> ADerived;
    auto Atan() && -> ADerived;

    /// @}
    /// @name Unary math functions: Hyperbolic
    /// @{

    /// @brief In-place sinh: @f$x \to \sinh(x)@f$.
    auto SinhInPlace() & -> ADerived&;
    auto Sinh() const& -> ADerived;
    auto Sinh() && -> ADerived;

    /// @brief In-place cosh: @f$x \to \cosh(x)@f$.
    auto CoshInPlace() & -> ADerived&;
    auto Cosh() const& -> ADerived;
    auto Cosh() && -> ADerived;

    /// @brief In-place tanh: @f$x \to \tanh(x)@f$.
    auto TanhInPlace() & -> ADerived&;
    auto Tanh() const& -> ADerived;
    auto Tanh() && -> ADerived;

    /// @}
    /// @name Unary math functions: Inverse Hyperbolic
    /// @{

    /// @brief In-place asinh: @f$x \to \operatorname{arsinh}(x)@f$.
    auto AsinhInPlace() & -> ADerived&;
    auto Asinh() const& -> ADerived;
    auto Asinh() && -> ADerived;

    /// @brief In-place acosh: @f$x \to \operatorname{arcosh}(x)@f$.
    auto AcoshInPlace() & -> ADerived&;
    auto Acosh() const& -> ADerived;
    auto Acosh() && -> ADerived;

    /// @brief In-place atanh: @f$x \to \operatorname{artanh}(x)@f$.
    auto AtanhInPlace() & -> ADerived&;
    auto Atanh() const& -> ADerived;
    auto Atanh() && -> ADerived;

    /// @}
    /// @name Unary math functions: Special / Transcendental
    /// @{

    /// @brief In-place error function: @f$x \to \operatorname{erf}(x)@f$.
    auto ErfInPlace() & -> ADerived&;
    auto Erf() const& -> ADerived;
    auto Erf() && -> ADerived;

    /// @brief In-place complementary error function: @f$x \to \operatorname{erfc}(x)@f$.
    auto ErfcInPlace() & -> ADerived&;
    auto Erfc() const& -> ADerived;
    auto Erfc() && -> ADerived;

    /// @brief In-place log-gamma: @f$x \to \ln|\Gamma(x)|@f$.
    auto LgammaInPlace() & -> ADerived&;
    auto Lgamma() const& -> ADerived;
    auto Lgamma() && -> ADerived;

    /// @brief In-place inverse normal CDF: @f$x \to \Phi^{-1}(x)@f$.
    auto NdtriInPlace() & -> ADerived&;
    auto Ndtri() const& -> ADerived;
    auto Ndtri() && -> ADerived;

    /// @}
    /// @name Binary math functions
    /// @{

    /// @brief In-place element-wise power: @f$x_i \to x_i^{\mathit{expo}_i}@f$.
    /// @note The operands are assumed independent.
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto PowInPlace(const Estimate<L, D>& other) & -> ADerived&;
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto Pow(const Estimate<L, D>& other) const& -> ADerived;
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto Pow(const Estimate<L, D>& other) && -> ADerived;

    /// @brief In-place element-wise power with a scalar estimate exponent: @f$x_i \to x_i^{\mathit{c}}@f$.
    /// @note The operands are assumed independent.
    template<CovarianceOption D>
    auto PowInPlace(const Estimate<1, D>& c) & -> ADerived&;
    template<CovarianceOption D>
    auto Pow(const Estimate<1, D>& c) const& -> ADerived;
    template<CovarianceOption D>
    auto Pow(const Estimate<1, D>& c) && -> ADerived;

    /// @brief In-place element-wise power: @f$x_i \to x_i^{\mathit{expo}_i}@f$.
    template<typename AVec>
        requires(K != 1)
    auto PowInPlace(const Eigen::MatrixBase<AVec>& yXpr) & -> ADerived&;
    template<typename AVec>
        requires(K != 1)
    auto Pow(const Eigen::MatrixBase<AVec>& yXpr) const& -> ADerived;
    template<typename AVec>
        requires(K != 1)
    auto Pow(const Eigen::MatrixBase<AVec>& yXpr) && -> ADerived;

    /// @brief In-place element-wise power: @f$x_i \to x_i^{\mathit{expo}}@f$.
    auto PowInPlace(double c) & -> ADerived&;
    auto Pow(double c) const& -> ADerived;
    auto Pow(double c) && -> ADerived;

    /// @brief In-place element-wise power from a base estimate: @f$x_i \to \mathit{base}_i^{x_i}@f$.
    /// @note The operands are assumed independent.
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto ExpInPlace(const Estimate<L, D>& other) & -> ADerived&;
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto Exp(const Estimate<L, D>& other) const& -> ADerived;
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto Exp(const Estimate<L, D>& other) && -> ADerived;

    /// @brief In-place element-wise power from a scalar estimate base: @f$x_i \to \mathit{c}^{x_i}@f$.
    /// @note The operands are assumed independent.
    template<CovarianceOption D>
    auto ExpInPlace(const Estimate<1, D>& c) & -> ADerived&;
    template<CovarianceOption D>
    auto Exp(const Estimate<1, D>& c) const& -> ADerived;
    template<CovarianceOption D>
    auto Exp(const Estimate<1, D>& c) && -> ADerived;

    /// @brief In-place element-wise power from a vector base: @f$x_i \to \mathit{base}_i^{x_i}@f$.
    template<typename AVec>
        requires(K != 1)
    auto ExpInPlace(const Eigen::MatrixBase<AVec>& yXpr) & -> ADerived&;
    template<typename AVec>
        requires(K != 1)
    auto Exp(const Eigen::MatrixBase<AVec>& yXpr) const& -> ADerived;
    template<typename AVec>
        requires(K != 1)
    auto Exp(const Eigen::MatrixBase<AVec>& yXpr) && -> ADerived;

    /// @brief In-place element-wise power from a scalar base: @f$x_i \to \mathit{base}^{x_i}@f$.
    auto ExpInPlace(double c) & -> ADerived&;
    auto Exp(double c) const& -> ADerived;
    auto Exp(double c) && -> ADerived;

    /// @}
    /// @name Reduction operations
    /// @brief Operations that collapse a vector estimate into a scalar estimate.
    /// @note These are member functions only (no free functions). They are not exposed
    ///       on `Estimate<1, C>` so are effectively unavailable for scalars.
    /// @{

    /// @brief Sum of all components, @f$\sum_i x_i@f$.
    /// @return A scalar estimate with value = sum of elements and properly propagated uncertainty.
    auto Sum() const -> Estimate<1, C>;

    /// @brief Product of all components, @f$\prod_i x_i@f$.
    /// @return A scalar estimate with value = product of elements and properly propagated uncertainty.
    auto Prod() const& -> auto { return ADerived{Self()}.Prod(); }
    auto Prod() && -> auto { return LogInPlace().Sum().Exp(); }

    /// @brief Squared Euclidean norm (sum of squares), @f$\sum_i x_i^2@f$.
    auto SquaredNorm() const& -> auto { return ADerived{Self()}.SquaredNorm(); }
    auto SquaredNorm() && -> auto { return SquareInPlace().Sum(); }

    /// @brief Euclidean norm (2-norm), @f$\sqrt{\sum_i x_i^2}@f$.
    auto Norm() const& -> auto { return ADerived{Self()}.Norm(); }
    auto Norm() && -> auto { return SquareInPlace().Sum().Sqrt(); }

    /// @brief @f$\ell_p@f$-norm, @f$\left(\sum_i |x_i|^p\right)^{1/p}@f$.
    /// @tparam P The power parameter (@f$p \ge 1@f$)
    template<double P>
        requires(P >= 1)
    auto LpNorm() const& -> auto { return ADerived{Self()}.template LpNorm<P>(); }
    template<double P>
        requires(P >= 1)
    auto LpNorm() && -> Estimate<1, C>;

    /// @brief Arithmetic mean, @f$\frac{1}{K}\sum_i x_i@f$.
    auto Mean() const -> auto { return Sum() / Dimension(); }

    /// @brief Harmonic mean, @f$\left(\frac{1}{K}\sum_i x_i^{-1}\right)^{-1}@f$.
    auto HarmonicMean() const& -> auto { return ADerived{Self()}.HarmonicMean(); }
    auto HarmonicMean() && -> auto { return InverseInPlace().Mean().Inverse(); }

    /// @brief Geometric mean, @f$\exp\left(\frac{1}{K}\sum_i \ln x_i\right)@f$.
    auto GeometricMean() const& -> auto { return ADerived{Self()}.GeometricMean(); }
    auto GeometricMean() && -> auto { return LogInPlace().Mean().Exp(); }

    /// @brief Quadratic mean (RMS), @f$\sqrt{\frac{1}{K}\sum_i x_i^2}@f$.
    auto QuadraticMean() const& -> auto { return ADerived{Self()}.QuadraticMean(); }
    auto QuadraticMean() && -> auto { return SquareInPlace().Mean().Sqrt(); }

    /// @brief Cubic mean, @f$\sqrt[3]{\frac{1}{K}\sum_i x_i^3}@f$.
    auto CubicMean() const& -> auto { return ADerived{Self()}.CubicMean(); }
    auto CubicMean() && -> auto { return CubeInPlace().Mean().Cbrt(); }

    /// @}
    /// @name Dot product and related operations
    /// @{

    /// @brief Inner product of two vector estimates.
    /// @return A scalar estimate with value = self·other and properly propagated uncertainty.
    /// @note The operands are assumed independent.
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto Dot(const Estimate<L, D>& other) const -> Estimate<1, C>;
    /// @brief Inner product with a plain vector.
    /// @return A scalar estimate with value = self·v and properly propagated uncertainty.
    template<typename AVec>
        requires(AVec::ColsAtCompileTime == 1)
    auto Dot(const Eigen::MatrixBase<AVec>& yXpr) const -> Estimate<1, C>;

    /// @brief Cosine of the angle between two vectors, @f$\frac{x \cdot y}{|x|\,|y|}@f$.
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto Cosine(const Estimate<L, D>& other) const -> Estimate<1, C>;
    /// @brief Cosine with a plain vector (no uncertainty on @p yXpr).
    template<typename AVec>
        requires(AVec::ColsAtCompileTime == 1)
    auto Cosine(const Eigen::MatrixBase<AVec>& yXpr) const -> Estimate<1, C>;

    /// @brief Angle between two vectors, @f$\arccos(\frac{x \cdot y}{|x|\,|y|})@f$, in radians.
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto Angle(const Estimate<L, D>& other) const -> auto { return Cosine(other).Acos(); }
    /// @brief Angle with a plain vector, in radians.
    template<typename AVec>
        requires(AVec::ColsAtCompileTime == 1)
    auto Angle(const Eigen::MatrixBase<AVec>& yXpr) const -> auto { return Cosine(yXpr).Acos(); }

    /// @brief Scalar projection of @c *this onto @p other, @f$\frac{x \cdot y}{|y|}@f$.
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto ScalarProjTo(const Estimate<L, D>& other) const -> Estimate<1, C>;
    /// @brief Scalar projection of @c *this onto a plain vector (no uncertainty on @p yXpr).
    template<typename AVec>
        requires(AVec::ColsAtCompileTime == 1)
    auto ScalarProjTo(const Eigen::MatrixBase<AVec>& yXpr) const -> Estimate<1, C>;

    /// @brief Scalar projection of @p other onto @c *this, @f$\frac{y \cdot x}{|x|}@f$.
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto ScalarProjFrom(const Estimate<L, D>& other) const -> auto { return other.ScalarProjTo(Self()); }
    /// @brief Scalar projection of @p yXpr onto @c *this, @f$\frac{y \cdot x}{|x|}@f$.
    template<typename AVec>
        requires(AVec::ColsAtCompileTime == 1)
    auto ScalarProjFrom(const Eigen::MatrixBase<AVec>& yXpr) const -> Estimate<1, C>;

    /// @}
    /// @name Vector projection
    /// @brief Project one vector onto another, producing a vector estimate.
    /// @note The operands are assumed independent.
    /// @{

    /// @brief In-place projection of @c *this onto @p other.
    ///
    /// Replaces @c *this with the projection onto @p other:
    /// @f$x \gets \frac{x \cdot y}{\|y\|^2} y@f$.
    /// @param other The vector to project onto
    /// @return Reference to @c *this
    /// @note The operands are assumed independent.
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto ProjToInPlace(const Estimate<L, D>& other) & -> ADerived&;
    /// @brief In-place projection of @c *this onto a plain vector @p yXpr.
    template<typename AVec>
        requires(K != 1)
    auto ProjToInPlace(const Eigen::MatrixBase<AVec>& yXpr) & -> ADerived&;

    /// @brief Projection of @c *this onto @p other, returning a new estimate.
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto ProjTo(const Estimate<L, D>& other) const& -> ADerived;
    /// @brief Moving projection of @c *this onto @p other.
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto ProjTo(const Estimate<L, D>& other) && -> ADerived;
    /// @brief Projection onto a plain vector @p yXpr, returning a new estimate.
    template<typename AVec>
        requires(K != 1)
    auto ProjTo(const Eigen::MatrixBase<AVec>& yXpr) const& -> ADerived;
    /// @brief Moving projection onto a plain vector @p yXpr.
    template<typename AVec>
        requires(K != 1)
    auto ProjTo(const Eigen::MatrixBase<AVec>& yXpr) && -> ADerived;

    /// @brief In-place projection of @p other onto @c *this.
    ///
    /// Replaces @c *this with the projection of @p other onto @c *this:
    /// @f$x \gets \frac{y \cdot x}{\|x\|^2} x@f$.
    /// @param other The vector to project onto @c *this
    /// @return Reference to @c *this
    /// @note The operands are assumed independent.
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto ProjFromInPlace(const Estimate<L, D>& other) & -> ADerived&;
    /// @brief In-place projection of a plain vector @p yXpr onto @c *this.
    template<typename AVec>
        requires(K != 1)
    auto ProjFromInPlace(const Eigen::MatrixBase<AVec>& yXpr) & -> ADerived&;

    /// @brief Projection of @p other onto @c *this, returning a new estimate.
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto ProjFrom(const Estimate<L, D>& other) const& -> ADerived;
    /// @brief Moving projection of @p other onto @c *this.
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto ProjFrom(const Estimate<L, D>& other) && -> ADerived;
    /// @brief Projection of a plain vector @p yXpr onto @c *this, returning a new estimate.
    template<typename AVec>
        requires(K != 1)
    auto ProjFrom(const Eigen::MatrixBase<AVec>& yXpr) const& -> ADerived;
    /// @brief Moving projection of a plain vector @p yXpr onto @c *this.
    template<typename AVec>
        requires(K != 1)
    auto ProjFrom(const Eigen::MatrixBase<AVec>& yXpr) && -> ADerived;

    /// @}
    /// @name Matrix-vector product
    /// @{

    /// @brief Left-multiply: returns @f$A x@f$ as a new estimate.
    /// @param aXpr A @f$M\times K@f$ matrix (Eigen expression)
    /// @return A new `Estimate<M, C>` with value @f$A \mu@f$ and covariance @f$A \Sigma A^\mathsf{T}@f$.
    template<typename AMat>
        requires(AMat::ColsAtCompileTime == K or
                 K == Eigen::Dynamic or AMat::RowsAtCompileTime == Eigen::Dynamic)
    auto LeftMultiply(const Eigen::MatrixBase<AMat>& aXpr) const -> Estimate<AMat::RowsAtCompileTime, C>;

    /// @brief Right-multiply: returns @f$x^\mathsf{T} A@f$, stored as @f$A^\mathsf{T} x@f$.
    /// @param aXpr A @f$K\times M@f$ matrix (Eigen expression)
    /// @return A new `Estimate<M, C>` with value @f$A^\mathsf{T} \mu@f$ and covariance @f$A^\mathsf{T} \Sigma A@f$.
    /// @note Equivalent to @f$\texttt{LeftMultiply}(A^\mathsf{T})@f$.
    template<typename AMat>
        requires(AMat::RowsAtCompileTime == K or
                 K == Eigen::Dynamic or AMat::RowsAtCompileTime == Eigen::Dynamic)
    auto RightMultiply(const Eigen::MatrixBase<AMat>& aXpr) const -> auto { return LeftMultiply(aXpr.transpose()); }

    /// @}
    /// @name Normalization
    /// @brief Normalize the vector to unit length with proper uncertainty propagation.
    /// @note Only available for vector estimates (@f$K \neq 1@f$).
    /// @{

    /// @brief In-place normalization: @f$x \to x / \|x\|@f$.
    /// The covariance matrix is updated via the Delta method:
    /// @f$\operatorname{Cov} \leftarrow J \operatorname{Cov} J^\mathsf{T}@f$,
    /// where @f$J = (I - \mu\mu^\mathsf{T}/\|\mu\|^2) / \|\mu\|@f$.
    /// If the vector length squared is close to zero (as determined by `muc::isclose`),
    /// the estimate is returned unchanged.
    auto Normalize() & -> ADerived&;
    /// @brief Return a normalized copy: @f$x \to x / \|x\|@f$.
    auto Normalized() const& -> ADerived;
    auto Normalized() && -> ADerived;

    /// @}
    /// @name Block operations
    /// @{

    /// @brief Concatenate two estimates: value vectors are concatenated,
    ///        covariance matrices arranged block-diagonally.
    template<int L, CovarianceOption D>
        requires GoodStatisticDimension<L>::value
    auto Concat(const Estimate<L, D>& other) const -> EstimateConcatResult<K, C, L, D>;

    /// @brief First n components with corresponding sub-covariance.
    auto Head(int n) const -> Estimate<Eigen::Dynamic, C>
        requires(K != 1);
    /// @brief First N components (compile-time dimension).
    template<int N>
        requires(K != 1 and N > 0 and (N <= K or K == Eigen::Dynamic))
    auto Head() const -> Estimate<N, C>;

    /// @brief Last n components with corresponding sub-covariance.
    auto Tail(int n) const -> Estimate<Eigen::Dynamic, C>
        requires(K != 1);
    /// @brief Last N components (compile-time dimension).
    template<int N>
        requires(K != 1 and N > 0 and (N <= K or K == Eigen::Dynamic))
    auto Tail() const -> Estimate<N, C>;

    /// @brief n components starting at position i.
    auto Segment(int i, int n) const -> Estimate<Eigen::Dynamic, C>
        requires(K != 1);
    /// @brief N components starting at position i (compile-time dimension).
    template<int N>
        requires(K != 1 and N > 0 and (N <= K or K == Eigen::Dynamic))
    auto Segment(int i) const -> Estimate<N, C>;

    /// @}

    /// @brief Convert the current state to a plain-old-data struct for MPI reduction and Base64 encoding.
    /// @return A `EstimatePOD<K, C>` containing copies of value and covariance.
    /// @note Only available when the dimension is known at compile time.
    ///       The `EstimatePOD` type is introspected by `MPLR_REFLECTION_TEMPLATE`
    ///       for use with MPI collective operations.
    auto ToPOD() const -> PODType
        requires(K != Eigen::Dynamic);
    /// @brief Restore state from a previously saved POD snapshot (e.g., from MPI reduction or Base64 decoding).
    /// @param data The POD data to restore from
    /// @note Only available when the dimension is known at compile time.
    auto FromPOD(const PODType& data) -> void
        requires(K != Eigen::Dynamic);

private:
    /// @brief Value vector as an Eigen array (unevaluated expression).
    auto ArrX() -> auto { return fX.array(); }
    /// @brief Value vector as an Eigen array (unevaluated expression).
    auto ArrX() const -> auto { return fX.array(); }
    /// @brief Variance vector (unevaluated expression for full covariance or reference for diagonal).
    auto VarXpr() -> decltype(auto) { return fCov.diagonal(); }
    /// @brief Variance vector (unevaluated expression for full covariance or reference for diagonal).
    auto VarXpr() const -> decltype(auto) { return fCov.diagonal(); }
    /// @brief Standard deviation vector (unevaluated expression).
    auto StdDevXpr() const -> auto { return VarXpr().cwiseSqrt(); }

    /// @brief Check that a plain vector has matching dimension.
    /// @throws std::invalid_argument if dimensions differ (only checked for dynamic dimension)
    template<typename AVec>
        requires((AVec::RowsAtCompileTime == K or
                  K == Eigen::Dynamic or AVec::RowsAtCompileTime == Eigen::Dynamic) and
                 AVec::ColsAtCompileTime == 1)
    auto CheckVectorDimensionMatch(const Eigen::MatrixBase<AVec>& yXpr) const -> void;

    /// @brief Check that a plain matrix has compatible row dimension (number of rows == @f$K@f$).
    /// @throws std::invalid_argument if dimensions differ (only checked for dynamic dimension)
    template<typename AMat>
        requires(AMat::RowsAtCompileTime == K or
                 K == Eigen::Dynamic or AMat::RowsAtCompileTime == Eigen::Dynamic)
    auto CheckMatrixRowDimensionMatch(const Eigen::MatrixBase<AMat>& aXpr) const -> void;

    /// @brief Check that a plain matrix has compatible column dimension (number of columns == @f$K@f$).
    /// @throws std::invalid_argument if dimensions differ (only checked for dynamic dimension)
    template<typename AMat>
        requires(AMat::ColsAtCompileTime == K or
                 K == Eigen::Dynamic or AMat::ColsAtCompileTime == Eigen::Dynamic)
    auto CheckMatrixColDimensionMatch(const Eigen::MatrixBase<AMat>& aXpr) const -> void;

    /// @brief Copy estimate state from another estimate with possibly different CovarianceOption.
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
    /// @param other Source estimate to copy from
    template<typename AOther, int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto CopyFrom(const EstimateBase<AOther, L, D>& other) & -> ADerived&;

    template<typename AVec>
        requires(AVec::ColsAtCompileTime == 1)
    auto CovBilinearForm(const Eigen::DenseBase<AVec>& uXpr) const -> double;
    template<typename AVec>
        requires(AVec::ColsAtCompileTime == 1)
    auto CovRankUpdate(double c, const Eigen::DenseBase<AVec>& uXpr) -> void;
    template<typename AVecU, typename AVecV>
        requires(AVecU::ColsAtCompileTime == 1 and AVecV::ColsAtCompileTime == 1)
    auto CovRankUpdate(double c, const Eigen::DenseBase<AVecU>& uXpr, const Eigen::DenseBase<AVecV>& vXpr) -> void;
    template<typename AJac>
        requires(AJac::ColsAtCompileTime == 1)
    auto CovCwiseUnaryUpdate(const Eigen::DenseBase<AJac>& diagJacXpr) -> void;
    template<int L, CovarianceOption D, typename AJacX, typename AJacY>
        requires(AJacX::ColsAtCompileTime == 1 and AJacY::ColsAtCompileTime == 1)
    auto CovCwiseBinaryUpdate(const Estimate<L, D>& other,
                              const Eigen::DenseBase<AJacX>& diagJacXXpr,
                              const Eigen::DenseBase<AJacY>& diagJacYXpr) -> void;

    auto Self() & -> auto&& { return static_cast<ADerived&>(*this); }
    auto Self() const& -> auto&& { return static_cast<const ADerived&>(*this); }
    auto Self() && -> auto&& { return static_cast<ADerived&&>(*this); }
    auto Self() const&& -> auto&& { return static_cast<const ADerived&&>(*this); }

private:
    ValueType fX;        ///< Value vector
    CovarianceType fCov; ///< Covariance matrix (reduces to scalar variance when K=1)
};

} // namespace impl

/// @brief Scalar (1-dimensional) specialization of Estimate.
///
/// Provides scalar `double` accessors instead of Eigen vector/matrix types,
/// making the interface more convenient for single-value estimates.
///
/// @tparam C Covariance option (`CovarianceOption::Full` or `Diagonal`)
///
/// @see EstimateBase, EstimatePOD, Statistic
template<CovarianceOption C>
class Estimate<1, C> : protected impl::EstimateBase<Estimate<1, C>, 1, C> {
    template<typename AOther, int L, CovarianceOption D>
        requires impl::GoodStatisticDimension<L>::value
    friend class impl::EstimateBase;

private:
    using Base = impl::EstimateBase<Estimate<1, C>, 1, C>;

public:
    /// @brief Whether the dimension is static (always true for this specialization).
    using typename Base::StaticDimension;
    /// @brief Whether full covariance matrix is stored.
    using typename Base::FullCovariance;
    /// @brief POD type for serialization.
    using typename Base::PODType;
    /// @brief Value type is plain `double` for the scalar case.
    using ValueType = double;
    /// @brief Covariance type is plain `double` for the scalar case.
    using CovarianceType = double;

public:
    /// @brief Inherit constructors from the base class.
    using Base::Base;
    /// @brief Construct from value and variance.
    /// @param x The value
    /// @param var The variance; stored directly
    Estimate(double x, double var);

    /// @brief Inherit assignment operators from the base class.
    using Base::operator=;

    /// @brief Return the dimension of the value space. Always returns 1.
    using Base::Dimension;

    /// @brief The value, @f$x@f$.
    auto Value() const -> auto { return Base::Value(0); }
    /// @brief The variance (identical to Covariance() for scalar).
    auto Variance() const -> auto { return Base::Variance(0); }
    /// @brief The standard deviation (uncertainty), @f$\sigma@f$.
    auto StdDev() const -> auto { return Base::StdDev(0); }
    /// @brief The uncertainty (standard deviation), @f$\sigma@f$.
    auto Uncertainty() const -> auto { return Base::Uncertainty(0); }
    /// @brief Relative uncertainty, @f$\sigma / |v|@f$.
    auto RelativeUncertainty() const -> auto { return Base::RelativeUncertainty(0); }

    /// @name Compound assignment operators
    /// @{
    using Base::operator+=;
    using Base::operator-=;
    using Base::operator*=;
    using Base::operator/=;
    /// @}

    /// @name Math functions
    /// @{
    using Base::Abs;
    using Base::AbsInPlace;
    using Base::Acos;
    using Base::Acosh;
    using Base::AcoshInPlace;
    using Base::AcosInPlace;
    using Base::Asin;
    using Base::Asinh;
    using Base::AsinhInPlace;
    using Base::AsinInPlace;
    using Base::Atan;
    using Base::Atanh;
    using Base::AtanhInPlace;
    using Base::AtanInPlace;
    using Base::Cbrt;
    using Base::CbrtInPlace;
    using Base::CombineInPlace;
    using Base::Concat;
    using Base::Cos;
    using Base::Cosh;
    using Base::CoshInPlace;
    using Base::CosInPlace;
    using Base::Cube;
    using Base::CubeInPlace;
    using Base::Divide;
    using Base::DivideInPlace;
    using Base::Erf;
    using Base::Erfc;
    using Base::ErfcInPlace;
    using Base::ErfInPlace;
    using Base::Exp;
    using Base::Exp2;
    using Base::Exp2InPlace;
    using Base::ExpInPlace;
    using Base::Expm1;
    using Base::Expm1InPlace;
    using Base::Inverse;
    using Base::InverseInPlace;
    using Base::Lgamma;
    using Base::LgammaInPlace;
    using Base::Log;
    using Base::Log10;
    using Base::Log10InPlace;
    using Base::Log1p;
    using Base::Log1pInPlace;
    using Base::Log2;
    using Base::Log2InPlace;
    using Base::LogInPlace;
    using Base::Logistic;
    using Base::LogisticInPlace;
    using Base::Ndtri;
    using Base::NdtriInPlace;
    using Base::Negate;
    using Base::NegateAdd;
    using Base::NegateAddInPlace;
    using Base::NegateInPlace;
    using Base::Pow;
    using Base::PowInPlace;
    using Base::Rsqrt;
    using Base::RsqrtInPlace;
    using Base::Sin;
    using Base::Sinh;
    using Base::SinhInPlace;
    using Base::SinInPlace;
    using Base::Sqrt;
    using Base::SqrtInPlace;
    using Base::Square;
    using Base::SquareInPlace;
    using Base::Tan;
    using Base::Tanh;
    using Base::TanhInPlace;
    using Base::TanInPlace;
    /// @}

    /// @brief Inherit POD conversion from the base class.
    using Base::ToPOD;
    /// @brief Inherit POD restoration from the base class.
    using Base::FromPOD;
};

/// @brief @f$K@f$-dimensional Estimate (general template for @f$K \neq 1@f$).
///
/// All Eigen vectors and matrices are fixed-size at compile time when @f$K@f$ is static,
/// and dynamically sized when @f$K@f$ is `Eigen::Dynamic`,
/// giving optimal performance for the chosen approach.
///
/// @tparam K Dimension (@f$K \neq 1@f$; use `Estimate<1, C>` for the scalar case, or `Eigen::Dynamic` for runtime dimension)
/// @tparam C Covariance option
///
/// @see EstimateBase, EstimatePOD, Statistic
template<int K, CovarianceOption C>
class Estimate : public impl::EstimateBase<Estimate<K, C>, K, C> {
private:
    using Base = impl::EstimateBase<Estimate<K, C>, K, C>;

public:
    /// @brief Whether full covariance matrix is stored.
    using typename Base::FullCovariance;
    /// @brief The Eigen vector type for the value (@f$\mathbb{R}^K@f$).
    using typename Base::ValueType;
    /// @brief The Eigen matrix type for the covariance.
    ///        Full @f$K\times K@f$ matrix when covariance is enabled, diagonal-only otherwise.
    using typename Base::CovarianceType;

public:
    /// @brief Inherit constructors from the base class.
    using Base::Base;
    /// @brief Construct from value vector and covariance matrix.
    /// @param x Value vector
    /// @param cov Covariance matrix
    template<typename AVec, typename ACov>
        requires(std::assignable_from<ValueType&, const AVec&> and
                 std::assignable_from<CovarianceType&, const ACov&>)
    Estimate(const Eigen::MatrixBase<AVec>& x, const Eigen::EigenBase<ACov>& cov);

    /// @brief Inherit assignment operators from the base class.
    using Base::operator=;
};

// TODO: Estimate<2, C> specialization with extra RhoPhi(), RhoPhiInPlace(), Cross(), ...
// TODO: Estimate<3, C> specialization with extra RThetaPhi(), RThetaPhiInPlace(), Cross(), ...
// TODO: Estimate<4, C> specialization with extra Boost(), BoostInPlace(), ...

namespace impl {

/// @brief Determines the result type of a binary operation between two `Estimate` objects.
///
/// The result dimension is the compile-time K of the left operand when it is static,
/// otherwise the compile-time L of the right operand when it is static,
/// otherwise `Eigen::Dynamic` (runtime-determined).
///
/// The result covariance option is `Full` if either operand stores the full covariance
/// matrix, and `Diagonal` only if both are diagonal.
///
/// @note A `Diagonal` operand is treated as having zero off-diagonal covariance
///       elements in the computation. When the result type is `Diagonal`,
///       off-diagonal elements are discarded from the computed result.
///
/// @tparam K Left-hand dimension
/// @tparam C Left-hand covariance option
/// @tparam L Right-hand dimension
/// @tparam D Right-hand covariance option
template<int K, CovarianceOption C, int L, CovarianceOption D>
using EstimateBinaryOpResult = Estimate<
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

/// @name Estimate operators
/// @{

/// @name Unary operators
/// @{

template<int K, CovarianceOption C>
auto operator+(const Estimate<K, C>& est) -> auto { return est; }
template<int K, CovarianceOption C>
auto operator+(Estimate<K, C>&& est) -> auto { return std::move(est); }

template<int K, CovarianceOption C>
auto operator-(const Estimate<K, C>& est) -> auto { return est.Negate(); }
template<int K, CovarianceOption C>
auto operator-(Estimate<K, C>&& est) -> auto { return std::move(est.NegateInPlace()); }

/// @}
/// @name Binary operators
/// @brief Binary arithmetic operations for `Estimate` objects.
///
/// Each operator creates a new `Estimate` result from two operands.
/// @note The operands are assumed independent.
/// @{

#define MUSTARD_MATH_ESTIMATE_ESTIMATE_BINARY_OP_DECLARATIONS(Op)                                              \
    template<int K, CovarianceOption C, int L, CovarianceOption D>                                             \
    auto Op(const Estimate<K, C>& lhs, const Estimate<L, D>& rhs) -> impl::EstimateBinaryOpResult<K, C, L, D>; \
                                                                                                               \
    template<int K, CovarianceOption C, int L, CovarianceOption D>                                             \
    auto Op(const Estimate<K, C>& lhs, Estimate<L, D>&& rhs) -> impl::EstimateBinaryOpResult<K, C, L, D>;      \
                                                                                                               \
    template<int K, CovarianceOption C, int L, CovarianceOption D>                                             \
    auto Op(Estimate<K, C>&& lhs, const Estimate<L, D>& rhs) -> impl::EstimateBinaryOpResult<K, C, L, D>;      \
                                                                                                               \
    template<int K, CovarianceOption C, int L, CovarianceOption D>                                             \
    auto Op(Estimate<K, C>&& lhs, Estimate<L, D>&& rhs) -> impl::EstimateBinaryOpResult<K, C, L, D>;

#define MUSTARD_MATH_ESTIMATE_VECTOR_BINARY_OP_DECLARATIONS(Op)                               \
    template<int K, CovarianceOption C, typename AVec>                                        \
        requires(K != 1 and AVec::ColsAtCompileTime == 1)                                     \
    auto Op(const Estimate<K, C>& lhs, const Eigen::MatrixBase<AVec>& rhs) -> Estimate<K, C>; \
                                                                                              \
    template<int K, CovarianceOption C, typename AVec>                                        \
        requires(K != 1 and AVec::ColsAtCompileTime == 1)                                     \
    auto Op(Estimate<K, C>&& lhs, const Eigen::MatrixBase<AVec>& rhs) -> Estimate<K, C>;      \
                                                                                              \
    template<typename AVec, int K, CovarianceOption C>                                        \
        requires(K != 1 and AVec::ColsAtCompileTime == 1)                                     \
    auto Op(const Eigen::MatrixBase<AVec>& lhs, const Estimate<K, C>& rhs) -> Estimate<K, C>; \
                                                                                              \
    template<typename AVec, int K, CovarianceOption C>                                        \
        requires(K != 1 and AVec::ColsAtCompileTime == 1)                                     \
    auto Op(const Eigen::MatrixBase<AVec>& lhs, Estimate<K, C>&& rhs) -> Estimate<K, C>;

#define MUSTARD_MATH_ESTIMATE_SCALAR_BINARY_OP_DECLARATIONS(Op)       \
    template<int K, CovarianceOption C>                               \
    auto Op(const Estimate<K, C>& lhs, double rhs) -> Estimate<K, C>; \
                                                                      \
    template<int K, CovarianceOption C>                               \
    auto Op(Estimate<K, C>&& lhs, double rhs) -> Estimate<K, C>;      \
                                                                      \
    template<int K, CovarianceOption C>                               \
    auto Op(double lhs, const Estimate<K, C>& rhs) -> Estimate<K, C>; \
                                                                      \
    template<int K, CovarianceOption C>                               \
    auto Op(double lhs, Estimate<K, C>&& rhs) -> Estimate<K, C>;

MUSTARD_MATH_ESTIMATE_ESTIMATE_BINARY_OP_DECLARATIONS(Combine)

MUSTARD_MATH_ESTIMATE_ESTIMATE_BINARY_OP_DECLARATIONS(operator+)
MUSTARD_MATH_ESTIMATE_VECTOR_BINARY_OP_DECLARATIONS(operator+)
MUSTARD_MATH_ESTIMATE_SCALAR_BINARY_OP_DECLARATIONS(operator+)

MUSTARD_MATH_ESTIMATE_ESTIMATE_BINARY_OP_DECLARATIONS(operator-)
MUSTARD_MATH_ESTIMATE_VECTOR_BINARY_OP_DECLARATIONS(operator-)
MUSTARD_MATH_ESTIMATE_SCALAR_BINARY_OP_DECLARATIONS(operator-)

MUSTARD_MATH_ESTIMATE_ESTIMATE_BINARY_OP_DECLARATIONS(operator*)
MUSTARD_MATH_ESTIMATE_VECTOR_BINARY_OP_DECLARATIONS(operator*)
MUSTARD_MATH_ESTIMATE_SCALAR_BINARY_OP_DECLARATIONS(operator*)

MUSTARD_MATH_ESTIMATE_ESTIMATE_BINARY_OP_DECLARATIONS(operator/)
MUSTARD_MATH_ESTIMATE_VECTOR_BINARY_OP_DECLARATIONS(operator/)
MUSTARD_MATH_ESTIMATE_SCALAR_BINARY_OP_DECLARATIONS(operator/)

MUSTARD_MATH_ESTIMATE_ESTIMATE_BINARY_OP_DECLARATIONS(pow)
MUSTARD_MATH_ESTIMATE_VECTOR_BINARY_OP_DECLARATIONS(pow)
MUSTARD_MATH_ESTIMATE_SCALAR_BINARY_OP_DECLARATIONS(pow)

MUSTARD_MATH_ESTIMATE_ESTIMATE_BINARY_OP_DECLARATIONS(Project)

#undef MUSTARD_MATH_ESTIMATE_ESTIMATE_BINARY_OP_DECLARATIONS
#undef MUSTARD_MATH_ESTIMATE_VECTOR_BINARY_OP_DECLARATIONS
#undef MUSTARD_MATH_ESTIMATE_SCALAR_BINARY_OP_DECLARATIONS

/// @}
/// @name Element-wise math functions
/// @brief Each has a const& overload (copies, transforms, returns by value) and
/// an && overload (moves, transforms in place, returns by value).
/// @{

#define MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(func, Func, FuncInPlace) \
    template<int K, CovarianceOption C>                                                \
    auto func(const Estimate<K, C>& est) -> auto { return est.Func(); }                \
                                                                                       \
    template<int K, CovarianceOption C>                                                \
    auto func(Estimate<K, C>&& est) -> auto { return std::move(est.FuncInPlace()); }

MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(square, Square, SquareInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(cube, Cube, CubeInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(sqrt, Sqrt, SqrtInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(cbrt, Cbrt, CbrtInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(rsqrt, Rsqrt, RsqrtInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(inverse, Inverse, InverseInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(abs, Abs, AbsInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(exp, Exp, ExpInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(exp2, Exp2, Exp2InPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(expm1, Expm1, Expm1InPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(log, Log, LogInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(log10, Log10, Log10InPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(log2, Log2, Log2InPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(log1p, Log1p, Log1pInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(sin, Sin, SinInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(cos, Cos, CosInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(tan, Tan, TanInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(asin, Asin, AsinInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(acos, Acos, AcosInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(atan, Atan, AtanInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(sinh, Sinh, SinhInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(cosh, Cosh, CoshInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(tanh, Tanh, TanhInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(asinh, Asinh, AsinhInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(acosh, Acosh, AcoshInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(atanh, Atanh, AtanhInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(logistic, Logistic, LogisticInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(erf, Erf, ErfInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(erfc, Erfc, ErfcInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(lgamma, Lgamma, LgammaInPlace)
MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS(ndtri, Ndtri, NdtriInPlace)
#undef MUSTARD_MATH_ESTIMATE_CWISE_MATH_FUNCTION_DEFINITIONS

/// @}
/// @name Dot product and related operations
/// @{

#define MUSTARD_MATH_ESTIMATE_DOT_LIKE_OP_DEFINITIONS(Op, FwdOp, BwdOp)                                       \
    template<int K, CovarianceOption C, int L, CovarianceOption D>                                            \
        requires(K != 1)                                                                                      \
    auto Op(const Estimate<K, C>& lhs, const Estimate<L, D>& rhs) -> auto { return lhs.FwdOp(rhs); }          \
                                                                                                              \
    template<int K, CovarianceOption C, typename AVec>                                                        \
        requires(K != 1)                                                                                      \
    auto Op(const Estimate<K, C>& lhs, const Eigen::MatrixBase<AVec>& rhs) -> auto { return lhs.FwdOp(rhs); } \
                                                                                                              \
    template<typename AVec, int K, CovarianceOption C>                                                        \
        requires(K != 1)                                                                                      \
    auto Op(const Eigen::MatrixBase<AVec>& lhs, const Estimate<K, C>& rhs) -> auto { return rhs.BwdOp(lhs); }

MUSTARD_MATH_ESTIMATE_DOT_LIKE_OP_DEFINITIONS(Dot, Dot, Dot)
MUSTARD_MATH_ESTIMATE_DOT_LIKE_OP_DEFINITIONS(Cosine, Cosine, Cosine)
MUSTARD_MATH_ESTIMATE_DOT_LIKE_OP_DEFINITIONS(Angle, Angle, Angle)
MUSTARD_MATH_ESTIMATE_DOT_LIKE_OP_DEFINITIONS(ScalarProj, ScalarProjTo, ScalarProjFrom)
#undef MUSTARD_MATH_ESTIMATE_DOT_LIKE_OP_DEFINITIONS

/// @}
/// @name Matrix-vector operator*
/// @brief `A * est` (left-multiply) and `est * A` (right-multiply).
/// @note Uses @f$\texttt{AMat::ColsAtCompileTime} \neq 1@f$ to avoid ambiguity
///       with the element-wise vector-Estimate `operator*`.
/// @{

template<typename AMat, int K, CovarianceOption C>
    requires(K != 1 and AMat::ColsAtCompileTime != 1 and
             (AMat::ColsAtCompileTime == K or K == Eigen::Dynamic or AMat::RowsAtCompileTime == Eigen::Dynamic))
auto operator*(const Eigen::MatrixBase<AMat>& aXpr, const Estimate<K, C>& est) -> auto { return est.LeftMultiply(aXpr); }

template<int K, CovarianceOption C, typename AMat>
    requires(K != 1 and AMat::ColsAtCompileTime != 1 and
             (AMat::RowsAtCompileTime == K or K == Eigen::Dynamic or AMat::RowsAtCompileTime == Eigen::Dynamic))
auto operator*(const Estimate<K, C>& est, const Eigen::MatrixBase<AMat>& aXpr) -> auto { return est.RightMultiply(aXpr); }

/// @}

/// @brief POD (plain-old-data) struct holding a snapshot of `Estimate` state.
///
/// The primary purpose of this struct is MPI operation (enabled by the
/// `MPLR_REFLECTION_TEMPLATE` introspection at the bottom of this header)
/// and Base64 encoding for storage or network transfer. The contiguous
/// memory layout allows direct `memcpy` and use with MPI collective operations.
///
/// @tparam K Dimension (must be a compile-time constant, not `Eigen::Dynamic`)
/// @tparam C Covariance option
///
/// @see EstimateBase::ToPOD, EstimateBase::FromPOD, MPLR_REFLECTION_TEMPLATE
template<int K, CovarianceOption C>
    requires(impl::GoodStatisticDimension<K>{}() and K != Eigen::Dynamic)
struct EstimatePOD<K, C> {
    double x[K];                             ///< Value vector
    double cov[C == CovarianceOption::Full ? ///< Covariance matrix (flattened if Full, variances if Diagonal)
                   K * K :
                   K];

    /// @brief Combine two estimates.
    template<int L, CovarianceOption D>
    auto Combine(const EstimatePOD<L, D>& other) -> auto { return Combine(Estimate<K, C>{*this}, Estimate<L, D>{other}).ToPOD(); }
    /// @brief Add two estimates.
    template<int L, CovarianceOption D>
    auto operator+(const EstimatePOD<L, D>& other) -> auto { return (Estimate<K, C>{*this} + Estimate<L, D>{other}).ToPOD(); }
    /// @brief Multiply two estimates element-wise.
    template<int L, CovarianceOption D>
    auto operator*(const EstimatePOD<L, D>& other) -> auto { return (Estimate<K, C>{*this} * Estimate<L, D>{other}).ToPOD(); }
};

} // namespace Mustard::inline Math

MPLR_REFLECTION_TEMPLATE((int K, Mustard::CovarianceOption C),
                         (Mustard::EstimatePOD<K, C>),
                         x, cov)

#include "Mustard/Math/Estimate.inl"
