// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

#include "Mustard/Concept/MathVector.h++"
#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Math/Random/Distribution/UniformRectangle.h++"
#include "Mustard/Math/Random/RandomNumberDistributionBase.h++"
#include "Mustard/Utility/InlineMacro.h++"
#include "Mustard/Utility/VectorValueType.h++"

#include "muc/array"
#include "muc/concepts"
#include "muc/math"

#include <array>
#include <cmath>
#include <concepts>
#include <iomanip>

namespace Mustard::Math::Random::inline Distribution {

namespace internal {

template<Concept::NumericVector2Any T, template<typename> typename AUniformDisk>
class BasicUniformDiskParameter final : public DistributionParameterBase<BasicUniformDiskParameter<T, AUniformDisk>,
                                                                         AUniformDisk<T>> {
private:
    using VT = VectorValueType<T>;
    using Base = DistributionParameterBase<BasicUniformDiskParameter<T, AUniformDisk>,
                                           AUniformDisk<T>>;

public:
    constexpr BasicUniformDiskParameter();
    constexpr BasicUniformDiskParameter(VT r, VT x0, VT y0);
    constexpr BasicUniformDiskParameter(VT radius, T center);
    constexpr explicit BasicUniformDiskParameter(VT radius);

    constexpr auto Radius() const -> auto { return fRadius; }
    constexpr auto CenterX() const -> auto { return fCenterX; }
    constexpr auto CenterY() const -> auto { return fCenterY; }
    constexpr auto Center() const -> T { return {fCenterX, fCenterY}; }

    constexpr auto Radius(VT r) -> void { fRadius = r; }
    constexpr auto CenterX(VT x0) -> void { fCenterX = x0; }
    constexpr auto CenterY(VT y0) -> void { fCenterY = y0; }
    constexpr auto Center(VT x0, VT y0) -> void { CenterX(x0), CenterY(y0); }
    constexpr auto Center(T r0) -> void { Center(r0[0], r0[1]); }

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const BasicUniformDiskParameter& self) -> decltype(os) { return self.StreamOutput(os); }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, BasicUniformDiskParameter& self) -> decltype(is) { return self.StreamInput(is); }

private:
    template<muc::character AChar>
    auto StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os);
    template<muc::character AChar>
    auto StreamInput(std::basic_istream<AChar>& is) & -> decltype(is);

private:
    VT fRadius;
    VT fCenterX;
    VT fCenterY;
};

template<Concept::NumericVector2Any T, template<typename> typename AUniformDisk>
class UniformDiskBase : public RandomNumberDistributionBase<AUniformDisk<T>,
                                                            BasicUniformDiskParameter<T, AUniformDisk>,
                                                            T> {
private:
    using VT = VectorValueType<T>;
    using Base = RandomNumberDistributionBase<AUniformDisk<T>,
                                              BasicUniformDiskParameter<T, AUniformDisk>,
                                              T>;

public:
    constexpr UniformDiskBase() = default;
    constexpr UniformDiskBase(VT r, VT x0, VT y0);
    constexpr UniformDiskBase(VT radius, T center);
    constexpr explicit UniformDiskBase(VT radius);
    constexpr explicit UniformDiskBase(const typename Base::ParameterType& p);

protected:
    constexpr ~UniformDiskBase() = default;

public:
    constexpr auto Reset() -> void {}

    constexpr auto Parameter() const -> auto { return fParameter; }
    constexpr auto Radius() const -> auto { return fParameter.Radius(); }
    constexpr auto CenterX() const -> auto { return fParameter.CenterX(); }
    constexpr auto CenterY() const -> auto { return fParameter.CenterY(); }
    constexpr auto Center() const -> auto { return fParameter.Center(); }

    constexpr auto Parameter(const typename Base::ParameterType& p) -> void { fParameter = p; }
    constexpr auto Radius(VT r) -> void { fParameter.Radius(r); }
    constexpr auto CenterX(VT x0) -> void { fParameter.CenterX(x0); }
    constexpr auto CenterY(VT y0) -> void { fParameter.CenterY(y0); }
    constexpr auto Center(VT x0, VT y0) -> void { fParameter.Center(x0, y0); }
    constexpr auto Center(T r0) -> void { fParameter.Center(r0); }

    constexpr auto Min() const -> T { return {CenterX() - Radius(), CenterY() - Radius()}; }
    constexpr auto Max() const -> T { return {CenterX() + Radius(), CenterY() + Radius()}; }

    static constexpr auto Stateless() -> bool { return true; }

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const UniformDiskBase<T, AUniformDisk>& self) -> auto& { return os << self.fParameter; }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, UniformDiskBase<T, AUniformDisk>& self) -> auto& { return is >> self.fParameter; }

protected:
    static constexpr auto UniformCompactDisk(UniformRandomBitGenerator auto& g, const DistributionParameter auto& p) -> std::pair<T, VT>;

protected:
    typename Base::ParameterType fParameter;
};

} // namespace internal

template<Concept::NumericVector2Any T = muc::array2d>
class UniformCompactDisk;

template<Concept::NumericVector2Any T>
using UniformCompactDiskParameter = internal::BasicUniformDiskParameter<T, UniformCompactDisk>;

template<Concept::NumericVector2Any T>
class UniformCompactDisk final : public internal::UniformDiskBase<T, UniformCompactDisk> {
public:
    using internal::UniformDiskBase<T, UniformCompactDisk>::UniformDiskBase;

    MUSTARD_STRONG_INLINE constexpr auto operator()(UniformRandomBitGenerator auto& g) -> auto { return (*this)(g, this->fParameter); }
    MUSTARD_STRONG_INLINE constexpr auto operator()(UniformRandomBitGenerator auto& g, const UniformCompactDiskParameter<T>& p) -> T;
};

template<typename VT, typename T>
UniformCompactDisk(VT, T) -> UniformCompactDisk<std::enable_if_t<std::same_as<VT, VectorValueType<std::decay_t<T>>>, std::decay_t<T>>>;
template<typename T, typename U, typename V>
UniformCompactDisk(T, U, V) -> UniformCompactDisk<std::array<std::common_type_t<T, U, V>, 2>>;
template<typename T>
UniformCompactDisk(T) -> UniformCompactDisk<std::array<T, 2>>;

template<Concept::NumericVector2Any T = muc::array2d>
class UniformDisk;

template<Concept::NumericVector2Any T>
using UniformDiskParameter = internal::BasicUniformDiskParameter<T, UniformDisk>;

template<Concept::NumericVector2Any T>
class UniformDisk final : public internal::UniformDiskBase<T, UniformDisk> {
public:
    using internal::UniformDiskBase<T, UniformDisk>::UniformDiskBase;

    MUSTARD_STRONG_INLINE constexpr auto operator()(UniformRandomBitGenerator auto& g) -> auto { return (*this)(g, this->fParameter); }
    MUSTARD_STRONG_INLINE constexpr auto operator()(UniformRandomBitGenerator auto& g, const UniformDiskParameter<T>& p) -> T;
};

template<typename VT, typename T>
UniformDisk(VT, T) -> UniformDisk<std::enable_if_t<std::same_as<VT, VectorValueType<std::decay_t<T>>>, std::decay_t<T>>>;
template<typename T, typename U, typename V>
UniformDisk(T, U, V) -> UniformDisk<std::array<std::common_type_t<T, U, V>, 2>>;
template<typename T>
UniformDisk(T) -> UniformDisk<std::array<T, 2>>;

} // namespace Mustard::Math::Random::inline Distribution

#include "Mustard/Math/Random/Distribution/UniformDisk.inl"
