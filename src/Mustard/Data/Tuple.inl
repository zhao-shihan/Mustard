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

namespace Mustard::Data {

namespace internal {

template<typename ADerived>
constexpr EnableGet<ADerived>::EnableGet() {
    static_assert(std::derived_from<ADerived, EnableGet>);
    static_assert(TupleLike<ADerived>);
}

} // namespace internal

template<TupleModelizable... Ts>
template<TupleLike ATuple>
constexpr auto Tuple<Ts...>::operator==(const ATuple& that) const -> auto {
    if constexpr (not EquivalentTuple<Tuple, ATuple>) {
        return false;
    }
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
                if constexpr (nameI == nameJ) {
                    tuple.template Get<nameJ>() = Get<nameI>();
                }
            }());
        }(gslx::make_index_sequence<ATuple::Size()>{}, std::integral_constant<gsl::index, Is>{}));
    }(gslx::make_index_sequence<Size()>{});
    return tuple;
}

// not good for small data model
/* #define MUSTARD_DATA_TUPLE_VISIT_IMPL(GetImpl)                                                             \
    if constexpr (L > R) {                                                                                 \
        Throw<std::out_of_range>(fmt::format("Index {} out of range", i));                                 \
    } else {                                                                                               \
        constexpr auto idx{std::midpoint(L, R)};                                                           \
        if (i == idx) {                                                                                    \
            if constexpr (requires { std::invoke(std::forward<decltype(F)>(F), *GetImpl); }) {             \
                std::invoke(std::forward<decltype(F)>(F), *GetImpl);                                       \
                return;                                                                                    \
            } else {                                                                                       \
                Throw<std::invalid_argument>(fmt::format("The function provided is not invocable with {}", \
                                                         muc::try_demangle(typeid(*GetImpl).name())));     \
            }                                                                                              \
        } else if (i < idx) {                                                                              \
            return VisitImpl<L, idx - 1>(i, std::forward<decltype(F)>(F));                                 \
        } else {                                                                                           \
            return VisitImpl<idx + 1, R>(i, std::forward<decltype(F)>(F));                                 \
        }                                                                                                  \
    } */

// better, seems to be optimized as a jump table
#define MUSTARD_DATA_TUPLE_VISIT_IMPL(GetImpl)                                                             \
    if constexpr (L > R) {                                                                                 \
        Throw<std::out_of_range>(fmt::format("Index {} out of range", i));                                 \
    } else {                                                                                               \
        if (i == L) {                                                                                      \
            constexpr auto idx{L};                                                                         \
            if constexpr (requires { std::invoke(std::forward<decltype(F)>(F), *GetImpl); }) {             \
                std::invoke(std::forward<decltype(F)>(F), *GetImpl);                                       \
                return;                                                                                    \
            } else {                                                                                       \
                Throw<std::invalid_argument>(fmt::format("The function provided is not invocable with {}", \
                                                         muc::try_demangle(typeid(*GetImpl).name())));     \
            }                                                                                              \
        } else {                                                                                           \
            return VisitImpl<L + 1, R>(i, std::forward<decltype(F)>(F));                                   \
        }                                                                                                  \
    }

template<TupleModelizable... Ts>
template<gsl::index L, gsl::index R>
MUSTARD_ALWAYS_INLINE auto Tuple<Ts...>::VisitImpl(gsl::index i, auto&& F) const& -> void {
    MUSTARD_DATA_TUPLE_VISIT_IMPL(GetImpl<idx>())
}

template<TupleModelizable... Ts>
template<gsl::index L, gsl::index R>
MUSTARD_ALWAYS_INLINE auto Tuple<Ts...>::VisitImpl(gsl::index i, auto&& F) & -> void {
    MUSTARD_DATA_TUPLE_VISIT_IMPL(GetImpl<idx>())
}

template<TupleModelizable... Ts>
template<gsl::index L, gsl::index R>
MUSTARD_ALWAYS_INLINE auto Tuple<Ts...>::VisitImpl(gsl::index i, auto&& F) && -> void {
    MUSTARD_DATA_TUPLE_VISIT_IMPL(std::move(GetImpl<idx>()))
}

template<TupleModelizable... Ts>
template<gsl::index L, gsl::index R>
MUSTARD_ALWAYS_INLINE auto Tuple<Ts...>::VisitImpl(gsl::index i, auto&& F) const&& -> void {
    MUSTARD_DATA_TUPLE_VISIT_IMPL(std::move(GetImpl<idx>()))
}

#undef MUSTARD_DATA_TUPLE_VISIT_IMPL

template<TupleModelizable... Ts>
auto Tuple<Ts...>::DynIndex(std::string_view name) -> gsl::index {
    static const auto index{
        []<gsl::index... Is>(gslx::index_sequence<Is...>) {
            muc::flat_hash_map<std::string_view, gsl::index> indexMap;
            indexMap.reserve(sizeof...(Is));
            (indexMap.emplace(std::tuple_element_t<Is, typename Model::StdTuple>::Name().sv(), Is), ...);
            return indexMap;
        }(gslx::make_index_sequence<Size()>{})};
    try {
        return index.at(name);
    } catch (const std::out_of_range& e) {
        PrintError(fmt::format("No field named '{}'", name));
        throw e;
    }
}

} // namespace Mustard::Data
