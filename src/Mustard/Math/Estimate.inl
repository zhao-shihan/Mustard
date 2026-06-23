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

namespace impl {

// =========================================================================
// EstimateBase
// =========================================================================

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
EstimateBase<ADerived, K, C>::EstimateBase(std::monostate) :
    fX{},
    fCov{} {}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
EstimateBase<ADerived, K, C>::EstimateBase() // clang-format off
    requires(K != Eigen::Dynamic) : // clang-format on
    EstimateBase{std::monostate{}} {
    fX.setZero();
    fCov.setZero();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
EstimateBase<ADerived, K, C>::EstimateBase(const EstimateBase& other) :
    EstimateBase{std::monostate{}} {
    CopyFrom(other);
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
EstimateBase<ADerived, K, C>::EstimateBase(int dim) // clang-format off
    requires(K == Eigen::Dynamic) : // clang-format on
    EstimateBase{std::monostate{}} {
    if (dim <= 0) {
        Throw<std::invalid_argument>(fmt::format("Dimension must be positive, got {}.", dim));
    }
    fX.resize(dim);
    fX.setZero();
    if constexpr (C == CovarianceOption::Full) {
        fCov.resize(dim, dim);
    } else {
        fCov.resize(dim);
    }
    fCov.setZero();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<typename AVec, typename ACov>
    requires(std::assignable_from<typename EstimateBase<ADerived, K, C>::ValueType&, const AVec&> and
             std::assignable_from<typename EstimateBase<ADerived, K, C>::CovarianceType&, const ACov&>)
EstimateBase<ADerived, K, C>::EstimateBase(const Eigen::MatrixBase<AVec>& x, const Eigen::EigenBase<ACov>& cov) :
    EstimateBase{std::monostate{}} {
    if constexpr (K == Eigen::Dynamic) {
        if (x.size() != cov.rows() or cov.rows() != cov.cols()) {
            Throw<std::invalid_argument>(fmt::format("Dimension mismatch: dim(x)={}, dim(cov)={}*{}.",
                                                     x.size(), cov.rows(), cov.cols()));
        }
    }
    fX = x.derived();
    fCov = cov.derived();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<int L, CovarianceOption D>
    requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
EstimateBase<ADerived, K, C>::EstimateBase(const Estimate<L, D>& other) :
    EstimateBase{std::monostate{}} {
    CopyFrom(other);
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
EstimateBase<ADerived, K, C>::EstimateBase(const PODType& data) // clang-format off
    requires(K != Eigen::Dynamic) : // clang-format on
    EstimateBase{std::monostate{}} {
    FromPOD(data);
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::Dimension() const -> int {
    if constexpr (K != Eigen::Dynamic) {
        return K;
    } else {
        return fX.size();
    }
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::Correlation(int i, int j) const -> double {
    if (i == j) {
        return 1;
    }
    if constexpr (C == CovarianceOption::Diagonal) {
        return 0;
    }
    return Covariance(i, j) / std::sqrt(Variance(i) * Variance(j));
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::Correlation() const -> CovarianceType {
    if constexpr (C == CovarianceOption::Full) {
        const auto sigma{VarXpr().cwiseSqrt().cwiseInverse().eval()};
        auto corr{(sigma.asDiagonal() * fCov * sigma.asDiagonal()).eval()};
        corr.diagonal().setOnes();
        return corr;
    } else {
        CovarianceType corr(Dimension());
        corr.diagonal().setOnes();
        return corr;
    }
}

// -----------------------------------------------------------------------------
// Statistical operations
// -----------------------------------------------------------------------------

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<int L, CovarianceOption D>
    requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
auto EstimateBase<ADerived, K, C>::CombineInPlace(const Estimate<L, D>& other) & -> ADerived& {
    CheckVectorDimensionMatch(other.fX);
    if constexpr (C == CovarianceOption::Full and D == CovarianceOption::Full) {
        // Both Full: full matrix inversion
        auto invSelf{fCov.inverse().eval()};
        auto invOther{other.fCov.inverse().eval()};
        auto newCov{(invSelf + invOther).inverse().eval()};
        fX = newCov * (invSelf * fX + invOther * other.fX);
        fCov = std::move(newCov);
    } else if constexpr (C == CovarianceOption::Full and D == CovarianceOption::Diagonal) {
        // Self Full, Other Diagonal
        auto invSelf{fCov.inverse().eval()};
        auto invVarOther{other.VarXpr().cwiseInverse().eval()};
        auto invSum{invSelf.eval()};
        invSum.diagonal() += invVarOther;
        auto newCov{invSum.inverse().eval()};
        fX = newCov * (invSelf * fX + invVarOther.cwiseProduct(other.fX));
        fCov = std::move(newCov);
    } else if constexpr (C == CovarianceOption::Diagonal and D == CovarianceOption::Full) {
        // Self Diagonal, Other Full: use only variances from both
        auto invVarSelf{VarXpr().cwiseInverse().eval()};
        auto invVarOther{other.VarXpr().cwiseInverse().eval()};
        auto newVar{(invVarSelf + invVarOther).cwiseInverse().eval()};
        fX = newVar.cwiseProduct(invVarSelf.cwiseProduct(fX) + invVarOther.cwiseProduct(other.fX));
        VarXpr() = newVar;
    } else {
        // Both Diagonal: element-wise inverse-variance weighting
        auto invVarSelf{VarXpr().cwiseInverse().eval()};
        auto invVarOther{other.VarXpr().cwiseInverse().eval()};
        auto newVar{(invVarSelf + invVarOther).cwiseInverse().eval()};
        fX = newVar.cwiseProduct(invVarSelf.cwiseProduct(fX) + invVarOther.cwiseProduct(other.fX));
        VarXpr() = newVar;
    }
    return Self();
}

// -----------------------------------------------------------------------------
// operator+=
// -----------------------------------------------------------------------------

#define MUSTARD_MATH_ESTIMATE_PLUS_EQUAL_UPDATE_COV()       \
    if constexpr (C == D and D == CovarianceOption::Full) { \
        fCov += other.fCov;                                 \
    } else {                                                \
        VarXpr() += other.VarXpr();                         \
    }

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<int L, CovarianceOption D>
    requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
auto EstimateBase<ADerived, K, C>::operator+=(const Estimate<L, D>& other) -> ADerived& {
    CheckVectorDimensionMatch(other.fX);
    fX += other.fX;
    MUSTARD_MATH_ESTIMATE_PLUS_EQUAL_UPDATE_COV()
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<typename AVec>
    requires(K != 1)
auto EstimateBase<ADerived, K, C>::operator+=(const Eigen::MatrixBase<AVec>& yXpr) -> ADerived& {
    CheckVectorDimensionMatch(yXpr);
    fX += yXpr;
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::operator+=(double c) -> ADerived& {
    ArrX() += c;
    return Self();
}

// -----------------------------------------------------------------------------
// operator-=
// -----------------------------------------------------------------------------

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<int L, CovarianceOption D>
    requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
auto EstimateBase<ADerived, K, C>::operator-=(const Estimate<L, D>& other) -> ADerived& {
    CheckVectorDimensionMatch(other.fX);
    fX -= other.fX;
    MUSTARD_MATH_ESTIMATE_PLUS_EQUAL_UPDATE_COV() // Cov(x-y) = Cov(x) + Cov(y) for independent x and y
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<typename AVec>
    requires(K != 1)
auto EstimateBase<ADerived, K, C>::operator-=(const Eigen::MatrixBase<AVec>& yXpr) -> ADerived& {
    CheckVectorDimensionMatch(yXpr);
    fX -= yXpr;
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::operator-=(double c) -> ADerived& {
    ArrX() -= c;
    return Self();
}

// -----------------------------------------------------------------------------
// operator*=
// -----------------------------------------------------------------------------

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<int L, CovarianceOption D>
    requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
auto EstimateBase<ADerived, K, C>::operator*=(const Estimate<L, D>& other) -> ADerived& {
    CheckVectorDimensionMatch(other.fX);
    // Cov = diag(y)·Cov(x)·diag(y) + diag(x)·Cov(y)·diag(x)
    CwiseBinaryUpdateCov(other, other.fX, fX);
    ArrX() *= other.ArrX();
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<typename AVec>
    requires(K != 1)
auto EstimateBase<ADerived, K, C>::operator*=(const Eigen::MatrixBase<AVec>& yXpr) -> ADerived& {
    CheckVectorDimensionMatch(yXpr);
    // Cov = diag(y)·Cov·diag(y)
    const auto& y{yXpr.eval()};
    ArrX() *= y.array();
    CwiseUnaryUpdateCov(y);
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::operator*=(double c) -> ADerived& {
    fX *= c;
    if constexpr (C == CovarianceOption::Full) {
        fCov *= muc::pow(c, 2);
    } else {
        VarXpr() *= muc::pow(c, 2);
    }
    return Self();
}

// -----------------------------------------------------------------------------
// operator/=
// -----------------------------------------------------------------------------

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<int L, CovarianceOption D>
    requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
auto EstimateBase<ADerived, K, C>::operator/=(const Estimate<L, D>& other) -> ADerived& {
    CheckVectorDimensionMatch(other.fX);
    // Cov = diag(1/y)·Cov(x)·diag(1/y) + diag(-x/y^2)·Cov(y)·diag(-x/y^2)
    // We drop the minus sign since it gets squared anyway.
    const auto invY{other.fX.cwiseInverse().eval()};
    ArrX() *= invY.array();
    CwiseBinaryUpdateCov(other, invY, fX.cwiseProduct(invY));
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<int L, CovarianceOption D>
    requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
auto EstimateBase<ADerived, K, C>::operator/=(Estimate<L, D>&& other) -> ADerived& {
    CheckVectorDimensionMatch(other.fX);
    if (&other == this) {
        // use the lvalue version to avoid "aliasing" issue
        return Self() /= other;
    }
    // Cov = diag(1/y)·Cov(x)·diag(1/y) + diag(-x/y^2)·Cov(y)·diag(-x/y^2)
    // We drop the minus sign since it gets squared anyway.
    other.fX = other.fX.cwiseInverse();
    ArrX() *= other.fX.array();
    CwiseBinaryUpdateCov(other, other.fX, fX.cwiseProduct(other.fX));
    return Self();
}

// -----------------------------------------------------------------------------
// Negate
// -----------------------------------------------------------------------------

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::NegateInPlace() & -> ADerived& {
    fX = -fX;
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<int L, CovarianceOption D>
    requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
auto EstimateBase<ADerived, K, C>::NegateAddInPlace(const Estimate<L, D>& other) & -> ADerived& {
    CheckVectorDimensionMatch(other.fX);
    fX = other.fX - fX;
    MUSTARD_MATH_ESTIMATE_PLUS_EQUAL_UPDATE_COV() // Cov(-x+y) = Cov(x) + Cov(y) for independent x and y
    return Self();
}

#undef MUSTARD_MATH_ESTIMATE_PLUS_EQUAL_UPDATE_COV

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<typename AVec>
    requires(K != 1)
auto EstimateBase<ADerived, K, C>::NegateAddInPlace(const Eigen::MatrixBase<AVec>& yXpr) & -> ADerived& {
    CheckVectorDimensionMatch(yXpr);
    fX = yXpr - fX;
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::NegateAddInPlace(double c) & -> ADerived& {
    ArrX() = c - ArrX();
    return Self();
}

// -----------------------------------------------------------------------------
// Reciprocal
// -----------------------------------------------------------------------------

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<int L, CovarianceOption D>
    requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
auto EstimateBase<ADerived, K, C>::DivideInPlace(const Estimate<L, D>& other) & -> ADerived& {
    CheckVectorDimensionMatch(other.fX);
    // Cov = diag(-y/x^2)·Cov(x)·diag(-y/x^2) + diag(1/x)·Cov(y)·diag(1/x)
    // We drop the minus sign since it gets squared anyway.
    const auto invX{fX.cwiseInverse().eval()};
    fX = other.fX.cwiseProduct(invX);
    CwiseBinaryUpdateCov(other, fX.cwiseProduct(invX), invX);
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<int L, CovarianceOption D>
    requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
auto EstimateBase<ADerived, K, C>::DivideInPlace(Estimate<L, D>&& other) & -> ADerived& {
    CheckVectorDimensionMatch(other.fX);
    if (&other == this) {
        // use the lvalue version to avoid "aliasing" issue
        Self() = Divide(other);
        return Self();
    }
    // Cov = diag(-y/x^2)·Cov(x)·diag(-y/x^2) + diag(1/x)·Cov(y)·diag(1/x)
    // We drop the minus sign since it gets squared anyway.
    fX = fX.cwiseInverse();
    other.ArrX() *= ArrX();
    CwiseBinaryUpdateCov(other, other.ArrX().cwiseProduct(fX), fX);
    fX = other.ArrX();
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<typename AVec>
    requires(K != 1)
auto EstimateBase<ADerived, K, C>::DivideInPlace(const Eigen::MatrixBase<AVec>& yXpr) & -> ADerived& {
    CheckVectorDimensionMatch(yXpr);
    // Cov = diag(-y/x²)·Cov·diag(-y/x²)
    // We drop the minus sign since it gets squared anyway.
    const auto invX{fX.cwiseInverse().eval()};
    fX = yXpr.cwiseProduct(invX);
    CwiseUnaryUpdateCov(fX.cwiseProduct(invX));
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::DivideInPlace(double c) & -> ADerived& {
    // Cov = diag(-c/x²)·Cov·diag(-c/x²)
    // We drop the minus sign since it gets squared anyway.
    fX = fX.cwiseInverse();
    CwiseUnaryUpdateCov(c * fX.cwiseSquare());
    fX *= c;
    return Self();
}

// -----------------------------------------------------------------------------
// Unary math functions: Algebraic / Power
// -----------------------------------------------------------------------------

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::SquareInPlace() & -> ADerived& {
    // Cov = diag(2·x_i)·Cov·diag(2·x_i)
    CwiseUnaryUpdateCov(2 * fX);
    fX = fX.cwiseSquare();
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::CubeInPlace() & -> ADerived& {
    // Cov = diag(3·x_i²)·Cov·diag(3·x_i²)
    const auto xSquare{fX.cwiseSquare().eval()};
    CwiseUnaryUpdateCov(3 * xSquare);
    ArrX() *= xSquare.array();
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::SqrtInPlace() & -> ADerived& {
    // Cov = diag(1/(2·sqrt(x_i)))·Cov·diag(1/(2·sqrt(x_i)))
    fX = fX.cwiseSqrt();
    CwiseUnaryUpdateCov(0.5 * fX.cwiseInverse());
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::CbrtInPlace() & -> ADerived& {
    // Cov = diag(1/(3·cbrt(x_i)²))·Cov·diag(1/(3·cbrt(x_i)²))
    fX = fX.cwiseCbrt();
    CwiseUnaryUpdateCov((1. / 3.) * fX.cwiseSquare().cwiseInverse());
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::RsqrtInPlace() & -> ADerived& {
    // Cov = diag(-rsqrt(x_i)³/2)·Cov·diag(-rsqrt(x_i)³/2)
    // We drop the minus sign since it gets squared anyway.
    ArrX() = rsqrt(ArrX());
    CwiseUnaryUpdateCov(0.5 * ArrX().cube());
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::InverseInPlace() & -> ADerived& {
    // Cov = diag(-1/x_i²)·Cov·diag(-1/x_i²)
    // We drop the minus sign since it gets squared anyway.
    fX = fX.cwiseInverse();
    CwiseUnaryUpdateCov(fX.cwiseSquare());
    return Self();
}

// -----------------------------------------------------------------------------
// Unary math functions: Piecewise
// -----------------------------------------------------------------------------

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::AbsInPlace() & -> ADerived& {
    // Cov = diag(sign(x_i))·Cov·diag(sign(x_i))
    CwiseUnaryUpdateCov(sign(ArrX()));
    fX = fX.cwiseAbs();
    return Self();
}

// -----------------------------------------------------------------------------
// Unary math functions: Exponential
// -----------------------------------------------------------------------------

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::ExpInPlace() & -> ADerived& {
    // Cov = diag(exp(x_i))·Cov·diag(exp(x_i))
    ArrX() = exp(ArrX());
    CwiseUnaryUpdateCov(fX);
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::Exp2InPlace() & -> ADerived& {
    // Cov = diag(ln2·2^{x_i})·Cov·diag(ln2·2^{x_i})
    ArrX() = exp2(ArrX());
    CwiseUnaryUpdateCov(std::numbers::ln2 * fX);
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::Expm1InPlace() & -> ADerived& {
    // Cov = diag(exp(x_i))·Cov·diag(exp(x_i))
    ArrX() = expm1(ArrX());
    CwiseUnaryUpdateCov(1 + ArrX());
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::LogisticInPlace() & -> ADerived& {
    // Cov = diag(s(x_i)·(1-s(x_i)))·Cov·diag(s(x_i)·(1-s(x_i)))
    ArrX() = logistic(ArrX());
    CwiseUnaryUpdateCov(ArrX() * (1 - ArrX()));
    return Self();
}

// -----------------------------------------------------------------------------
// Unary math functions: Logarithmic
// -----------------------------------------------------------------------------

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::LogInPlace() & -> ADerived& {
    // Cov = diag(1/x_i)·Cov·diag(1/x_i)
    CwiseUnaryUpdateCov(fX.cwiseInverse());
    ArrX() = log(ArrX());
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::Log10InPlace() & -> ADerived& {
    // Cov = diag(1/(x_i·ln10))·Cov·diag(1/(x_i·ln10))
    constexpr auto invLog10{1 / std::numbers::ln10};
    CwiseUnaryUpdateCov(invLog10 / ArrX());
    ArrX() = log10(ArrX());
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::Log2InPlace() & -> ADerived& {
    // Cov = diag(1/(x_i·ln2))·Cov·diag(1/(x_i·ln2))
    constexpr auto invLog2{1 / std::numbers::ln2};
    CwiseUnaryUpdateCov(invLog2 / ArrX());
    ArrX() = log2(ArrX());
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::Log1pInPlace() & -> ADerived& {
    // Cov = diag(1/(1+x_i))·Cov·diag(1/(1+x_i))
    CwiseUnaryUpdateCov(inverse(1 + ArrX()));
    ArrX() = log1p(ArrX());
    return Self();
}

// -----------------------------------------------------------------------------
// Unary math functions: Trigonometric
// -----------------------------------------------------------------------------

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::SinInPlace() & -> ADerived& {
    // Cov = diag(cos(x_i))·Cov·diag(cos(x_i))
    CwiseUnaryUpdateCov(cos(ArrX()));
    ArrX() = sin(ArrX());
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::CosInPlace() & -> ADerived& {
    // Cov = diag(-sin(x_i))·Cov·diag(-sin(x_i))
    // We drop the minus sign since it gets squared anyway.
    CwiseUnaryUpdateCov(sin(ArrX()));
    ArrX() = cos(ArrX());
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::TanInPlace() & -> ADerived& {
    // Cov = diag(sec²(x_i))·Cov·diag(sec²(x_i))
    ArrX() = tan(ArrX());
    CwiseUnaryUpdateCov((1 + ArrX().square()));
    return Self();
}

// -----------------------------------------------------------------------------
// Unary math functions: Inverse Trigonometric
// -----------------------------------------------------------------------------

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::AsinInPlace() & -> ADerived& {
    // Cov = diag(1/sqrt(1-x_i²))·Cov·diag(1/sqrt(1-x_i²))
    CwiseUnaryUpdateCov(inverse(sqrt(1 - ArrX().square())));
    ArrX() = asin(ArrX());
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::AcosInPlace() & -> ADerived& {
    // Cov = diag(-1/sqrt(1-x_i²))·Cov·diag(-1/sqrt(1-x_i²))
    // We drop the minus sign since it gets squared anyway.
    CwiseUnaryUpdateCov(inverse(sqrt(1 - ArrX().square())));
    ArrX() = acos(ArrX());
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::AtanInPlace() & -> ADerived& {
    // Cov = diag(1/(1+x_i²))·Cov·diag(1/(1+x_i²))
    CwiseUnaryUpdateCov(inverse(1 + ArrX().square()));
    ArrX() = atan(ArrX());
    return Self();
}

// -----------------------------------------------------------------------------
// Unary math functions: Hyperbolic
// -----------------------------------------------------------------------------

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::SinhInPlace() & -> ADerived& {
    // Cov = diag(cosh(x_i))·Cov·diag(cosh(x_i))
    ArrX() = sinh(ArrX());
    CwiseUnaryUpdateCov(sqrt(1 + ArrX().square()));
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::CoshInPlace() & -> ADerived& {
    // Cov = diag(sinh(x_i))·Cov·diag(sinh(x_i))
    const auto signX{sign(ArrX())};
    ArrX() = cosh(ArrX());
    CwiseUnaryUpdateCov(signX * sqrt(ArrX().square() - 1));
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::TanhInPlace() & -> ADerived& {
    // Cov = diag(sech²(x_i))·Cov·diag(sech²(x_i))
    ArrX() = tanh(ArrX());
    CwiseUnaryUpdateCov(1 - ArrX().square());
    return Self();
}

// -----------------------------------------------------------------------------
// Unary math functions: Inverse Hyperbolic
// -----------------------------------------------------------------------------

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::AsinhInPlace() & -> ADerived& {
    // Cov = diag(1/sqrt(1+x_i²))·Cov·diag(1/sqrt(1+x_i²))
    CwiseUnaryUpdateCov(inverse(sqrt(1 + ArrX().square())));
    ArrX() = asinh(ArrX());
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::AcoshInPlace() & -> ADerived& {
    // Cov = diag(1/sqrt(x_i²-1))·Cov·diag(1/sqrt(x_i²-1))
    CwiseUnaryUpdateCov(inverse(sqrt(ArrX().square() - 1)));
    ArrX() = acosh(ArrX());
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::AtanhInPlace() & -> ADerived& {
    // Cov = diag(1/(1-x_i²))·Cov·diag(1/(1-x_i²))
    CwiseUnaryUpdateCov(inverse(1 - ArrX().square()));
    ArrX() = atanh(ArrX());
    return Self();
}

// -----------------------------------------------------------------------------
// Unary math functions: Special / Transcendental
// -----------------------------------------------------------------------------

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::ErfInPlace() & -> ADerived& {
    // Cov = diag(2/√π·exp(-x_i²))·Cov·diag(2/√π·exp(-x_i²))
    constexpr auto twoInvSqrtPi{2 * std::numbers::inv_sqrtpi};
    CwiseUnaryUpdateCov(twoInvSqrtPi * exp(-ArrX().square()));
    ArrX() = erf(ArrX());
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::ErfcInPlace() & -> ADerived& {
    // Cov = diag(-2/√π·exp(-x_i²))·Cov·diag(-2/√π·exp(-x_i²))
    constexpr auto twoInvSqrtPi{2 * std::numbers::inv_sqrtpi};
    CwiseUnaryUpdateCov(-twoInvSqrtPi * exp(-ArrX().square()));
    ArrX() = erfc(ArrX());
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::LgammaInPlace() & -> ADerived& {
    // Cov = diag(digamma(x_i))·Cov·diag(digamma(x_i))
    CwiseUnaryUpdateCov(digamma(ArrX()));
    ArrX() = lgamma(ArrX());
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::NdtriInPlace() & -> ADerived& {
    // Cov = diag(√(2π)·exp(Φ⁻¹(x_i)²/2))·Cov·diag(√(2π)·exp(Φ⁻¹(x_i)²/2))
    ArrX() = ndtri(ArrX());
    constexpr auto sqrt2pi{std::numbers::sqrt2 / std::numbers::inv_sqrtpi};
    CwiseUnaryUpdateCov(sqrt2pi * exp(ArrX().square() / 2));
    return Self();
}

// -----------------------------------------------------------------------------
// Binary math functions
// -----------------------------------------------------------------------------

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<int L, CovarianceOption D>
    requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
auto EstimateBase<ADerived, K, C>::PowInPlace(const Estimate<L, D>& other) & -> ADerived& {
    CheckVectorDimensionMatch(other.fX);
    // x^y = exp(y·log(x))
    // Cov = diag(y/x·x^y)·Cov(x)·diag(y/x·x^y) + diag(x^y·log(x))·Cov(y)·diag(x^y·log(x))
    const auto logX{log(ArrX()).eval()};
    const auto xPowY{exp(other.ArrX() * logX).eval()};
    CwiseBinaryUpdateCov(other, other.ArrX() / ArrX() * xPowY, xPowY * logX);
    ArrX() = xPowY;
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<typename AVec>
    requires(K != 1)
auto EstimateBase<ADerived, K, C>::PowInPlace(const Eigen::MatrixBase<AVec>& yXpr) & -> ADerived& {
    CheckVectorDimensionMatch(yXpr);
    // x^y = exp(y·log(x))
    // Cov = diag(y/x·x^y)·Cov·diag(y/x·x^y)
    const auto& y{yXpr.eval()};
    const auto logX{log(ArrX()).eval()};
    const auto xPowY{exp(y.array() * logX).eval()};
    CwiseUnaryUpdateCov(y.array() / ArrX() * xPowY);
    ArrX() = xPowY;
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::PowInPlace(double c) & -> ADerived& {
    // Cov = diag(c/x·x^c)·Cov·diag(c/x·x^c)
    const auto xPowY{ArrX().pow(c).eval()};
    CwiseUnaryUpdateCov(c / ArrX() * xPowY);
    ArrX() = xPowY;
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<int L, CovarianceOption D>
    requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
auto EstimateBase<ADerived, K, C>::ExpInPlace(const Estimate<L, D>& other) & -> ADerived& {
    CheckVectorDimensionMatch(other.fX);
    // y^x = exp(x·log(y))
    // Cov = diag(y^x·log(y))·Cov(x)·diag(y^x·log(y)) + diag(x/y·y^x)·Cov(y)·diag(x/y·y^x)
    const auto logY{log(other.ArrX()).eval()};
    const auto yPowX{exp(ArrX() * logY).eval()};
    CwiseBinaryUpdateCov(other, yPowX * logY, ArrX() / other.ArrX() * yPowX);
    ArrX() = yPowX;
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<typename AVec>
    requires(K != 1)
auto EstimateBase<ADerived, K, C>::ExpInPlace(const Eigen::MatrixBase<AVec>& yXpr) & -> ADerived& {
    CheckVectorDimensionMatch(yXpr);
    // y^x = exp(x·log(y))
    // Cov = diag(y^x·log(y))·Cov·diag(y^x·log(y))
    const auto logY{log(yXpr.array()).eval()};
    ArrX() = exp(ArrX() * logY);
    CwiseUnaryUpdateCov(ArrX() * logY);
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::ExpInPlace(double c) & -> ADerived& {
    // c^x = exp(x·log(c))
    // Cov = diag(c^x·log(c))·Cov·diag(c^x·log(c))
    const auto logC{std::log(c)};
    ArrX() = exp(ArrX() * logC);
    CwiseUnaryUpdateCov(ArrX() * logC);
    return Self();
}

// -----------------------------------------------------------------------------
// Dot product (vector-vector)
// -----------------------------------------------------------------------------

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<int L, CovarianceOption D>
    requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
auto EstimateBase<ADerived, K, C>::Dot(const Estimate<L, D>& other) const -> Estimate<1, C> {
    CheckVectorDimensionMatch(other.fX);
    const auto x{fX.dot(other.fX)};
    auto var{(other.fX.transpose() * this->fCov * other.fX).coeff(0, 0) +
             (this->fX.transpose() * other.fCov * this->fX).coeff(0, 0)};
    return {x, var};
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<typename AVec>
    requires(AVec::ColsAtCompileTime == 1)
auto EstimateBase<ADerived, K, C>::Dot(const Eigen::MatrixBase<AVec>& yXpr) const -> Estimate<1, C> {
    CheckVectorDimensionMatch(yXpr);
    const auto& y{yXpr.eval()};
    const auto x{fX.dot(y)};
    const auto var{(y.transpose() * fCov * y).coeff(0, 0)};
    return {x, var};
}

// -----------------------------------------------------------------------------
// Matrix-vector product
// -----------------------------------------------------------------------------

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<typename AMat>
    requires(AMat::ColsAtCompileTime == K or
             K == Eigen::Dynamic or AMat::RowsAtCompileTime == Eigen::Dynamic)
auto EstimateBase<ADerived, K, C>::LeftMultiply(const Eigen::MatrixBase<AMat>& aXpr) const -> Estimate<AMat::RowsAtCompileTime, C> {
    CheckMatrixColDimensionMatch(aXpr);
    const auto& a{aXpr.eval()};
    const auto x{a * fX};
    if constexpr (C == CovarianceOption::Full) {
        const auto cov{a * fCov * a.transpose()};
        return {x, cov};
    } else {
        const auto var{a.cwiseSquare() * VarXpr()};
        return {x, var.asDiagonal()};
    }
}

// -----------------------------------------------------------------------------
// Reduction operations
// -----------------------------------------------------------------------------

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::Sum() const -> Estimate<1, C> {
    const auto value{fX.sum()};
    const auto var{[this] {
        if constexpr (C == CovarianceOption::Full) {
            return fCov.sum();
        } else {
            return VarXpr().sum();
        }
    }()};
    return {value, var};
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<double P>
    requires(P >= 1)
auto EstimateBase<ADerived, K, C>::LpNorm() && -> Estimate<1, C> {
    if constexpr (muc::isclose(P, 1.)) {
        return AbsInPlace().Sum();
    } else if constexpr (muc::isclose(P, 2.)) {
        return SquareInPlace().Sum().Sqrt();
    } else if constexpr (muc::isclose(P, 3.)) {
        return AbsInPlace().CubeInPlace().Sum().Cbrt();
    } else if constexpr (muc::isclose(P, 4.)) {
        return SquareInPlace().SquareInPlace().Sum().Sqrt().Sqrt();
    } else {
        return AbsInPlace().PowInPlace(P).Sum().Pow(1. / P);
    }
}

// -----------------------------------------------------------------------------
// Normalization
// -----------------------------------------------------------------------------

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::Normalize() & -> ADerived& {
    const auto sqNorm{fX.squaredNorm()};
    if (muc::isclose(sqNorm, 0.)) {
        return Self();
    }
    const auto invSqNorm{1 / sqNorm};
    fX *= std::sqrt(invSqNorm);
    // J = (I - x x^T) / |x_old|,
    // Cov = J Cov J^T = (I - x x^T) Cov (I - x x^T) / |x_old|^2
    if constexpr (C == CovarianceOption::Full) {
        // (I - x x^T) Cov (I - x x^T) = Cov - (Cov x x^T + x (Cov x - (x^T Cov x) x)^T)
        //                             = Cov - (y x^T + x (y - (x^T y) x)^T)
        //                             = Cov - (y x^T + x z^T)
        const auto y{(fCov * fX).eval()};
        const auto z{(y - fX.dot(y) * fX).eval()};
        fCov.noalias() -= y * fX.transpose() + fX * z.transpose();
        fCov *= invSqNorm;
    } else {
        // For diagonal covariance, only diagonal elements are needed:
        // Cov_{ii} = (1/|x_old|^2) * Σ_j (δ_{ij} - x_i x_j)^2 σ_j^2
        // with (δ_{ij} - x_i x_j)^2 = δ_{ij}(1 - 2x_i^2) + x_i^2 x_j^2:
        // Cov_{ii} = (1/|x_old|^2) * (σ_i^2 (1 - 2x_i^2) + x_i^2 Σ_j x_j^2 σ_j^2)
        const auto xSq{fX.cwiseSquare().array().eval()};
        const auto s{VarXpr().dot(xSq.matrix())};
        VarXpr() = invSqNorm * (VarXpr().array() * (1 - 2 * xSq) + s * xSq);
    }
    return Self();
}

// -----------------------------------------------------------------------------
// Other member functions
// -----------------------------------------------------------------------------

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::ToPOD() const -> PODType
    requires(K != Eigen::Dynamic) {
    PODType data;
    std::memcpy(data.x, fX.data(), K * sizeof(double));
    if constexpr (C == CovarianceOption::Full) {
        std::memcpy(data.cov, fCov.data(), K * K * sizeof(double));
    } else {
        std::memcpy(data.cov, VarXpr().data(), K * sizeof(double));
    }
    return data;
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto EstimateBase<ADerived, K, C>::FromPOD(const PODType& data) -> void
    requires(K != Eigen::Dynamic) {
    std::memcpy(fX.data(), data.x, K * sizeof(double));
    if constexpr (C == CovarianceOption::Full) {
        std::memcpy(fCov.data(), data.cov, K * K * sizeof(double));
    } else {
        std::memcpy(VarXpr().data(), data.cov, K * sizeof(double));
    }
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<typename AVec>
    requires((AVec::RowsAtCompileTime == K or
              K == Eigen::Dynamic or AVec::RowsAtCompileTime == Eigen::Dynamic) and
             AVec::ColsAtCompileTime == 1)
auto EstimateBase<ADerived, K, C>::CheckVectorDimensionMatch(const Eigen::MatrixBase<AVec>& yXpr) const -> void {
    if constexpr (K == Eigen::Dynamic or AVec::RowsAtCompileTime == Eigen::Dynamic) {
        if (Dimension() != yXpr.size()) {
            Throw<std::invalid_argument>(fmt::format("Dimension mismatch: dim(self)={}, dim(other)={}",
                                                     Dimension(), yXpr.size()));
        }
    }
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<typename AMat>
    requires(AMat::RowsAtCompileTime == K or
             K == Eigen::Dynamic or AMat::RowsAtCompileTime == Eigen::Dynamic)
auto EstimateBase<ADerived, K, C>::CheckMatrixRowDimensionMatch(const Eigen::MatrixBase<AMat>& aXpr) const -> void {
    if constexpr (K == Eigen::Dynamic or AMat::RowsAtCompileTime == Eigen::Dynamic) {
        if (Dimension() != aXpr.rows()) {
            Throw<std::invalid_argument>(fmt::format("Row dimension mismatch: dim(self)={}, matrix rows={}",
                                                     Dimension(), aXpr.rows()));
        }
    }
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<typename AMat>
    requires(AMat::ColsAtCompileTime == K or
             K == Eigen::Dynamic or AMat::ColsAtCompileTime == Eigen::Dynamic)
auto EstimateBase<ADerived, K, C>::CheckMatrixColDimensionMatch(const Eigen::MatrixBase<AMat>& aXpr) const -> void {
    if constexpr (K == Eigen::Dynamic or AMat::ColsAtCompileTime == Eigen::Dynamic) {
        if (Dimension() != aXpr.cols()) {
            Throw<std::invalid_argument>(fmt::format("Col dimension mismatch: dim(self)={}, matrix cols={}",
                                                     Dimension(), aXpr.cols()));
        }
    }
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<typename AOther, int L, CovarianceOption D>
    requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
auto EstimateBase<ADerived, K, C>::CopyFrom(const EstimateBase<AOther, L, D>& other) & -> ADerived& {
    if constexpr (K != Eigen::Dynamic) { // dynamic dimension matrices will be resized so do not check
        CheckVectorDimensionMatch(other.fX);
    }
    fX = other.fX;
    if constexpr (C == D) {
        fCov = other.fCov;
    } else {
        if constexpr (K == Eigen::Dynamic) {
            if constexpr (C == CovarianceOption::Full) {
                fCov.resize(other.Dimension(), other.Dimension());
            } else {
                fCov.resize(other.Dimension());
            }
        }
        if constexpr (C == CovarianceOption::Full) {
            fCov.setZero();
        }
        VarXpr() = other.VarXpr();
    }
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<typename AJac>
    requires(AJac::ColsAtCompileTime == 1)
auto EstimateBase<ADerived, K, C>::CwiseUnaryUpdateCov(const Eigen::DenseBase<AJac>& diagJacXpr) -> void {
    const auto& diagJac{[&diagJacXpr]() -> decltype(auto) {
        if constexpr (std::derived_from<AJac, Eigen::MatrixBase<AJac>>) {
            return diagJacXpr.derived();
        } else {
            return diagJacXpr.derived().matrix();
        }
    }()};
    if constexpr (C == CovarianceOption::Full) {
        const auto& diagJacEval{diagJac.eval()};
        const auto& jac{diagJacEval.asDiagonal()};
        fCov = jac * fCov * jac;
    } else {
        VarXpr() = VarXpr().cwiseProduct(diagJac.cwiseSquare());
    }
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<int L, CovarianceOption D, typename AJacX, typename AJacY>
    requires(AJacX::ColsAtCompileTime == 1 and AJacY::ColsAtCompileTime == 1)
auto EstimateBase<ADerived, K, C>::CwiseBinaryUpdateCov(const Estimate<L, D>& other,
                                                        const Eigen::DenseBase<AJacX>& diagJacXXpr,
                                                        const Eigen::DenseBase<AJacY>& diagJacYXpr) -> void {
    constexpr auto toVecXpr{[]<typename AJac>(const Eigen::DenseBase<AJac>& diagJacXpr) -> decltype(auto) {
        if constexpr (std::derived_from<AJac, Eigen::MatrixBase<AJac>>) {
            return diagJacXpr.derived();
        } else {
            return diagJacXpr.derived().matrix();
        }
    }};
    const auto& diagJacX{toVecXpr(diagJacXXpr)};
    const auto& diagJacY{toVecXpr(diagJacYXpr)};
    if constexpr (C == CovarianceOption::Full) {
        const auto& diagJacXEval{diagJacX.eval()};
        const auto& jacX{diagJacXEval.asDiagonal()};
        if constexpr (D == CovarianceOption::Full) {
            const auto& diagJacYEval{diagJacY.eval()};
            const auto& jacY{diagJacYEval.asDiagonal()};
            fCov = jacX * fCov * jacX + jacY * other.fCov * jacY;
        } else {
            fCov = jacX * fCov * jacX;
            VarXpr() += other.VarXpr().cwiseProduct(diagJacY.cwiseSquare());
        }
    } else {
        VarXpr() = VarXpr().cwiseProduct(diagJacX.cwiseSquare()) +
                   other.VarXpr().cwiseProduct(diagJacY.cwiseSquare());
    }
}

} // namespace impl

// =========================================================================
// Estimate<1, C>
// =========================================================================

template<CovarianceOption C>
Estimate<1, C>::Estimate(double x, double var) :
    Base{typename Base::ValueType{{x}},
         typename Base::CovarianceType{{var}}} {}

// =========================================================================
// Estimate<K, C>
// =========================================================================

template<int K, CovarianceOption C>
template<typename AVec, typename ACov>
    requires(std::assignable_from<typename Estimate<K, C>::ValueType&, const AVec&> and
             std::assignable_from<typename Estimate<K, C>::CovarianceType&, const ACov&>)
Estimate<K, C>::Estimate(const Eigen::MatrixBase<AVec>& x, const Eigen::EigenBase<ACov>& cov) :
    Base{x, cov} {}

// =========================================================================
// Forwarding functions definitions
// =========================================================================

#define MUSTARD_MATH_ESTIMATE_TWO_STEP_FORWARDING_FUNCTION_BODY(RetType, source, Op, operand) \
    RetType result{source};                                                                   \
    result.Op(operand);                                                                       \
    return result; // NRVO should kick in to avoid unnecessary copy/move

#define MUSTARD_MATH_ESTIMATE_BINARY_OP_RET_TYPE impl::EstimateBinaryOpResult<K, C, L, D>

#define MUSTARD_MATH_ESTIMATE_ESTIMATE_BINARY_OP_DEFINITIONS(Op, FwdOpInPlace, BwdOpInPlace)                      \
    template<int K, CovarianceOption C, int L, CovarianceOption D>                                                \
    auto Op(const Estimate<K, C>& lhs, const Estimate<L, D>& rhs) -> MUSTARD_MATH_ESTIMATE_BINARY_OP_RET_TYPE {   \
        MUSTARD_MATH_ESTIMATE_TWO_STEP_FORWARDING_FUNCTION_BODY(MUSTARD_MATH_ESTIMATE_BINARY_OP_RET_TYPE,         \
                                                                lhs, FwdOpInPlace, rhs)                           \
    }                                                                                                             \
                                                                                                                  \
    template<int K, CovarianceOption C, int L, CovarianceOption D>                                                \
    auto Op(const Estimate<K, C>& lhs, Estimate<L, D>&& rhs) -> MUSTARD_MATH_ESTIMATE_BINARY_OP_RET_TYPE {        \
        if constexpr (std::same_as<MUSTARD_MATH_ESTIMATE_BINARY_OP_RET_TYPE, Estimate<L, D>>) {                   \
            MUSTARD_MATH_ESTIMATE_TWO_STEP_FORWARDING_FUNCTION_BODY(MUSTARD_MATH_ESTIMATE_BINARY_OP_RET_TYPE,     \
                                                                    std::move(rhs), BwdOpInPlace, lhs)            \
        } else {                                                                                                  \
            MUSTARD_MATH_ESTIMATE_TWO_STEP_FORWARDING_FUNCTION_BODY(MUSTARD_MATH_ESTIMATE_BINARY_OP_RET_TYPE,     \
                                                                    lhs, FwdOpInPlace, std::move(rhs))            \
        }                                                                                                         \
    }                                                                                                             \
                                                                                                                  \
    template<int K, CovarianceOption C, int L, CovarianceOption D>                                                \
    auto Op(Estimate<K, C>&& lhs, const Estimate<L, D>& rhs) -> MUSTARD_MATH_ESTIMATE_BINARY_OP_RET_TYPE {        \
        if constexpr (std::same_as<MUSTARD_MATH_ESTIMATE_BINARY_OP_RET_TYPE, Estimate<K, C>>) {                   \
            MUSTARD_MATH_ESTIMATE_TWO_STEP_FORWARDING_FUNCTION_BODY(MUSTARD_MATH_ESTIMATE_BINARY_OP_RET_TYPE,     \
                                                                    std::move(lhs), FwdOpInPlace, rhs)            \
        } else {                                                                                                  \
            MUSTARD_MATH_ESTIMATE_TWO_STEP_FORWARDING_FUNCTION_BODY(MUSTARD_MATH_ESTIMATE_BINARY_OP_RET_TYPE,     \
                                                                    rhs, BwdOpInPlace, std::move(lhs))            \
        }                                                                                                         \
    }                                                                                                             \
                                                                                                                  \
    template<int K, CovarianceOption C, int L, CovarianceOption D>                                                \
    auto Op(Estimate<K, C>&& lhs, Estimate<L, D>&& rhs) -> MUSTARD_MATH_ESTIMATE_BINARY_OP_RET_TYPE {             \
        if constexpr (std::same_as<MUSTARD_MATH_ESTIMATE_BINARY_OP_RET_TYPE, Estimate<K, C>> or                   \
                      not std::same_as<MUSTARD_MATH_ESTIMATE_BINARY_OP_RET_TYPE, Estimate<L, D>>) {               \
            MUSTARD_MATH_ESTIMATE_TWO_STEP_FORWARDING_FUNCTION_BODY(MUSTARD_MATH_ESTIMATE_BINARY_OP_RET_TYPE,     \
                                                                    std::move(lhs), FwdOpInPlace, std::move(rhs)) \
        } else {                                                                                                  \
            MUSTARD_MATH_ESTIMATE_TWO_STEP_FORWARDING_FUNCTION_BODY(MUSTARD_MATH_ESTIMATE_BINARY_OP_RET_TYPE,     \
                                                                    std::move(rhs), BwdOpInPlace, std::move(lhs)) \
        }                                                                                                         \
    }

MUSTARD_MATH_ESTIMATE_ESTIMATE_BINARY_OP_DEFINITIONS(operator+, operator+=, operator+=)
MUSTARD_MATH_ESTIMATE_ESTIMATE_BINARY_OP_DEFINITIONS(operator-, operator-=, NegateAddInPlace)
MUSTARD_MATH_ESTIMATE_ESTIMATE_BINARY_OP_DEFINITIONS(operator*, operator*=, operator*=)
MUSTARD_MATH_ESTIMATE_ESTIMATE_BINARY_OP_DEFINITIONS(operator/, operator/=, DivideInPlace)
MUSTARD_MATH_ESTIMATE_ESTIMATE_BINARY_OP_DEFINITIONS(pow, PowInPlace, ExpInPlace)
MUSTARD_MATH_ESTIMATE_ESTIMATE_BINARY_OP_DEFINITIONS(Combine, CombineInPlace, CombineInPlace)
#undef MUSTARD_MATH_ESTIMATE_ESTIMATE_BINARY_OP_DEFINITIONS

#define MUSTARD_MATH_ESTIMATE_VECTOR_BINARY_OP_DEFINITIONS(Op, FwdOpInPlace, BwdOpInPlace)     \
    template<int K, CovarianceOption C, typename AVec>                                         \
        requires(K != 1 and AVec::ColsAtCompileTime == 1)                                      \
    auto Op(const Estimate<K, C>& lhs, const Eigen::MatrixBase<AVec>& rhs) -> Estimate<K, C> { \
        MUSTARD_MATH_ESTIMATE_TWO_STEP_FORWARDING_FUNCTION_BODY(auto, lhs, FwdOpInPlace, rhs)  \
    }                                                                                          \
                                                                                               \
    template<int K, CovarianceOption C, typename AVec>                                         \
        requires(K != 1 and AVec::ColsAtCompileTime == 1)                                      \
    auto Op(Estimate<K, C>&& lhs, const Eigen::MatrixBase<AVec>& rhs) -> Estimate<K, C> {      \
        return std::move(lhs.FwdOpInPlace(rhs));                                               \
    }                                                                                          \
                                                                                               \
    template<typename AVec, int K, CovarianceOption C>                                         \
        requires(K != 1 and AVec::ColsAtCompileTime == 1)                                      \
    auto Op(const Eigen::MatrixBase<AVec>& lhs, const Estimate<K, C>& rhs) -> Estimate<K, C> { \
        MUSTARD_MATH_ESTIMATE_TWO_STEP_FORWARDING_FUNCTION_BODY(auto, rhs, BwdOpInPlace, lhs)  \
    }                                                                                          \
                                                                                               \
    template<typename AVec, int K, CovarianceOption C>                                         \
        requires(K != 1 and AVec::ColsAtCompileTime == 1)                                      \
    auto Op(const Eigen::MatrixBase<AVec>& lhs, Estimate<K, C>&& rhs) -> Estimate<K, C> {      \
        return std::move(rhs.BwdOpInPlace(lhs));                                               \
    }

MUSTARD_MATH_ESTIMATE_VECTOR_BINARY_OP_DEFINITIONS(operator+, operator+=, operator+=)
MUSTARD_MATH_ESTIMATE_VECTOR_BINARY_OP_DEFINITIONS(operator-, operator-=, NegateAddInPlace)
MUSTARD_MATH_ESTIMATE_VECTOR_BINARY_OP_DEFINITIONS(operator*, operator*=, operator*=)
MUSTARD_MATH_ESTIMATE_VECTOR_BINARY_OP_DEFINITIONS(operator/, operator/=, DivideInPlace)
MUSTARD_MATH_ESTIMATE_VECTOR_BINARY_OP_DEFINITIONS(pow, PowInPlace, ExpInPlace)
#undef MUSTARD_MATH_ESTIMATE_VECTOR_BINARY_OP_DEFINITIONS

#define MUSTARD_MATH_ESTIMATE_SCALAR_BINARY_OP_DEFINITIONS(Op, FwdOpInPlace, BwdOpInPlace)    \
    template<int K, CovarianceOption C>                                                       \
    auto Op(const Estimate<K, C>& lhs, double rhs) -> Estimate<K, C> {                        \
        MUSTARD_MATH_ESTIMATE_TWO_STEP_FORWARDING_FUNCTION_BODY(auto, lhs, FwdOpInPlace, rhs) \
    }                                                                                         \
                                                                                              \
    template<int K, CovarianceOption C>                                                       \
    auto Op(Estimate<K, C>&& lhs, double rhs) -> Estimate<K, C> {                             \
        return std::move(lhs.FwdOpInPlace(rhs));                                              \
    }                                                                                         \
                                                                                              \
    template<int K, CovarianceOption C>                                                       \
    auto Op(double lhs, const Estimate<K, C>& rhs) -> Estimate<K, C> {                        \
        MUSTARD_MATH_ESTIMATE_TWO_STEP_FORWARDING_FUNCTION_BODY(auto, rhs, BwdOpInPlace, lhs) \
    }                                                                                         \
                                                                                              \
    template<int K, CovarianceOption C>                                                       \
    auto Op(double lhs, Estimate<K, C>&& rhs) -> Estimate<K, C> {                             \
        return std::move(rhs.BwdOpInPlace(lhs));                                              \
    }

MUSTARD_MATH_ESTIMATE_SCALAR_BINARY_OP_DEFINITIONS(operator+, operator+=, operator+=)
MUSTARD_MATH_ESTIMATE_SCALAR_BINARY_OP_DEFINITIONS(operator-, operator-=, NegateAddInPlace)
MUSTARD_MATH_ESTIMATE_SCALAR_BINARY_OP_DEFINITIONS(operator*, operator*=, operator*=)
MUSTARD_MATH_ESTIMATE_SCALAR_BINARY_OP_DEFINITIONS(operator/, operator/=, DivideInPlace)
MUSTARD_MATH_ESTIMATE_SCALAR_BINARY_OP_DEFINITIONS(pow, PowInPlace, ExpInPlace)
#undef MUSTARD_MATH_ESTIMATE_SCALAR_BINARY_OP_DEFINITIONS

#undef MUSTARD_MATH_ESTIMATE_BINARY_OP_RET_TYPE

namespace impl {

#define MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Op, OpInPlace)                   \
    template<typename ADerived, int K, CovarianceOption C>                                 \
        requires GoodStatisticDimension<K>::value                                          \
    auto EstimateBase<ADerived, K, C>::Op() const& -> ADerived {                           \
        MUSTARD_MATH_ESTIMATE_TWO_STEP_FORWARDING_FUNCTION_BODY(auto, Self(), OpInPlace, ) \
    }                                                                                      \
                                                                                           \
    template<typename ADerived, int K, CovarianceOption C>                                 \
        requires GoodStatisticDimension<K>::value                                          \
    auto EstimateBase<ADerived, K, C>::Op() && -> ADerived {                               \
        return std::move(OpInPlace());                                                     \
    }

MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Abs, AbsInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Acos, AcosInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Acosh, AcoshInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Asin, AsinInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Asinh, AsinhInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Atan, AtanInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Atanh, AtanhInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Cbrt, CbrtInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Cos, CosInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Cosh, CoshInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Cube, CubeInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Erf, ErfInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Erfc, ErfcInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Exp, ExpInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Exp2, Exp2InPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Expm1, Expm1InPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Inverse, InverseInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Lgamma, LgammaInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Log, LogInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Log10, Log10InPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Log1p, Log1pInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Log2, Log2InPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Logistic, LogisticInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Ndtri, NdtriInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Negate, NegateInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Normalized, Normalize)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Rsqrt, RsqrtInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Sin, SinInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Sinh, SinhInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Sqrt, SqrtInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Square, SquareInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Tan, TanInPlace)
MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS(Tanh, TanhInPlace)
#undef MUSTARD_MATH_ESTIMATE_UNARY_MATH_FUNC_DEFINITIONS

#define MUSTARD_MATH_ESTIMATE_BINARY_MATH_FUNC_DEFINITION(Op, OpInPlace)                            \
    template<typename ADerived, int K, CovarianceOption C>                                          \
        requires GoodStatisticDimension<K>::value                                                   \
    template<int L, CovarianceOption D>                                                             \
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)                              \
    auto EstimateBase<ADerived, K, C>::Op(const Estimate<L, D>& other) const& -> ADerived {         \
        MUSTARD_MATH_ESTIMATE_TWO_STEP_FORWARDING_FUNCTION_BODY(auto, Self(), OpInPlace, other)     \
    }                                                                                               \
                                                                                                    \
    template<typename ADerived, int K, CovarianceOption C>                                          \
        requires GoodStatisticDimension<K>::value                                                   \
    template<int L, CovarianceOption D>                                                             \
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)                              \
    auto EstimateBase<ADerived, K, C>::Op(const Estimate<L, D>& other) && -> ADerived {             \
        return std::move(OpInPlace(other));                                                         \
    }                                                                                               \
                                                                                                    \
    template<typename ADerived, int K, CovarianceOption C>                                          \
        requires GoodStatisticDimension<K>::value                                                   \
    template<typename AVec>                                                                         \
        requires(K != 1)                                                                            \
    auto EstimateBase<ADerived, K, C>::Op(const Eigen::MatrixBase<AVec>& yXpr) const& -> ADerived { \
        MUSTARD_MATH_ESTIMATE_TWO_STEP_FORWARDING_FUNCTION_BODY(auto, Self(), OpInPlace, yXpr)      \
    }                                                                                               \
                                                                                                    \
    template<typename ADerived, int K, CovarianceOption C>                                          \
        requires GoodStatisticDimension<K>::value                                                   \
    template<typename AVec>                                                                         \
        requires(K != 1)                                                                            \
    auto EstimateBase<ADerived, K, C>::Op(const Eigen::MatrixBase<AVec>& yXpr) && -> ADerived {     \
        return std::move(OpInPlace(yXpr));                                                          \
    }                                                                                               \
                                                                                                    \
    template<typename ADerived, int K, CovarianceOption C>                                          \
        requires GoodStatisticDimension<K>::value                                                   \
    auto EstimateBase<ADerived, K, C>::Op(double c) const& -> ADerived {                            \
        MUSTARD_MATH_ESTIMATE_TWO_STEP_FORWARDING_FUNCTION_BODY(auto, Self(), OpInPlace, c)         \
    }                                                                                               \
                                                                                                    \
    template<typename ADerived, int K, CovarianceOption C>                                          \
        requires GoodStatisticDimension<K>::value                                                   \
    auto EstimateBase<ADerived, K, C>::Op(double c) && -> ADerived {                                \
        return std::move(OpInPlace(c));                                                             \
    }

MUSTARD_MATH_ESTIMATE_BINARY_MATH_FUNC_DEFINITION(Divide, DivideInPlace)
MUSTARD_MATH_ESTIMATE_BINARY_MATH_FUNC_DEFINITION(Exp, ExpInPlace)
MUSTARD_MATH_ESTIMATE_BINARY_MATH_FUNC_DEFINITION(NegateAdd, NegateAddInPlace)
MUSTARD_MATH_ESTIMATE_BINARY_MATH_FUNC_DEFINITION(Pow, PowInPlace)
#undef MUSTARD_MATH_ESTIMATE_BINARY_MATH_FUNC_DEFINITION

#define MUSTARD_MATH_ESTIMATE_BINARY_MATH_FUNC_WITH_RVALUE_OPERAND_DEFINITION(Op, OpInPlace)               \
    template<typename ADerived, int K, CovarianceOption C>                                                 \
        requires GoodStatisticDimension<K>::value                                                          \
    template<int L, CovarianceOption D>                                                                    \
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)                                     \
    auto EstimateBase<ADerived, K, C>::Op(Estimate<L, D>&& other) const& -> ADerived {                     \
        MUSTARD_MATH_ESTIMATE_TWO_STEP_FORWARDING_FUNCTION_BODY(auto, Self(), OpInPlace, std::move(other)) \
    }                                                                                                      \
                                                                                                           \
    template<typename ADerived, int K, CovarianceOption C>                                                 \
        requires GoodStatisticDimension<K>::value                                                          \
    template<int L, CovarianceOption D>                                                                    \
        requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)                                     \
    auto EstimateBase<ADerived, K, C>::Op(Estimate<L, D>&& other) && -> ADerived {                         \
        return std::move(OpInPlace(std::move(other)));                                                     \
    }

MUSTARD_MATH_ESTIMATE_BINARY_MATH_FUNC_WITH_RVALUE_OPERAND_DEFINITION(Divide, DivideInPlace)
#undef MUSTARD_MATH_ESTIMATE_BINARY_MATH_FUNC_WITH_RVALUE_OPERAND_DEFINITION

} // namespace impl

#undef MUSTARD_MATH_ESTIMATE_TWO_STEP_FORWARDING_FUNCTION_BODY

} // namespace Mustard::inline Math
