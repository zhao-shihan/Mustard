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

namespace Mustard::Data::inline Object {

template<Modelized M>
const gtl::flat_hash_map<std::string_view, gsl::index> Tuple<M>::fgDynIndexMap{
    []<gsl::index... Is>(gslx::index_sequence<Is...>) {
        gtl::flat_hash_map<std::string_view, gsl::index> indexMap;
        indexMap.reserve(sizeof...(Is));
        (..., indexMap.emplace(std::tuple_element_t<Is, typename Model::StdTuple>::Name().sv(), Is));
        return indexMap;
    }(gslx::make_index_sequence<Size()>{})};

template<Modelized M>
template<TupleLike ATuple>
constexpr auto Tuple<M>::operator==(const ATuple& that) const -> bool {
    if constexpr (not EquivalentTuple<Tuple, M>) {
        return false;
    }
    const auto fieldFieldCompare{[this, &that]<gsl::index I, gsl::index J>() constexpr {
        constexpr auto nameI{std::tuple_element_t<I, Tuple>::Name()};
        constexpr auto nameJ{std::tuple_element_t<J, ATuple>::Name()};
        if constexpr (nameI != nameJ) {
            return true; // skip non-matching fields
        } else {
            return this->template Get<nameI>() == that.template Get<nameJ>(); // ADL fails here; use member Get
        }
    }};
    const auto fieldTupleCompare{[&fieldFieldCompare]<gsl::index I>() constexpr {
        return [&]<gsl::index... Js>(gslx::index_sequence<Js...>) constexpr {
            return (... and fieldFieldCompare.template operator()<I, Js>());
        }(gslx::make_index_sequence<ATuple::Size()>{});
    }};
    return [&fieldTupleCompare]<gsl::index... Is>(gslx::index_sequence<Is...>) constexpr {
        return (... and fieldTupleCompare.template operator()<Is>());
    }(gslx::make_index_sequence<Size()>{});
}

template<Modelized M>
template<TupleLike ATuple>
constexpr auto Tuple<M>::AsImpl() const -> ATuple {
    ATuple tuple;
    [&]<gsl::index... Is>(gslx::index_sequence<Is...>) constexpr {
        (..., [&]<gsl::index... Js, gsl::index I>(gslx::index_sequence<Js...>, std::integral_constant<gsl::index, I>) constexpr {
            (..., [&]() constexpr {
                constexpr auto nameI{std::tuple_element_t<I, Tuple>::Name()};
                constexpr auto nameJ{std::tuple_element_t<Js, ATuple>::Name()};
                if constexpr (nameI == nameJ) {
                    tuple.template Get<nameJ>() = this->template Get<nameI>(); // ADL fails here; use member Get
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
        }                                                                                                  \
        return VisitImpl<L + 1, R>(i, std::forward<decltype(F)>(F));                                       \
    }

template<Modelized M>
template<gsl::index L, gsl::index R>
MUSTARD_ALWAYS_INLINE auto Tuple<M>::VisitImpl(gsl::index i, auto&& F) const -> void {
    MUSTARD_DATA_TUPLE_VISIT_IMPL(GetImpl<idx>())
}

template<Modelized M>
template<gsl::index L, gsl::index R>
MUSTARD_ALWAYS_INLINE auto Tuple<M>::VisitImpl(gsl::index i, auto&& F) -> void {
    MUSTARD_DATA_TUPLE_VISIT_IMPL(GetImpl<idx>())
}

#undef MUSTARD_DATA_TUPLE_VISIT_IMPL

template<Modelized M>
MUSTARD_ALWAYS_INLINE auto Tuple<M>::DynIndex(std::string_view name) -> gsl::index {
    try {
        return fgDynIndexMap.at(name);
    } catch (const std::out_of_range& e) {
        PrintError(fmt::format("No field named '{}'", name));
        throw e;
    }
}

} // namespace Mustard::Data::inline Object
