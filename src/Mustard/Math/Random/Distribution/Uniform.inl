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

namespace Mustard::inline Math::Random::inline Distribution {

namespace impl {

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

} // namespace impl

template<std::floating_point T>
template<std::uniform_random_bit_generator G>
MUSTARD_ALWAYS_INLINE constexpr auto UniformReal<T>::operator()(G& g, const UniformParameter<T>& p) -> T {
    constexpr auto fromU64{sizeof(typename G::result_type) == sizeof(std::uint64_t)};
    constexpr auto fromU32{sizeof(typename G::result_type) == sizeof(std::uint32_t)};
    constexpr auto toIEEE754Double{std::numeric_limits<T>::is_iec559 and std::same_as<T, double>};
    constexpr auto toIEEE754Float{std::numeric_limits<T>::is_iec559 and std::same_as<T, float>};
    T u; // uniform on [0,1)
    if constexpr (fromU64 and toIEEE754Double) {
        u = (g() >> 11) * 0x1.p-53;
    } else if constexpr (fromU32 and toIEEE754Float) {
        u = (g() >> 8) * 0x1.p-24f;
    } else if constexpr (fromU64 and toIEEE754Float) {
        u = (g() >> 40) * 0x1.p-24f;
    } else if constexpr (fromU32 and toIEEE754Double) {
        u = g() * 0x1.p-32;
    } else { // the above should cover all the common cases
        u = std::uniform_real_distribution<T>{0, 1}(g);
    }
    muc::assume(0 <= u and u < 1);
    return muc::lerp(p.Infimum(), p.Supremum(), u);
}

template<std::floating_point T>
MUSTARD_ALWAYS_INLINE auto UniformReal<T>::operator()(CLHEP::HepRandomEngine& g, const UniformParameter<T>& p) -> T {
    return muc::lerp(p.Infimum(), p.Supremum(), g.flat());
}

template<std::floating_point T>
MUSTARD_ALWAYS_INLINE auto UniformReal<T>::operator()(TRandom& g, const UniformParameter<T>& p) -> T {
    return muc::lerp(p.Infimum(), p.Supremum(), g.Rndm());
}

template<std::integral T>
template<std::uniform_random_bit_generator G>
MUSTARD_ALWAYS_INLINE constexpr auto UniformInteger<T>::operator()(G& g, const UniformParameter<T>& p) -> T {
    // Do we need an independent implementation?
    return std::uniform_int_distribution<T>{p.Infimum(), p.Supremum()}(g);
}

} // namespace Mustard::inline Math::Random::inline Distribution
