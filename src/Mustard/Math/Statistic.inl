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

MUSTARD_ALWAYS_INLINE Statistic<1>::Statistic() :
    fN{},
    fW{},
    fW2{},
    fM{},
    fM2{} {}

MUSTARD_ALWAYS_INLINE Statistic<1>::Statistic(const SerializedType& data) :
    Statistic{} {
    Deserialize(data);
}

MUSTARD_ALWAYS_INLINE auto Statistic<1>::Fill(double x, double w) -> void {
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
    fM2 += w * (fW / prevW) * muc::pow(x - fM / fW, 2);
}

MUSTARD_ALWAYS_INLINE auto Statistic<1>::operator+=(const Statistic& other) -> Statistic& {
    if (other.fW == 0) {
        return *this;
    }
    fN += other.fN;
    const auto prevW{fW};
    fW += other.fW;
    fW2 += other.fW2;
    fM += other.fM;
    if (prevW == 0 or fW == 0) [[unlikely]] {
        return *this;
    }
    fM2 += other.fM2;
    fM2 += other.fW * (fW / prevW) * muc::pow(fM / fW - other.fM / other.fW, 2);
    return *this;
}

MUSTARD_ALWAYS_INLINE auto Statistic<1>::Serialize() const -> SerializedType {
    SerializedType data;
    data.fN = fN;
    data.fW = fW;
    data.fW2 = fW2;
    *data.fM = fM;
    *data.fM2 = fM2;
    return data;
}

MUSTARD_ALWAYS_INLINE auto Statistic<1>::Deserialize(const SerializedType& data) -> void {
    fN = data.fN;
    fW = data.fW;
    fW2 = data.fW2;
    fM = *data.fM;
    fM2 = *data.fM2;
}

MUSTARD_ALWAYS_INLINE auto Statistic<1>::Debias() const -> double {
    const auto tmp{muc::pow(fW, 2)};
    return tmp / (tmp - fW2);
}

template<int N>
    requires(0 < N and N * N <= EIGEN_STACK_ALLOCATION_LIMIT / sizeof(double))
MUSTARD_STRONG_INLINE Statistic<N>::Statistic() :
    fN{},
    fW{},
    fW2{},
    fM{},
    fM2{} {
    // Eigen does not zero-initialize by default, so we need to do it ourselves.
    fM.setZero();
    fM2.setZero();
}

template<int N>
    requires(0 < N and N * N <= EIGEN_STACK_ALLOCATION_LIMIT / sizeof(double))
MUSTARD_STRONG_INLINE Statistic<N>::Statistic(const SerializedType& data) :
    Statistic{} {
    Deserialize(data);
}

template<int N>
    requires(0 < N and N * N <= EIGEN_STACK_ALLOCATION_LIMIT / sizeof(double))
template<typename T>
    requires Concept::InputVectorAny<std::decay_t<T>, N>
MUSTARD_STRONG_INLINE auto Statistic<N>::Fill(T&& x0, double w) -> void {
    if (w == 0) {
        return;
    }
    ++fN;
    const auto prevW{fW};
    fW += w;
    fW2 += muc::pow(w, 2);
    const auto& x{VectorCast<MeanType>(x0)};
    fM += w * x;
    if (prevW == 0 or fW == 0) [[unlikely]] {
        return;
    }
    const auto delta{(x - fM / fW).eval()};
    fM2 += w * (fW / prevW) * delta * delta.transpose();
}

template<int N>
    requires(0 < N and N * N <= EIGEN_STACK_ALLOCATION_LIMIT / sizeof(double))
MUSTARD_STRONG_INLINE auto Statistic<N>::operator+=(const Statistic& other) -> Statistic& {
    if (other.fW == 0) {
        return *this;
    }
    fN += other.fN;
    const auto prevW{fW};
    fW += other.fW;
    fW2 += other.fW2;
    fM += other.fM;
    if (prevW == 0 or fW == 0) [[unlikely]] {
        return *this;
    }
    const auto delta{(fM / fW - other.fM / other.fW).eval()};
    fM2 += other.fM2;
    fM2 += other.fW * (fW / prevW) * delta * delta.transpose();
    return *this;
}

template<int N>
    requires(0 < N and N * N <= EIGEN_STACK_ALLOCATION_LIMIT / sizeof(double))
MUSTARD_STRONG_INLINE auto Statistic<N>::Serialize() const -> SerializedType {
    SerializedType data;
    data.fN = fN;
    data.fW = fW;
    data.fW2 = fW2;
    std::memcpy(data.fM, fM.data(), N * sizeof(double));
    std::memcpy(data.fM2, fM2.data(), N * N * sizeof(double));
    return data;
}

template<int N>
    requires(0 < N and N * N <= EIGEN_STACK_ALLOCATION_LIMIT / sizeof(double))
MUSTARD_STRONG_INLINE auto Statistic<N>::Deserialize(const SerializedType& data) -> void {
    fN = data.fN;
    fW = data.fW;
    fW2 = data.fW2;
    std::memcpy(fM.data(), data.fM, N * sizeof(double));
    std::memcpy(fM2.data(), data.fM2, N * N * sizeof(double));
}

template<int N>
    requires(0 < N and N * N <= EIGEN_STACK_ALLOCATION_LIMIT / sizeof(double))
MUSTARD_ALWAYS_INLINE auto Statistic<N>::Debias() const -> double {
    const auto tmp{muc::pow(fW, 2)};
    return tmp / (tmp - fW2);
}

template<int N>
    requires(0 < N and N * N <= EIGEN_STACK_ALLOCATION_LIMIT / sizeof(double))
SerializedStatistic<N>::SerializedStatistic() :
    fN{},
    fW{},
    fW2{},
    fM{},
    fM2{} {}

template<int N>
    requires(0 < N and N * N <= EIGEN_STACK_ALLOCATION_LIMIT / sizeof(double))
SerializedStatistic<N>::SerializedStatistic(const std::string& base64) :
    SerializedStatistic{} {
    DecodeBase64(base64);
}

template<int N>
    requires(0 < N and N * N <= EIGEN_STACK_ALLOCATION_LIMIT / sizeof(double))
auto SerializedStatistic<N>::EncodeBase64() const -> std::string {
    const auto byteArray{std::bit_cast<std::array<char, sizeof(*this)>>(*this)};
    return std::string{TBase64::Encode(byteArray.data(), byteArray.size()).View()};
}

template<int N>
    requires(0 < N and N * N <= EIGEN_STACK_ALLOCATION_LIMIT / sizeof(double))
auto SerializedStatistic<N>::DecodeBase64(const std::string& base64) -> void {
    const auto byteString{TBase64::Decode(base64.c_str())};
    if (byteString.Length() != sizeof(*this)) {
        Throw<std::runtime_error>(fmt::format("Decoded base64 string is invalid. Expected {} bytes, got {} bytes.",
                                              sizeof(*this), byteString.Length()));
    }
    std::memcpy(this, byteString.Data(), sizeof(*this));
}

} // namespace Mustard::inline Math
