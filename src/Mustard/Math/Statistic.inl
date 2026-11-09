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
// StatisticBase
// =========================================================================

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
StatisticBase<ADerived, K, C>::StatisticBase(std::monostate) :
    fN{},
    fW{},
    fW2{},
    fM{},
    fM2{} {}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
StatisticBase<ADerived, K, C>::StatisticBase() :
    StatisticBase{std::monostate{}} {
    if constexpr (K != Eigen::Dynamic) {
        fM.setZero();
        fM2.setZero();
    }
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
StatisticBase<ADerived, K, C>::StatisticBase(const StatisticBase& other) :
    StatisticBase{std::monostate{}} {
    CopyFrom(other);
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
StatisticBase<ADerived, K, C>::StatisticBase([[maybe_unused]] int dim) :
    StatisticBase{std::monostate{}} {
    if constexpr (K == Eigen::Dynamic) {
        if (dim <= 0) {
            Throw<std::invalid_argument>(fmt::format("Dimension must be positive, got {}.", dim));
        }
        fM.resize(dim);
        if constexpr (C == CovarianceOption::Full) {
            fM2.resize(dim, dim);
        } else {
            fM2.resize(dim);
        }
    } else {
        assert(dim == K);
    }
    fM.setZero();
    fM2.setZero();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<int L, CovarianceOption D>
    requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
StatisticBase<ADerived, K, C>::StatisticBase(const Statistic<L, D>& other) :
    StatisticBase{std::monostate{}} {
    CopyFrom(other);
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
StatisticBase<ADerived, K, C>::StatisticBase(const PODType& data) // clang-format off
    requires(K != Eigen::Dynamic) : // clang-format on
    StatisticBase{std::monostate{}} {
    FromPOD(data);
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto StatisticBase<ADerived, K, C>::Dimension() const -> int {
    if constexpr (K != Eigen::Dynamic) {
        return K;
    } else {
        return fM.size();
    }
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<typename AVec>
auto StatisticBase<ADerived, K, C>::Fill(const Eigen::MatrixBase<AVec>& xXpr, double w) -> void {
    const auto& x{xXpr.eval()};
    CheckDimensionMatch(x);
    if (w == 0) {
        return;
    }
    ++fN;
    const auto prevW{fW};
    fW += w;
    fW2 += muc::pow(w, 2);
    fM += w * x;
    if (prevW == 0 or fW == 0) [[unlikely]] {
        return;
    }
    AppendM2CrossTerm(w, prevW, x - MeanXpr());
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto StatisticBase<ADerived, K, C>::Correlation(int i, int j) const -> double {
    return Corr(i, j, [this](int a, int b) { return Covariance(a, b); }, [this](int a) { return Variance(a); });
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto StatisticBase<ADerived, K, C>::CorrelationOfMean(int i, int j) const -> double {
    return Corr(i, j, [this](int a, int b) { return CovarianceOfMean(a, b); }, [this](int a) { return VarianceOfMean(a); });
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<int L, CovarianceOption D>
    requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
auto StatisticBase<ADerived, K, C>::operator+=(const Statistic<L, D>& other) -> ADerived& {
    CheckDimensionMatch(other.fM);
    fN += other.fN;
    const auto prevW{fW};
    fW += other.fW;
    fW2 += other.fW2;
    fM += other.fM;
    if constexpr (C == D and D == CovarianceOption::Full) {
        fM2 += other.fM2;
    } else {
        fM2.diagonal() += other.fM2.diagonal();
    }
    if (prevW == 0 or fW == 0 or other.fW == 0) [[unlikely]] {
        return Self();
    }
    AppendM2CrossTerm(other.fW, prevW, MeanXpr() - other.MeanXpr());
    return Self();
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto StatisticBase<ADerived, K, C>::ToPOD() const -> PODType
    requires(K != Eigen::Dynamic) {
    PODType data;
    data.n = fN;
    data.w = fW;
    data.w2 = fW2;
    std::memcpy(data.m, fM.data(), K * sizeof(double));
    if constexpr (C == CovarianceOption::Full) {
        std::memcpy(data.m2, fM2.data(), K * K * sizeof(double));
    } else {
        std::memcpy(data.m2, fM2.diagonal().data(), K * sizeof(double));
    }
    return data;
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto StatisticBase<ADerived, K, C>::FromPOD(const PODType& data) -> void
    requires(K != Eigen::Dynamic) {
    fN = data.n;
    fW = data.w;
    fW2 = data.w2;
    std::memcpy(fM.data(), data.m, K * sizeof(double));
    if constexpr (C == CovarianceOption::Full) {
        std::memcpy(fM2.data(), data.m2, K * K * sizeof(double));
    } else {
        std::memcpy(fM2.diagonal().data(), data.m2, K * sizeof(double));
    }
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto StatisticBase<ADerived, K, C>::ToFlat() const -> std::pair<std::vector<long long>, std::vector<double>> {
    const auto dim{Dimension()};
    std::vector<long long> flatCount{fN, dim};
    std::vector<double> flatValue;
    flatValue.reserve(2 + dim + (C == CovarianceOption::Full ? dim * dim : dim));
    flatValue.push_back(fW);
    flatValue.push_back(fW2);
    flatValue.insert(flatValue.end(), fM.data(), fM.data() + dim);
    // Store the weighted sum of squares, which is additive across independent
    // groups, instead of the within-group M2 statistic.
    if constexpr (C == CovarianceOption::Full) {
        Eigen::Matrix<double, K, K> sumSquare{Eigen::Matrix<double, K, K>::Zero(dim, dim)};
        if (fW != 0) {
            sumSquare = fM2 + fM * fM.transpose() / fW;
        }
        flatValue.insert(flatValue.end(), sumSquare.data(), sumSquare.data() + dim * dim);
    } else {
        Eigen::Vector<double, K> sumSquare{Eigen::Vector<double, K>::Zero(dim)};
        if (fW != 0) {
            sumSquare = fM2.diagonal() + fM.cwiseAbs2() / fW;
        }
        flatValue.insert(flatValue.end(), sumSquare.data(), sumSquare.data() + dim);
    }
    return {std::move(flatCount), std::move(flatValue)};
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto StatisticBase<ADerived, K, C>::FromFlat(const std::pair<std::vector<long long>, std::vector<double>>& data) -> void {
    const auto& flatCount{data.first};
    const auto& flatValue{data.second};
    if (flatCount.size() != 2) {
        Throw<std::invalid_argument>(fmt::format("Flat statistic counts must contain 2 elements, got {}.", flatCount.size()));
    }
    const auto dim{static_cast<int>(flatCount[1])};
    if (dim <= 0) {
        Throw<std::invalid_argument>(fmt::format("Flat statistic dimension must be positive, got {}.", dim));
    }
    if constexpr (K != Eigen::Dynamic) {
        if (dim != K) {
            Throw<std::invalid_argument>(fmt::format("Flat statistic dimension ({}) does not match the statistic dimension ({}).", dim, K));
        }
    }
    const auto expectedSize{2 + dim + (C == CovarianceOption::Full ? dim * dim : dim)};
    if (static_cast<int>(flatValue.size()) != expectedSize) {
        Throw<std::invalid_argument>(fmt::format("Flat statistic values must contain {} elements, got {}.", expectedSize, flatValue.size()));
    }
    if (flatCount[0] < 0) {
        Throw<std::invalid_argument>(fmt::format("Flat statistic observation count must be non-negative, got {}.", flatCount[0]));
    }
    if constexpr (K == Eigen::Dynamic) {
        fM.resize(dim);
        if constexpr (C == CovarianceOption::Full) {
            fM2.resize(dim, dim);
        } else {
            fM2.resize(dim);
        }
    }
    fN = flatCount[0];
    fW = flatValue[0];
    fW2 = flatValue[1];
    std::copy_n(flatValue.data() + 2, dim, fM.data());
    if constexpr (C == CovarianceOption::Full) {
        std::copy_n(flatValue.data() + 2 + dim, dim * dim, fM2.data());
        if (fW != 0) {
            fM2 -= fM * fM.transpose() / fW;
        } else {
            fM2.setZero();
        }
    } else {
        std::copy_n(flatValue.data() + 2 + dim, dim, fM2.diagonal().data());
        if (fW != 0) {
            fM2.diagonal() -= fM.cwiseAbs2() / fW;
        } else {
            fM2.diagonal().setZero();
        }
    }
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto StatisticBase<ADerived, K, C>::Corr(int i, int j, auto cov, auto var) const -> double {
    if (i == j) {
        return 1;
    }
    if constexpr (C == CovarianceOption::Diagonal) {
        return 0;
    }
    return cov(i, j) / std::sqrt(var(i) * var(j));
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
auto StatisticBase<ADerived, K, C>::Corr(auto covXpr, auto varXpr) const -> CovarianceType {
    if constexpr (C == CovarianceOption::Full) {
        const auto sigma{(this->*varXpr)().cwiseSqrt().cwiseInverse().eval()};
        auto corr{(sigma.asDiagonal() * (this->*covXpr)() * sigma.asDiagonal()).eval()};
        corr.diagonal().setOnes();
        return corr;
    } else {
        CovarianceType corr(Dimension());
        corr.diagonal().setOnes();
        return corr;
    }
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<typename AVec>
auto StatisticBase<ADerived, K, C>::AppendM2CrossTerm(double otherW, double prevW, const Eigen::MatrixBase<AVec>& deltaXpr) -> void {
    const auto www{otherW * (fW / prevW)};
    if constexpr (C == CovarianceOption::Full) {
        if constexpr (K <= 8 and K != Eigen::Dynamic) {
            const auto& delta{deltaXpr.eval()};
            fM2.noalias() += www * delta * delta.transpose();
        } else {
            fM2.template selfadjointView<Eigen::Lower>().rankUpdate(deltaXpr, www);
            fM2.template triangularView<Eigen::Upper>() = fM2.transpose();
        }
    } else {
        fM2.diagonal() += www * deltaXpr.cwiseSquare();
    }
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<typename AVec>
    requires((AVec::RowsAtCompileTime == K or
              K == Eigen::Dynamic or AVec::RowsAtCompileTime == Eigen::Dynamic) and
             AVec::ColsAtCompileTime == 1)
MUSTARD_ALWAYS_INLINE auto StatisticBase<ADerived, K, C>::CheckDimensionMatch(const Eigen::MatrixBase<AVec>& y) const -> void {
    if constexpr (K == Eigen::Dynamic or AVec::RowsAtCompileTime == Eigen::Dynamic) {
        if (Dimension() != y.size()) {
            Throw<std::invalid_argument>(fmt::format("Dimension mismatch: dim(self)={}, dim(other)={}",
                                                     Dimension(), y.size()));
        }
    }
}

template<typename ADerived, int K, CovarianceOption C>
    requires GoodStatisticDimension<K>::value
template<typename AOther, int L, CovarianceOption D>
    requires(L == K or K == Eigen::Dynamic or L == Eigen::Dynamic)
auto StatisticBase<ADerived, K, C>::CopyFrom(const StatisticBase<AOther, L, D>& other) & -> ADerived& {
    if constexpr (K != Eigen::Dynamic) { // dynamic dimension matrices will be resized so do not check
        CheckDimensionMatch(other.fM);
    }
    fN = other.fN;
    fW = other.fW;
    fW2 = other.fW2;
    fM = other.fM;
    if constexpr (C == D) {
        fM2 = other.fM2;
    } else {
        if constexpr (K == Eigen::Dynamic) {
            if constexpr (C == CovarianceOption::Full) {
                fM2.resize(other.Dimension(), other.Dimension());
            } else {
                fM2.resize(other.Dimension());
            }
        }
        if constexpr (C == CovarianceOption::Full) {
            fM2.setZero();
        }
        fM2.diagonal() = other.fM2.diagonal();
    }
    return Self();
}

} // namespace impl

// =========================================================================
// operator+(Statistic<K, C>, Statistic<L, D>)
// =========================================================================

template<int K, CovarianceOption C, int L, CovarianceOption D>
auto operator+(const Statistic<K, C>& lhs, const Statistic<L, D>& rhs) -> impl::StatisticCombineResultType<K, C, L, D> {
    impl::StatisticCombineResultType<K, C, L, D> result{lhs};
    result += rhs;
    return result; // NRVO
}

template<int K, CovarianceOption C, int L, CovarianceOption D>
auto operator+(const Statistic<K, C>& lhs, Statistic<L, D>&& rhs) -> impl::StatisticCombineResultType<K, C, L, D> {
    impl::StatisticCombineResultType<K, C, L, D> result{std::move(rhs)};
    result += lhs;
    return result; // NRVO
}

template<int K, CovarianceOption C, int L, CovarianceOption D>
auto operator+(Statistic<K, C>&& lhs, const Statistic<L, D>& rhs) -> impl::StatisticCombineResultType<K, C, L, D> {
    impl::StatisticCombineResultType<K, C, L, D> result{std::move(lhs)};
    result += rhs;
    return result; // NRVO
}

template<int K, CovarianceOption C, int L, CovarianceOption D>
auto operator+(Statistic<K, C>&& lhs, Statistic<L, D>&& rhs) -> impl::StatisticCombineResultType<K, C, L, D> {
    if constexpr (std::same_as<impl::StatisticCombineResultType<K, C, L, D>, Statistic<K, C>> or
                  not std::same_as<impl::StatisticCombineResultType<K, C, L, D>, Statistic<L, D>>) {
        impl::StatisticCombineResultType<K, C, L, D> result{std::move(lhs)};
        result += rhs;
        return result; // NRVO
    } else {
        impl::StatisticCombineResultType<K, C, L, D> result{std::move(rhs)};
        result += lhs;
        return result; // NRVO
    }
}

} // namespace Mustard::inline Math
