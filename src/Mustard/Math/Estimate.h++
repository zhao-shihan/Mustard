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
///       the result type's covariance option is `Full` if either operand is `Full`
///       (see `EstimateBinaryOpResult`). However, operations that merge covariances
///       (e.g., `operator+=`, `operator-=`) only preserve off-diagonal elements
///       when *both* operands are `Full`. If either operand is `Diagonal`, any
///       off-diagonal elements from a `Full` operand are discarded — only the
///       diagonal (variances) participates in the merge.
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
    auto Value(int i) const -> auto { return fX.coeff(i); }
    /// @brief Covariance between components i and j.
    /// @param i First component index (0-based)
    /// @param j Second component index (0-based)
    auto Covariance(int i, int j) const -> auto { return fCov.coeff(i, j); }
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
    /// @brief Variance vector.
    auto Variance() const -> auto { return VarXpr().eval(); }
    /// @brief Standard deviation vector.
    auto StdDev() const -> auto { return StdDevXpr().eval(); }
    /// @brief Per-component uncertainty (standard deviation) vector.
    auto Uncertainty() const -> auto { return StdDev(); }
    /// @brief Per-component relative uncertainty vector.
    auto RelativeUncertainty() const -> auto { return StdDevXpr().cwiseQuotient(fX.cwiseAbs()).eval(); }

    /// @name `operator+=`
    /// @{

    /// @brief Addition with another estimate.
    /// @param other Another estimate
    /// @return Reference to `*this`
    /// @note The operands are assumed independent.
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto operator+=(const Estimate<L, D>& other) -> ADerived&;
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
    /// @brief Element-wise division by another estimate.
    /// @note The operands are assumed independent.
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto operator/=(Estimate<L, D>&& other) -> ADerived&;
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
    auto DivideInPlace(Estimate<L, D>&& other) & -> ADerived&;
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto Divide(const Estimate<L, D>& other) const& -> ADerived;
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto Divide(Estimate<L, D>&& other) const& -> ADerived;
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto Divide(const Estimate<L, D>& other) && -> ADerived;
    template<int L, CovarianceOption D>
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
    auto Divide(Estimate<L, D>&& other) && -> ADerived;

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
    /// @name Dot product
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
    auto Dot(const Eigen::MatrixBase<AVec>& yXpr) const -> Estimate<1, C>;

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
    auto CheckDimensionMatch(const Eigen::MatrixBase<AVec>& yXpr) const -> void;

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
    auto CopyFrom(const EstimateBase<AOther, L, D>& other) -> ADerived&;

    template<typename AJac>
        requires(AJac::ColsAtCompileTime == 1)
    auto CwiseUnaryUpdateCov(const Eigen::DenseBase<AJac>& diagJacXpr) -> void;
    template<int L, CovarianceOption D, typename AJacX, typename AJacY>
        requires(AJacX::ColsAtCompileTime == 1 and AJacY::ColsAtCompileTime == 1)
    auto CwiseBinaryUpdateCov(const Estimate<L, D>& other,
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
/// @note Although the result type may be `Full`, the actual off-diagonal covariance
///       information in the result only comes from operands that store it. A `Diagonal`
///       operand contributes only variance information to the result.
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
auto operator-(Estimate<K, C>&& est) -> auto { return std::move(est).Negate(); }

/// @}
/// @name Binary operators
/// @brief Binary arithmetic operations for `Estimate` objects.
///
/// Each operator creates a new `Estimate` result from two operands.
/// @note The operands are assumed independent.
/// @{

#define MUSTARD_MATH_ESTIMATE_BINARY_OP_DECLARATIONS(Op)                                                       \
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
    auto Op(Estimate<K, C>&& lhs, Estimate<L, D>&& rhs) -> impl::EstimateBinaryOpResult<K, C, L, D>;           \
                                                                                                               \
    template<int K, CovarianceOption C, typename AVec>                                                         \
        requires(K != 1)                                                                                       \
    auto Op(const Estimate<K, C>& lhs, const Eigen::MatrixBase<AVec>& rhs) -> Estimate<K, C>;                  \
                                                                                                               \
    template<int K, CovarianceOption C, typename AVec>                                                         \
        requires(K != 1)                                                                                       \
    auto Op(Estimate<K, C>&& lhs, const Eigen::MatrixBase<AVec>& rhs) -> Estimate<K, C>;                       \
                                                                                                               \
    template<typename AVec, int K, CovarianceOption C>                                                         \
        requires(K != 1)                                                                                       \
    auto Op(const Eigen::MatrixBase<AVec>& lhs, const Estimate<K, C>& rhs) -> Estimate<K, C>;                  \
                                                                                                               \
    template<typename AVec, int K, CovarianceOption C>                                                         \
        requires(K != 1)                                                                                       \
    auto Op(const Eigen::MatrixBase<AVec>& lhs, Estimate<K, C>&& rhs) -> Estimate<K, C>;                       \
                                                                                                               \
    template<int K, CovarianceOption C>                                                                        \
    auto Op(const Estimate<K, C>& lhs, double rhs) -> Estimate<K, C>;                                          \
                                                                                                               \
    template<int K, CovarianceOption C>                                                                        \
    auto Op(Estimate<K, C>&& lhs, double rhs) -> Estimate<K, C>;                                               \
                                                                                                               \
    template<int K, CovarianceOption C>                                                                        \
    auto Op(double lhs, const Estimate<K, C>& rhs) -> Estimate<K, C>;                                          \
                                                                                                               \
    template<int K, CovarianceOption C>                                                                        \
    auto Op(double lhs, Estimate<K, C>&& rhs) -> Estimate<K, C>;

MUSTARD_MATH_ESTIMATE_BINARY_OP_DECLARATIONS(operator+)
MUSTARD_MATH_ESTIMATE_BINARY_OP_DECLARATIONS(operator-)
MUSTARD_MATH_ESTIMATE_BINARY_OP_DECLARATIONS(operator*)
MUSTARD_MATH_ESTIMATE_BINARY_OP_DECLARATIONS(operator/)
MUSTARD_MATH_ESTIMATE_BINARY_OP_DECLARATIONS(pow)
#undef MUSTARD_MATH_ESTIMATE_BINARY_OP_DECLARATIONS

/// @}
/// @name Element-wise math functions
/// @brief Each has a const& overload (copies, transforms, returns by value) and
/// an && overload (moves, transforms in place, returns by value).
/// @{

template<int K, CovarianceOption C>
auto square(const Estimate<K, C>& est) -> auto { return est.Square(); }
template<int K, CovarianceOption C>
auto square(Estimate<K, C>&& est) -> auto { return std::move(est).Square(); }

template<int K, CovarianceOption C>
auto cube(const Estimate<K, C>& est) -> auto { return est.Cube(); }
template<int K, CovarianceOption C>
auto cube(Estimate<K, C>&& est) -> auto { return std::move(est).Cube(); }

template<int K, CovarianceOption C>
auto sqrt(const Estimate<K, C>& est) -> auto { return est.Sqrt(); }
template<int K, CovarianceOption C>
auto sqrt(Estimate<K, C>&& est) -> auto { return std::move(est).Sqrt(); }

template<int K, CovarianceOption C>
auto cbrt(const Estimate<K, C>& est) -> auto { return est.Cbrt(); }
template<int K, CovarianceOption C>
auto cbrt(Estimate<K, C>&& est) -> auto { return std::move(est).Cbrt(); }

template<int K, CovarianceOption C>
auto rsqrt(const Estimate<K, C>& est) -> auto { return est.Rsqrt(); }
template<int K, CovarianceOption C>
auto rsqrt(Estimate<K, C>&& est) -> auto { return std::move(est).Rsqrt(); }

template<int K, CovarianceOption C>
auto inverse(const Estimate<K, C>& est) -> auto { return est.Inverse(); }
template<int K, CovarianceOption C>
auto inverse(Estimate<K, C>&& est) -> auto { return std::move(est).Inverse(); }

template<int K, CovarianceOption C>
auto abs(const Estimate<K, C>& est) -> auto { return est.Abs(); }
template<int K, CovarianceOption C>
auto abs(Estimate<K, C>&& est) -> auto { return std::move(est).Abs(); }

template<int K, CovarianceOption C>
auto exp(const Estimate<K, C>& est) -> auto { return est.Exp(); }
template<int K, CovarianceOption C>
auto exp(Estimate<K, C>&& est) -> auto { return std::move(est).Exp(); }

template<int K, CovarianceOption C>
auto exp2(const Estimate<K, C>& est) -> auto { return est.Exp2(); }
template<int K, CovarianceOption C>
auto exp2(Estimate<K, C>&& est) -> auto { return std::move(est).Exp2(); }

template<int K, CovarianceOption C>
auto expm1(const Estimate<K, C>& est) -> auto { return est.Expm1(); }
template<int K, CovarianceOption C>
auto expm1(Estimate<K, C>&& est) -> auto { return std::move(est).Expm1(); }

template<int K, CovarianceOption C>
auto log(const Estimate<K, C>& est) -> auto { return est.Log(); }
template<int K, CovarianceOption C>
auto log(Estimate<K, C>&& est) -> auto { return std::move(est).Log(); }

template<int K, CovarianceOption C>
auto log10(const Estimate<K, C>& est) -> auto { return est.Log10(); }
template<int K, CovarianceOption C>
auto log10(Estimate<K, C>&& est) -> auto { return std::move(est).Log10(); }

template<int K, CovarianceOption C>
auto log2(const Estimate<K, C>& est) -> auto { return est.Log2(); }
template<int K, CovarianceOption C>
auto log2(Estimate<K, C>&& est) -> auto { return std::move(est).Log2(); }

template<int K, CovarianceOption C>
auto log1p(const Estimate<K, C>& est) -> auto { return est.Log1p(); }
template<int K, CovarianceOption C>
auto log1p(Estimate<K, C>&& est) -> auto { return std::move(est).Log1p(); }

template<int K, CovarianceOption C>
auto sin(const Estimate<K, C>& est) -> auto { return est.Sin(); }
template<int K, CovarianceOption C>
auto sin(Estimate<K, C>&& est) -> auto { return std::move(est).Sin(); }

template<int K, CovarianceOption C>
auto cos(const Estimate<K, C>& est) -> auto { return est.Cos(); }
template<int K, CovarianceOption C>
auto cos(Estimate<K, C>&& est) -> auto { return std::move(est).Cos(); }

template<int K, CovarianceOption C>
auto tan(const Estimate<K, C>& est) -> auto { return est.Tan(); }
template<int K, CovarianceOption C>
auto tan(Estimate<K, C>&& est) -> auto { return std::move(est).Tan(); }

template<int K, CovarianceOption C>
auto asin(const Estimate<K, C>& est) -> auto { return est.Asin(); }
template<int K, CovarianceOption C>
auto asin(Estimate<K, C>&& est) -> auto { return std::move(est).Asin(); }

template<int K, CovarianceOption C>
auto acos(const Estimate<K, C>& est) -> auto { return est.Acos(); }
template<int K, CovarianceOption C>
auto acos(Estimate<K, C>&& est) -> auto { return std::move(est).Acos(); }

template<int K, CovarianceOption C>
auto atan(const Estimate<K, C>& est) -> auto { return est.Atan(); }
template<int K, CovarianceOption C>
auto atan(Estimate<K, C>&& est) -> auto { return std::move(est).Atan(); }

template<int K, CovarianceOption C>
auto sinh(const Estimate<K, C>& est) -> auto { return est.Sinh(); }
template<int K, CovarianceOption C>
auto sinh(Estimate<K, C>&& est) -> auto { return std::move(est).Sinh(); }

template<int K, CovarianceOption C>
auto cosh(const Estimate<K, C>& est) -> auto { return est.Cosh(); }
template<int K, CovarianceOption C>
auto cosh(Estimate<K, C>&& est) -> auto { return std::move(est).Cosh(); }

template<int K, CovarianceOption C>
auto tanh(const Estimate<K, C>& est) -> auto { return est.Tanh(); }
template<int K, CovarianceOption C>
auto tanh(Estimate<K, C>&& est) -> auto { return std::move(est).Tanh(); }

template<int K, CovarianceOption C>
auto asinh(const Estimate<K, C>& est) -> auto { return est.Asinh(); }
template<int K, CovarianceOption C>
auto asinh(Estimate<K, C>&& est) -> auto { return std::move(est).Asinh(); }

template<int K, CovarianceOption C>
auto acosh(const Estimate<K, C>& est) -> auto { return est.Acosh(); }
template<int K, CovarianceOption C>
auto acosh(Estimate<K, C>&& est) -> auto { return std::move(est).Acosh(); }

template<int K, CovarianceOption C>
auto atanh(const Estimate<K, C>& est) -> auto { return est.Atanh(); }
template<int K, CovarianceOption C>
auto atanh(Estimate<K, C>&& est) -> auto { return std::move(est).Atanh(); }

template<int K, CovarianceOption C>
auto logistic(const Estimate<K, C>& est) -> auto { return est.Logistic(); }
template<int K, CovarianceOption C>
auto logistic(Estimate<K, C>&& est) -> auto { return std::move(est).Logistic(); }

template<int K, CovarianceOption C>
auto erf(const Estimate<K, C>& est) -> auto { return est.Erf(); }
template<int K, CovarianceOption C>
auto erf(Estimate<K, C>&& est) -> auto { return std::move(est).Erf(); }

template<int K, CovarianceOption C>
auto erfc(const Estimate<K, C>& est) -> auto { return est.Erfc(); }
template<int K, CovarianceOption C>
auto erfc(Estimate<K, C>&& est) -> auto { return std::move(est).Erfc(); }

template<int K, CovarianceOption C>
auto lgamma(const Estimate<K, C>& est) -> auto { return est.Lgamma(); }
template<int K, CovarianceOption C>
auto lgamma(Estimate<K, C>&& est) -> auto { return std::move(est).Lgamma(); }

template<int K, CovarianceOption C>
auto ndtri(const Estimate<K, C>& est) -> auto { return est.Ndtri(); }
template<int K, CovarianceOption C>
auto ndtri(Estimate<K, C>&& est) -> auto { return std::move(est).Ndtri(); }

/// @}
/// @name Dot product
/// @{

/// @brief Inner product of two vector estimates.
/// @note The operands are assumed independent.
template<int K, CovarianceOption C, int L, CovarianceOption D>
    requires(K != 1)
auto Dot(const Estimate<K, C>& lhs, const Estimate<L, D>& rhs) -> auto { return lhs.Dot(rhs); }

template<int K, CovarianceOption C, typename AVec>
    requires(K != 1)
auto Dot(const Estimate<K, C>& lhs, const Eigen::MatrixBase<AVec>& rhs) -> auto { return lhs.Dot(rhs); }

/// @brief Inner product of a plain vector and an estimate.
template<typename AVec, int K, CovarianceOption C>
    requires(K != 1)
auto Dot(const Eigen::MatrixBase<AVec>& lhs, const Estimate<K, C>& rhs) -> auto { return rhs.Dot(lhs); }

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
