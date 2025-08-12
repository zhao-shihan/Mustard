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

#pragma once

#include "Mustard/Data/TupleModel.h++"
#include "Mustard/Data/Value.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Utility/InlineMacro.h++"
#include "Mustard/gslx/index_sequence.h++"

#include "muc/ceta_string"
#include "muc/concepts"
#include "muc/utility"

#include "gsl/gsl"

#include "fmt/core.h"

#include <concepts>
#include <functional>
#include <numeric>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <utility>

namespace Mustard::Data {

template<typename T>
concept TupleLike = requires {
    typename T::Model;
    requires muc::instantiated_from<typename T::Model, TupleModel>;
    { T::Size() } -> std::same_as<std::size_t>;
    requires T::Size() >= 0;
    requires([]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        return (... and requires(T t) { t.template Get<std::tuple_element_t<Is, typename T::Model::StdTuple>::Name()>(); });
    }(gslx::make_index_sequence<T::Size()>{}));
    requires([]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        return (... and requires(const T t) { t.template Get<std::tuple_element_t<Is, typename T::Model::StdTuple>::Name()>(); });
    }(gslx::make_index_sequence<T::Size()>{}));
    requires T::Size() <= 1 or requires {
        requires([]<gsl::index... Is>(gslx::index_sequence<Is...>) {
            return (... and requires(T t) { t.template Get<std::tuple_element_t<Is, typename T::Model::StdTuple>::Name(),
                                                           std::tuple_element_t<Is + 1, typename T::Model::StdTuple>::Name()>(); });
        }(gslx::make_index_sequence<T::Size() - 1>{}));
        requires([]<gsl::index... Is>(gslx::index_sequence<Is...>) {
            return (... and requires(const T t) { t.template Get<std::tuple_element_t<Is, typename T::Model::StdTuple>::Name(),
                                                                 std::tuple_element_t<Is + 1, typename T::Model::StdTuple>::Name()>(); });
        }(gslx::make_index_sequence<T::Size() - 1>{}));
    };
};

template<typename T1, typename T2>
concept EquivalentTuple = TupleLike<T1> and TupleLike<T2> and
                          EquivalentTupleModel<typename T1::Model, typename T2::Model>;

template<typename T1, typename T2>
concept SubTuple = TupleLike<T1> and TupleLike<T2> and
                   SubTupleModel<typename T1::Model, typename T2::Model>;

template<typename T1, typename T2>
concept SuperTuple = TupleLike<T1> and TupleLike<T2> and
                     SuperTupleModel<typename T1::Model, typename T2::Model>;

template<typename T1, typename T2>
concept ProperSubTuple = TupleLike<T1> and TupleLike<T2> and
                         ProperSubTupleModel<typename T1::Model, typename T2::Model>;

template<typename T1, typename T2>
concept ProperSuperTuple = TupleLike<T1> and TupleLike<T2> and
                           ProperSuperTupleModel<typename T1::Model, typename T2::Model>;

} // namespace Mustard::Data

namespace std {

template<typename T>
    requires requires { typename T::Model; } and
             muc::instantiated_from<typename T::Model, Mustard::Data::TupleModel>
struct tuple_size<T>
    : tuple_size<typename T::Model::StdTuple> {};

template<std::size_t I, typename T>
    requires requires { typename T::Model; } and
             muc::instantiated_from<typename T::Model, Mustard::Data::TupleModel>
struct tuple_element<I, T>
    : tuple_element<I, typename T::Model::StdTuple> {};

} // namespace std

namespace Mustard::Data {

namespace internal {

/// @brief Helper class for `Tuple` friend `Get` function.
template<typename ADerived>
class EnableGet {
    template<muc::ceta_string... ANames>
        requires(sizeof...(ANames) >= 1)
    friend constexpr auto Get(const ADerived& t) -> decltype(auto) { return t.template Get<ANames...>(); }
    template<muc::ceta_string... ANames>
        requires(sizeof...(ANames) >= 1)
    friend constexpr auto Get(ADerived& t) -> decltype(auto) { return t.template Get<ANames...>(); }
    template<muc::ceta_string... ANames>
        requires(sizeof...(ANames) >= 1)
    friend constexpr auto Get(ADerived&& t) -> decltype(auto) { return std::move(t).template Get<ANames...>(); }
    template<muc::ceta_string... ANames>
        requires(sizeof...(ANames) >= 1)
    friend constexpr auto Get(const ADerived&& t) -> decltype(auto) { return std::move(t).template Get<ANames...>(); }

    template<muc::ceta_string AName, typename T>
    friend constexpr auto GetAs(const ADerived& t) -> decltype(auto) { return t.template Get<AName>().template As<T>(); }
    template<muc::ceta_string AName, typename T>
    friend constexpr auto GetAs(ADerived&& t) -> decltype(auto) { return std::move(t).template Get<AName>().template As<T>(); }

    template<gsl::index I>
    friend constexpr auto get(const ADerived& t) -> decltype(auto) { return t.template Get<std::tuple_element_t<I, ADerived>::Name()>(); }
    template<gsl::index I>
    friend constexpr auto get(ADerived& t) -> decltype(auto) { return t.template Get<std::tuple_element_t<I, ADerived>::Name()>(); }
    template<gsl::index I>
    friend constexpr auto get(ADerived&& t) -> decltype(auto) { return std::move(t).template Get<std::tuple_element_t<I, ADerived>::Name()>(); }
    template<gsl::index I>
    friend constexpr auto get(const ADerived&& t) -> decltype(auto) { return std::move(t).template Get<std::tuple_element_t<I, ADerived>::Name()>(); }

protected:
    constexpr EnableGet();
    constexpr ~EnableGet() = default;
};

} // namespace internal

/// @brief Data model defined tuple.
template<TupleModelizable... Ts>
class Tuple : public internal::EnableGet<Tuple<Ts...>> {
public:
    using Model = TupleModel<Ts...>;

private:
    struct Dummy {
        using Type = Dummy;
    };

public:
    constexpr Tuple() = default;

    template<TupleModelizable... Us>
        requires std::constructible_from<typename Model::StdTuple, const typename Tuple<Us...>::Model::StdTuple&>
    constexpr Tuple(const Tuple<Us...>& tuple) :
        fTuple{tuple.fTuple} {}

    template<TupleModelizable... Us>
        requires std::constructible_from<typename Model::StdTuple, typename Tuple<Us...>::Model::StdTuple&&>
    constexpr Tuple(Tuple<Us...>&& tuple) :
        fTuple{std::move(tuple.fTuple)} {}

    template<typename... Us>
        requires std::constructible_from<typename Model::StdTuple, Us&&...>
    constexpr explicit(sizeof...(Us) == 1) Tuple(Us&&... values) :
        fTuple{std::forward<Us>(values)...} {}

    template<muc::ceta_string... ANames>
        requires(sizeof...(ANames) >= 1)
    constexpr auto Get() const& -> decltype(auto) { return GetImpl<Model::template Index<ANames>()...>(); }
    template<muc::ceta_string... ANames>
        requires(sizeof...(ANames) >= 1)
    constexpr auto Get() & -> decltype(auto) { return GetImpl<Model::template Index<ANames>()...>(); }
    template<muc::ceta_string... ANames>
        requires(sizeof...(ANames) >= 1)
    constexpr auto Get() && -> decltype(auto) { return std::move(*this).template GetImpl<Model::template Index<ANames>()...>(); }
    template<muc::ceta_string... ANames>
        requires(sizeof...(ANames) >= 1)
    constexpr auto Get() const&& -> decltype(auto) { return std::move(*this).template GetImpl<Model::template Index<ANames>()...>(); }

    template<SubTuple<Tuple<Ts...>> ATuple>
    constexpr auto As() const -> auto { return AsImpl<ATuple>(); }
    template<std::same_as<Tuple<Ts...>> ATuple>
    constexpr auto As() const -> const auto& { return *this; }

    template<TupleLike ATuple>
    constexpr auto operator==(const ATuple& that) const -> auto;
    template<TupleLike ATuple>
    constexpr auto operator<=>(const ATuple&) const -> auto = delete;

    auto Visit(std::string_view name, auto&& F) const& -> void { VisitImpl<0, Size() - 1>(DynIndex(name), std::forward<decltype(F)>(F)); }
    auto Visit(std::string_view name, auto&& F) & -> void { VisitImpl<0, Size() - 1>(DynIndex(name), std::forward<decltype(F)>(F)); }
    auto Visit(std::string_view name, auto&& F) && -> void { VisitImpl<0, Size() - 1>(DynIndex(name), std::forward<decltype(F)>(F)); }
    auto Visit(std::string_view name, auto&& F) const&& -> void { VisitImpl<0, Size() - 1>(DynIndex(name), std::forward<decltype(F)>(F)); }

    static constexpr auto Size() -> auto { return Model::Size(); }
    static constexpr auto NameVector() -> auto { return Model::NameVector(); }

private:
    template<gsl::index I>
        requires(0 <= I and I < Model::Size())
    constexpr auto GetImpl() const& -> const std::tuple_element_t<I, Tuple>& { return std::get<I>(fTuple); }
    template<gsl::index I>
        requires(0 <= I and I < Model::Size())
    constexpr auto GetImpl() & -> std::tuple_element_t<I, Tuple>& { return std::get<I>(fTuple); }
    template<gsl::index I>
        requires(0 <= I and I < Model::Size())
    constexpr auto GetImpl() && -> std::tuple_element_t<I, Tuple>&& { return std::get<I>(std::move(fTuple)); }
    template<gsl::index I>
        requires(0 <= I and I < Model::Size())
    constexpr auto GetImpl() const&& -> const std::tuple_element_t<I, Tuple>&& { return std::get<I>(std::move(fTuple)); }

    template<gsl::index... Is>
        requires((sizeof...(Is) >= 2) and ... and (0 <= Is and Is < Model::Size()))
    constexpr auto GetImpl() const& -> auto { return Tuple<std::tuple_element_t<Is, Tuple>...>{std::get<Is>(fTuple)...}; }
    template<gsl::index... Is>
        requires((sizeof...(Is) >= 2) and ... and (0 <= Is and Is < Model::Size()))
    constexpr auto GetImpl() && -> auto { return Tuple<std::tuple_element_t<Is, Tuple>...>{std::get<Is>(std::move(fTuple))...}; }
    template<gsl::index... Is>
        requires((sizeof...(Is) >= 2) and ... and (0 <= Is and Is < Model::Size()))
    constexpr auto GetImpl() const&& -> auto { return Tuple<std::tuple_element_t<Is, Tuple>...>{std::get<Is>(std::move(fTuple))...}; }

    template<TupleLike ATuple>
    constexpr auto AsImpl() const -> ATuple;

    template<gsl::index L, gsl::index R>
    MUSTARD_ALWAYS_INLINE auto VisitImpl(gsl::index i, auto&& F) const& -> void;
    template<gsl::index L, gsl::index R>
    MUSTARD_ALWAYS_INLINE auto VisitImpl(gsl::index i, auto&& F) & -> void;
    template<gsl::index L, gsl::index R>
    MUSTARD_ALWAYS_INLINE auto VisitImpl(gsl::index i, auto&& F) && -> void;
    template<gsl::index L, gsl::index R>
    MUSTARD_ALWAYS_INLINE auto VisitImpl(gsl::index i, auto&& F) const&& -> void;

    static auto DynIndex(std::string_view name) -> gsl::index;

private:
    typename Model::StdTuple fTuple;
};

template<typename... Ts>
Tuple(Ts...) -> Tuple<Ts...>;

} // namespace Mustard::Data

#include "Mustard/Data/Tuple.inl"
