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

#include "Mustard/Concept/MathVector.h++"
#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/Distribution/UniformRectangle.h++"
#include "Mustard/Math/Random/Distribution/impl/FastCosSinOnPiPi.h++"
#include "Mustard/Math/Random/RandomNumberDistributionBase.h++"
#include "Mustard/Utility/ClassSemanticHelper.h++"
#include "Mustard/Utility/VectorValueType.h++"

#include "CLHEP/Random/RandomEngine.h"

#include "TRandom.h"

#include "muc/array"
#include "muc/concepts"
#include "muc/math"

#include <array>
#include <cmath>
#include <concepts>
#include <iomanip>
#include <numbers>
#include <random>

namespace Mustard::inline Math::Random::inline Distribution {

namespace impl {

template<Concept::NumericVector2FloatingPoint T, template<typename> typename AUniformDisk>
class BasicUniformDiskParameter final : public DistributionParameterBase<BasicUniformDiskParameter<T, AUniformDisk>,
                                                                         AUniformDisk<T>> {
private:
    using Scalar = VectorValueType<T>;
    using Base = DistributionParameterBase<BasicUniformDiskParameter<T, AUniformDisk>,
                                           AUniformDisk<T>>;

public:
    constexpr BasicUniformDiskParameter();
    constexpr BasicUniformDiskParameter(Scalar r, Scalar x0, Scalar y0);
    constexpr BasicUniformDiskParameter(Scalar radius, T center);
    constexpr explicit BasicUniformDiskParameter(Scalar radius);

    constexpr auto Radius() const -> auto { return fRadius; }
    constexpr auto CenterX() const -> auto { return fCenterX; }
    constexpr auto CenterY() const -> auto { return fCenterY; }
    constexpr auto Center() const -> T { return {fCenterX, fCenterY}; }

    constexpr auto Radius(Scalar r) -> void { fRadius = r; }
    constexpr auto CenterX(Scalar x0) -> void { fCenterX = x0; }
    constexpr auto CenterY(Scalar y0) -> void { fCenterY = y0; }
    constexpr auto Center(Scalar x0, Scalar y0) -> void { CenterX(x0), CenterY(y0); }
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
    Scalar fRadius;
    Scalar fCenterX;
    Scalar fCenterY;
};

template<Concept::NumericVector2FloatingPoint T, template<typename> typename AUniformDisk>
class UniformDiskBase : public RandomNumberDistributionBase<AUniformDisk<T>,
                                                            BasicUniformDiskParameter<T, AUniformDisk>,
                                                            T> {
private:
    using Scalar = VectorValueType<T>;
    using Base = RandomNumberDistributionBase<AUniformDisk<T>,
                                              BasicUniformDiskParameter<T, AUniformDisk>,
                                              T>;

public:
    using typename Base::ParameterType;

public:
    constexpr UniformDiskBase() = default;
    constexpr UniformDiskBase(Scalar r, Scalar x0, Scalar y0);
    constexpr UniformDiskBase(Scalar radius, T center);
    constexpr explicit UniformDiskBase(Scalar radius);
    constexpr explicit UniformDiskBase(const typename ParameterType& p);

protected:
    MUSTARD_UTILITY_DECLARE_NONVIRTUAL_PROTECTED_DEFAULT_DESTRUCTOR_IN_DEFAULT_MOVEABLE(UniformDiskBase)

public:
    constexpr auto Reset() -> void {}

    constexpr auto Parameter() const -> auto { return fParameter; }
    constexpr auto Radius() const -> auto { return fParameter.Radius(); }
    constexpr auto CenterX() const -> auto { return fParameter.CenterX(); }
    constexpr auto CenterY() const -> auto { return fParameter.CenterY(); }
    constexpr auto Center() const -> auto { return fParameter.Center(); }

    constexpr auto Parameter(const typename ParameterType& p) -> void { fParameter = p; }
    constexpr auto Radius(Scalar r) -> void { fParameter.Radius(r); }
    constexpr auto CenterX(Scalar x0) -> void { fParameter.CenterX(x0); }
    constexpr auto CenterY(Scalar y0) -> void { fParameter.CenterY(y0); }
    constexpr auto Center(Scalar x0, Scalar y0) -> void { fParameter.Center(x0, y0); }
    constexpr auto Center(T r0) -> void { fParameter.Center(r0); }

    constexpr auto Min() const -> T { return {CenterX() - Radius(), CenterY() - Radius()}; }
    constexpr auto Max() const -> T { return {CenterX() + Radius(), CenterY() + Radius()}; }

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const UniformDiskBase<T, AUniformDisk>& self) -> auto& { return os << self.fParameter; }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, UniformDiskBase<T, AUniformDisk>& self) -> auto& { return is >> self.fParameter; }

protected:
    typename ParameterType fParameter;
};

} // namespace impl

template<Concept::NumericVector2FloatingPoint T = muc::array2d>
class UniformDisk;

template<Concept::NumericVector2FloatingPoint T>
using UniformDiskFastParameter = impl::BasicUniformDiskParameter<T, UniformDisk>;

template<Concept::NumericVector2FloatingPoint T>
class UniformDisk final : public impl::UniformDiskBase<UniformDisk, T> {
private:
    using Base = impl::UniformDiskBase<UniformDisk, T>;
    using typename Base::Scalar;

public:
    using impl::UniformDiskBase<UniformDisk, T>::UniformDiskBase;

    template<std::uniform_random_bit_generator G>
    auto operator()(G& g) -> auto { return Impl(g, this->fParameter); }
    template<std::uniform_random_bit_generator G>
    auto operator()(G& g, const UniformDiskFastParameter<T>& p) -> auto { return Impl(g, p); }

    auto operator()(CLHEP::HepRandomEngine& g) -> auto { return Impl(g, this->fParameter); }
    auto operator()(CLHEP::HepRandomEngine& g, const UniformDiskFastParameter<T>& p) -> auto { return Impl(g, p); }

    auto operator()(TRandom& g) -> auto { return Impl(g, this->fParameter); }
    auto operator()(TRandom& g, const UniformDiskFastParameter<T>& p) -> auto { return Impl(g, p); }

private:
    static auto Impl(auto& g, const UniformDiskFastParameter<T>& p) -> T;
};

template<typename Scalar, typename T>
UniformDisk(Scalar, T) -> UniformDisk<std::enable_if_t<std::same_as<Scalar, VectorValueType<std::decay_t<T>>>, std::decay_t<T>>>;
template<typename T, typename U, typename V>
UniformDisk(T, U, V) -> UniformDisk<std::array<std::common_type_t<T, U, V>, 2>>;
template<typename T>
UniformDisk(T) -> UniformDisk<std::array<T, 2>>;

template<Concept::NumericVector2FloatingPoint T = muc::array2d>
class UniformDiskS;

template<Concept::NumericVector2FloatingPoint T>
using UniformDiskParameter = impl::BasicUniformDiskParameter<T, UniformDiskS>;

template<Concept::NumericVector2FloatingPoint T>
class UniformDiskS final : public impl::UniformDiskBase<UniformDiskS, T> {
private:
    using Base = impl::UniformDiskBase<UniformDiskS, T>;
    using typename Base::Scalar;

public:
    using impl::UniformDiskBase<UniformDiskS, T>::UniformDiskBase;

    template<std::uniform_random_bit_generator G>
    constexpr auto operator()(G& g) -> auto { return Impl(g, this->fParameter); }
    template<std::uniform_random_bit_generator G>
    constexpr auto operator()(G& g, const UniformDiskParameter<T>& p) -> auto { return Impl(g, p); }

    auto operator()(CLHEP::HepRandomEngine& g) -> auto { return Impl(g, this->fParameter); }
    auto operator()(CLHEP::HepRandomEngine& g, const UniformDiskParameter<T>& p) -> auto { return Impl(g, p); }

    auto operator()(TRandom& g) -> auto { return Impl(g, this->fParameter); }
    auto operator()(TRandom& g, const UniformDiskParameter<T>& p) -> auto { return Impl(g, p); }

private:
    static constexpr auto Impl(auto& g, const UniformDiskParameter<T>& p) -> T;
};

template<typename Scalar, typename T>
UniformDiskS(Scalar, T) -> UniformDiskS<std::enable_if_t<std::same_as<Scalar, VectorValueType<std::decay_t<T>>>, std::decay_t<T>>>;
template<typename T, typename U, typename V>
UniformDiskS(T, U, V) -> UniformDiskS<std::array<std::common_type_t<T, U, V>, 2>>;
template<typename T>
UniformDiskS(T) -> UniformDiskS<std::array<T, 2>>;

} // namespace Mustard::inline Math::Random::inline Distribution

#include "Mustard/Math/Random/Distribution/UniformDisk.inl"
