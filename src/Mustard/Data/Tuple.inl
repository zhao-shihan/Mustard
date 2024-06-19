// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
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

namespace Mustard::Data {

template<typename ADerived>
constexpr EnableGet<ADerived>::EnableGet() {
    static_assert(std::derived_from<ADerived, EnableGet>);
    static_assert(TupleLike<ADerived>);
}

template<TupleModelizable... Ts>
template<TupleLike ATuple>
constexpr auto Tuple<Ts...>::operator==(const ATuple& that) const -> auto {
    if constexpr (not EquivalentTuple<Tuple, ATuple>) { return false; }
    return [&]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        return (... and ([&]<gsl::index... Js, gsl::index I>(gslx::index_sequence<Js...>, std::integral_constant<gsl::index, I>) {
                    return (... or ([&] {
                                constexpr auto nameI{std::tuple_element_t<I, Tuple>::Name()};
                                constexpr auto nameJ{std::tuple_element_t<Js, ATuple>::Name()};
                                if constexpr (nameI != nameJ) {
                                    return false;
                                } else {
                                    return Get<nameI>() == that.template Get<nameJ>(that);
                                }
                            }()));
                }(gslx::make_index_sequence<ATuple::Size()>{}, std::integral_constant<gsl::index, Is>{})));
    }(gslx::make_index_sequence<Size()>{});
}

template<TupleModelizable... Ts>
template<TupleLike ATuple>
constexpr auto Tuple<Ts...>::AsImpl() const -> ATuple {
    ATuple tuple;
    [&]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        (..., [&]<gsl::index... Js, gsl::index I>(gslx::index_sequence<Js...>, std::integral_constant<gsl::index, I>) {
            (..., [&] {
                constexpr auto nameI{std::tuple_element_t<I, Tuple>::Name()};
                constexpr auto nameJ{std::tuple_element_t<Js, ATuple>::Name()};
                if constexpr (nameI == nameJ) { tuple.template Get<nameJ>() = Get<nameI>(); }
            }());
        }(gslx::make_index_sequence<ATuple::Size()>{}, std::integral_constant<gsl::index, Is>{}));
    }(gslx::make_index_sequence<Size()>{});
    return tuple;
}

} // namespace Mustard::Data
