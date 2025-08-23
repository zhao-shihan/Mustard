// -*- C++ -*-
//
// Copyright 2020-2025  The Mustard development team
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

namespace Mustard::Math::Random::inline Distribution {

namespace internal {

template<typename... Ts>
CartesianProductMargin<Ts...>::CartesianProductMargin(const Ts&... objects) :
    internal::CartesianProductMarginBase<gslx::index_sequence_for<Ts...>, Ts...>{{objects}...} {}

} // namespace internal

template<typename ADerived, typename ADistribution, typename... Ds>
    requires(sizeof...(Ds) >= 2)
constexpr JointParameterInterface<ADerived, ADistribution, Ds...>::JointParameterInterface() :
    DistributionParameterBase<ADerived, ADistribution>{},
    internal::CartesianProductMargin<typename Ds::ParameterType...>{} {
    static_assert(std::derived_from<ADerived, JointParameterInterface<ADerived, ADistribution, Ds...>>);
}

template<typename ADerived, typename ADistribution, typename... Ds>
    requires(sizeof...(Ds) >= 2)
constexpr JointParameterInterface<ADerived, ADistribution, Ds...>::JointParameterInterface(const typename Ds::ParameterType&... p) :
    DistributionParameterBase<ADerived, ADistribution>{},
    internal::CartesianProductMargin<typename Ds::ParameterType...>{p...} {}

template<typename ADerived, typename ADistribution, typename... Ds>
    requires(sizeof...(Ds) >= 2)
template<muc::character AChar>
auto JointParameterInterface<ADerived, ADistribution, Ds...>::StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os) {
    return ([&]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        return (os << ... << this->template Margin<Is>());
    })(gslx::index_sequence_for<Ds...>());
}

template<typename ADerived, typename ADistribution, typename... Ds>
    requires(sizeof...(Ds) >= 2)
template<muc::character AChar>
auto JointParameterInterface<ADerived, ADistribution, Ds...>::StreamInput(std::basic_istream<AChar>& is) & -> decltype(is) {
    return ([&]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        return (is >> ... >> this->template Margin<Is>());
    })(gslx::index_sequence_for<Ds...>());
}

template<typename ADerived, typename AParameter, typename T, typename... Ds>
    requires(sizeof...(Ds) >= 2 and Concept::NumericVectorAny<T, sizeof...(Ds)>)
constexpr JointInterface<ADerived, AParameter, T, Ds...>::JointInterface() :
    RandomNumberDistributionBase<ADerived, AParameter, T>{},
    internal::CartesianProductMargin<Ds...>{} {
    static_assert(std::derived_from<ADerived, JointInterface<ADerived, AParameter, T, Ds...>>);
    static_assert(std::derived_from<AParameter, JointParameterInterface<AParameter, ADerived, Ds...>>);
}

template<typename ADerived, typename AParameter, typename T, typename... Ds>
    requires(sizeof...(Ds) >= 2 and Concept::NumericVectorAny<T, sizeof...(Ds)>)
constexpr JointInterface<ADerived, AParameter, T, Ds...>::JointInterface(const typename Ds::ParameterType&... p) :
    RandomNumberDistributionBase<ADerived, AParameter, T>{},
    internal::CartesianProductMargin<Ds...>{Ds{p}...} {}

template<typename ADerived, typename AParameter, typename T, typename... Ds>
    requires(sizeof...(Ds) >= 2 and Concept::NumericVectorAny<T, sizeof...(Ds)>)
constexpr JointInterface<ADerived, AParameter, T, Ds...>::JointInterface(const AParameter& p) :
    RandomNumberDistributionBase<ADerived, AParameter, T>{},
    internal::CartesianProductMargin<Ds...>{std::make_from_tuple<internal::CartesianProductMargin<Ds...>>(p)} {}

template<typename ADerived, typename AParameter, typename T, typename... Ds>
    requires(sizeof...(Ds) >= 2 and Concept::NumericVectorAny<T, sizeof...(Ds)>)
MUSTARD_STRONG_INLINE constexpr auto JointInterface<ADerived, AParameter, T, Ds...>::operator()(UniformRandomBitGenerator auto& g) -> T {
    return [this, &g]<gsl::index... Is>(gslx::index_sequence<Is...>) -> T {
        return {this->template Margin<Is>()(g)...};
    }(gslx::index_sequence_for<Ds...>());
}

template<typename ADerived, typename AParameter, typename T, typename... Ds>
    requires(sizeof...(Ds) >= 2 and Concept::NumericVectorAny<T, sizeof...(Ds)>)
MUSTARD_STRONG_INLINE constexpr auto JointInterface<ADerived, AParameter, T, Ds...>::operator()(UniformRandomBitGenerator auto& g, const AParameter& p) -> T {
    return [this, &g, &p]<gsl::index... Is>(gslx::index_sequence<Is...>) -> T {
        return {this->template Margin<Is>()(g, p.template Parameter<Is>())...};
    }(gslx::index_sequence_for<Ds...>());
}

template<typename ADerived, typename AParameter, typename T, typename... Ds>
    requires(sizeof...(Ds) >= 2 and Concept::NumericVectorAny<T, sizeof...(Ds)>)
constexpr auto JointInterface<ADerived, AParameter, T, Ds...>::Reset() -> void {
    [this]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        (..., this->template Margin<Is>().Reset());
    }(gslx::index_sequence_for<Ds...>());
}

template<typename ADerived, typename AParameter, typename T, typename... Ds>
    requires(sizeof...(Ds) >= 2 and Concept::NumericVectorAny<T, sizeof...(Ds)>)
constexpr auto JointInterface<ADerived, AParameter, T, Ds...>::Parameter() const -> AParameter {
    return [this]<gsl::index... Is>(gslx::index_sequence<Is...>) -> AParameter {
        return {this->Parameter<Is>()...};
    }(gslx::index_sequence_for<Ds...>());
}

template<typename ADerived, typename AParameter, typename T, typename... Ds>
    requires(sizeof...(Ds) >= 2 and Concept::NumericVectorAny<T, sizeof...(Ds)>)
constexpr auto JointInterface<ADerived, AParameter, T, Ds...>::Parameter(const AParameter& p) -> void {
    [this, &p]<gsl::index... Is>(std::index_sequence<Is...>) {
        (..., this->Parameter<Is>(p.template Parameter<Is>()));
    }(gslx::index_sequence_for<Ds...>());
}

template<typename ADerived, typename AParameter, typename T, typename... Ds>
    requires(sizeof...(Ds) >= 2 and Concept::NumericVectorAny<T, sizeof...(Ds)>)
constexpr auto JointInterface<ADerived, AParameter, T, Ds...>::Min() const -> T {
    return [this]<gsl::index... Is>(gslx::index_sequence<Is...>) -> T {
        return {this->template Margin<Is>().Min()...};
    }(gslx::index_sequence_for<Ds...>());
}

template<typename ADerived, typename AParameter, typename T, typename... Ds>
    requires(sizeof...(Ds) >= 2 and Concept::NumericVectorAny<T, sizeof...(Ds)>)
constexpr auto JointInterface<ADerived, AParameter, T, Ds...>::Max() const -> T {
    return [this]<gsl::index... Is>(gslx::index_sequence<Is...>) -> T {
        return {this->template Margin<Is>().Max()...};
    }(gslx::index_sequence_for<Ds...>());
}

template<typename ADerived, typename AParameter, typename T, typename... Ds>
    requires(sizeof...(Ds) >= 2 and Concept::NumericVectorAny<T, sizeof...(Ds)>)
template<muc::character AChar>
auto JointInterface<ADerived, AParameter, T, Ds...>::StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os) {
    return [&]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        return (os << ... << this->template Margin<Is>());
    }(gslx::index_sequence_for<Ds...>());
}

template<typename ADerived, typename AParameter, typename T, typename... Ds>
    requires(sizeof...(Ds) >= 2 and Concept::NumericVectorAny<T, sizeof...(Ds)>)
template<muc::character AChar>
auto JointInterface<ADerived, AParameter, T, Ds...>::StreamInput(std::basic_istream<AChar>& is) & -> decltype(is) {
    return [&]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        return (is >> ... >> this->template Margin<Is>());
    }(gslx::index_sequence_for<Ds...>());
}

} // namespace Mustard::Math::Random::inline Distribution
