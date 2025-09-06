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

#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Math/Random/RandomNumberDistributionBase.h++"
#include "Mustard/Utility/FunctionAttribute.h++"
#include "Mustard/gslx/index_sequence.h++"

#include "CLHEP/Random/RandomEngine.h"

#include "muc/concepts"

#include "gsl/gsl"

#include <array>
#include <concepts>
#include <tuple>
#include <type_traits>
#include <utility>

namespace Mustard::Math::Random::inline Distribution {

namespace internal {

namespace internal {

template<gsl::index I, typename T>
struct Margin {
    T value;
};

template<typename...>
struct CartesianProductMarginBase;
template<gsl::index... Is, typename... Ts>
    requires(sizeof...(Is) == sizeof...(Ts))
struct CartesianProductMarginBase<gslx::index_sequence<Is...>, Ts...> : Margin<Is, Ts>... {};

} // namespace internal

template<typename... Ts>
struct CartesianProductMargin : internal::CartesianProductMarginBase<gslx::index_sequence_for<Ts...>, Ts...> {
    CartesianProductMargin() = default;
    CartesianProductMargin(const Ts&... objects);

    template<gsl::index I>
    constexpr auto Margin() const -> const auto& { return static_cast<const internal::Margin<I, std::tuple_element_t<I, std::tuple<Ts...>>>*>(this)->value; }
    template<gsl::index I>
    constexpr auto Margin() -> auto& { return static_cast<internal::Margin<I, std::tuple_element_t<I, std::tuple<Ts...>>>*>(this)->value; }
};

} // namespace internal

template<typename ADerived, typename ADistribution, typename... Ds>
    requires(sizeof...(Ds) >= 2)
class JointParameterInterface : public DistributionParameterBase<ADerived, ADistribution>,
                                public internal::CartesianProductMargin<typename Ds::ParameterType...> {
public:
    constexpr JointParameterInterface();
    constexpr JointParameterInterface(const typename Ds::ParameterType&... p);

protected:
    constexpr ~JointParameterInterface() = default;

public:
    template<gsl::index I>
    constexpr auto Parameter() const -> const auto& { return this->template Margin<I>(); }

    template<gsl::index I>
    constexpr auto Parameter(const std::tuple_element_t<I, std::tuple<typename Ds::ParameterType...>>& p) -> void { this->template Margin<I>() = p; }

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const JointParameterInterface& self) -> decltype(os) { return self.StreamOutput(os); }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, JointParameterInterface& self) -> decltype(is) { return self.StreamInput(is); }

private:
    template<muc::character AChar>
    auto StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os);
    template<muc::character AChar>
    auto StreamInput(std::basic_istream<AChar>& is) & -> decltype(is);
};

template<typename ADerived, typename AParameter, typename T, typename... Ds>
    requires(sizeof...(Ds) >= 2 and Concept::NumericVectorAny<T, sizeof...(Ds)>)
class JointInterface : public RandomNumberDistributionBase<ADerived, AParameter, T>,
                       public internal::CartesianProductMargin<Ds...> {
public:
    constexpr JointInterface();
    constexpr JointInterface(const typename Ds::ParameterType&... p);
    constexpr explicit JointInterface(const AParameter& p);

protected:
    constexpr ~JointInterface() = default;

public:
    MUSTARD_ALWAYS_INLINE constexpr auto operator()(UniformRandomBitGenerator auto& g) -> auto { return Impl(g); }
    MUSTARD_ALWAYS_INLINE constexpr auto operator()(UniformRandomBitGenerator auto& g, const AParameter& p) -> auto { return Impl(g, p); }

    MUSTARD_ALWAYS_INLINE auto operator()(CLHEP::HepRandomEngine& g) -> auto { return Impl(g); }
    MUSTARD_ALWAYS_INLINE auto operator()(CLHEP::HepRandomEngine& g, const AParameter& p) -> auto { return Impl(g, p); }

    constexpr auto Reset() -> void;

    constexpr auto Parameter() const -> AParameter;
    template<gsl::index I>
    constexpr auto Parameter() const -> auto { return this->template Margin<I>().Parameter(); }

    constexpr auto Parameter(const AParameter& p) -> void;
    template<gsl::index I>
    constexpr auto Parameter(const typename std::tuple_element_t<I, std::tuple<Ds...>>::ParameterType& p) -> void { this->template Margin<I>().Parameter(p); }

    constexpr auto Min() const -> T;
    constexpr auto Max() const -> T;

    static constexpr auto Stateless() -> bool { return (... and Ds::Stateless()); }

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const JointInterface& self) -> decltype(os) { return self.StreamOutput(os); }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, JointInterface& self) -> decltype(is) { return self.StreamInput(is); }

private:
    MUSTARD_ALWAYS_INLINE constexpr auto Impl(auto& g) -> T;
    MUSTARD_ALWAYS_INLINE constexpr auto Impl(auto& g, const AParameter& p) -> T;

    template<muc::character AChar>
    auto StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os);
    template<muc::character AChar>
    auto StreamInput(std::basic_istream<AChar>& is) & -> decltype(is);
};

template<typename T, typename... Ds>
    requires(sizeof...(Ds) >= 2 and Concept::NumericVectorAny<T, sizeof...(Ds)> and (... and muc::arithmetic<typename Ds::ResultType>))
class Joint;

template<typename T, typename... Ds>
    requires(sizeof...(Ds) >= 2 and Concept::NumericVectorAny<T, sizeof...(Ds)> and (... and muc::arithmetic<typename Ds::ResultType>))
class JointParameter final : public JointParameterInterface<JointParameter<T, Ds...>,
                                                            Joint<T, Ds...>,
                                                            Ds...> {
    using JointParameterInterface<Ds...>::JointParameterInterface;
};

template<typename... Ps>
JointParameter(Ps...) -> JointParameter<std::array<std::common_type_t<typename Ps::DistributionType::ResultType...>, sizeof...(Ps)>, typename Ps::DistributionType...>;

template<typename T, typename... Ds>
    requires(sizeof...(Ds) >= 2 and Concept::NumericVectorAny<T, sizeof...(Ds)> and (... and muc::arithmetic<typename Ds::ResultType>))
class Joint final : public JointInterface<Joint<T, Ds...>,
                                          JointParameter<T, Ds...>,
                                          T,
                                          Ds...> {
    using JointInterface<T, Ds...>::JointInterface;
};

template<typename... Ps>
Joint(Ps...) -> Joint<std::array<std::common_type_t<typename Ps::DistributionType::ResultType...>, sizeof...(Ps)>, typename Ps::DistributionType...>;
template<typename... Ds>
Joint(JointParameter<Ds...>) -> Joint<std::array<std::common_type_t<typename Ds::ResultType...>, sizeof...(Ds)>, Ds...>;

} // namespace Mustard::Math::Random::inline Distribution

#include "Mustard/Math/Random/Distribution/Joint.inl"
