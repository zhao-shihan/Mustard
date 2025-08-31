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

namespace Mustard::Math::Random::inline Distribution {

namespace internal {

template<muc::arithmetic T, template<typename> typename AUniform>
constexpr BasicUniformParameter<T, AUniform>::BasicUniformParameter() :
    BasicUniformParameter{0,
                          std::integral<T> ?
                              std::numeric_limits<T>::max() :
                              1} {}

template<muc::arithmetic T, template<typename> typename AUniform>
constexpr BasicUniformParameter<T, AUniform>::BasicUniformParameter(T inf, T sup) :
    DistributionParameterBase<BasicUniformParameter<T, AUniform>, AUniform<T>>(),
    fInfimum{inf},
    fSupremum{sup} {}

template<muc::arithmetic T, template<typename> typename AUniform>
template<muc::character AChar>
auto BasicUniformParameter<T, AUniform>::StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os) {
    if constexpr (std::floating_point<T>) {
        const auto oldPrecision{os.precision(std::numeric_limits<T>::max_digits10)};
        return os << fInfimum << ' ' << fSupremum
                  << std::setprecision(oldPrecision);
    } else {
        return os << fInfimum << ' ' << fSupremum;
    }
}

template<muc::arithmetic T, template<typename> typename AUniform>
template<muc::character AChar>
auto BasicUniformParameter<T, AUniform>::StreamInput(std::basic_istream<AChar>& is) & -> decltype(is) {
    return is >> fInfimum >> fSupremum;
}

template<template<typename> typename ADerived, muc::arithmetic T>
constexpr UniformBase<ADerived, T>::UniformBase(T inf, T sup) :
    Base{},
    fParameter{inf, sup} {}

template<template<typename> typename ADerived, muc::arithmetic T>
constexpr UniformBase<ADerived, T>::UniformBase(const typename Base::ParameterType& p) :
    Base{},
    fParameter{p} {}

} // namespace internal

template<std::floating_point T>
MUSTARD_ALWAYS_INLINE constexpr auto UniformCompact<T>::operator()(UniformRandomBitGenerator auto& g, const UniformCompactParameter<T>& p) -> T {
    const auto u{static_cast<T>(g() - g.Min()) / (g.Max() - g.Min())};
    muc::assume(0 <= u and u <= 1);
    return p.Infimum() + u * (p.Supremum() - p.Infimum());
}

template<std::floating_point T>
MUSTARD_ALWAYS_INLINE auto UniformCompact<T>::operator()(CLHEP::HepRandomEngine& g, const UniformCompactParameter<T>& p) -> T {
    return p.Infimum() + g.flat() * (p.Supremum() - p.Infimum());
}

template<std::floating_point T>
MUSTARD_ALWAYS_INLINE constexpr auto UniformReal<T>::Impl(auto& g, const UniformParameter<T>& p) -> T {
    static_assert(UniformCompact<T>::Stateless());
    auto Uni{UniformCompact<T>{}};
    auto u{Uni(g)};
    muc::assume(0 <= u and u <= 1);
    while (u == 0 or u == 1) [[unlikely]] {
        u = Uni(g);
        muc::assume(0 <= u and u <= 1);
    }
    return p.Infimum() + u * (p.Supremum() - p.Infimum());
}

template<std::integral T>
MUSTARD_ALWAYS_INLINE constexpr auto UniformInteger<T>::operator()(UniformRandomBitGenerator auto& g, const UniformParameter<T>& p) -> T {
    // Do we need an independent implementation?
    return std::uniform_int_distribution<T>{p.Infimum(), p.Supremum()}(g);
}

} // namespace Mustard::Math::Random::inline Distribution
