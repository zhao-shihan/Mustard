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

#include "Mustard/Data/Model.h++"
#include "Mustard/Data/Object/Value.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Memory/Arc.h++"
#include "Mustard/Utility/FunctionAttribute.h++"
#include "Mustard/gslx/index_sequence.h++"

#include "gtl/phmap.hpp"

#include "muc/ceta_string"
#include "muc/concepts"
#include "muc/tuple"
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
#include <utility>

namespace Mustard::Data::inline Object {

/// @brief Concept satisfied by Tuple and Tuple-compatible types.
template<typename T>
concept TupleLike = requires {
    typename T::Model;
    requires Modelized<typename T::Model>;
    requires muc::tuple_like<T>;
    requires muc::tuple_like<typename T::Model::StdTuple>;
    { T::Size() } -> std::same_as<std::size_t>;
    requires T::Size() >= 0;
    requires([]<gsl::index... Is>(gslx::index_sequence<Is...>) consteval {
        return (... and requires(T t) { t.template F<std::tuple_element_t<Is, typename T::Model::StdTuple>::Name()>(); });
    }(gslx::make_index_sequence<T::Size()>{}));
    requires([]<gsl::index... Is>(gslx::index_sequence<Is...>) consteval {
        return (... and requires(const T t) { t.template F<std::tuple_element_t<Is, typename T::Model::StdTuple>::Name()>(); });
    }(gslx::make_index_sequence<T::Size()>{}));
};

/// @brief Concept for tuples whose model is equivalent to a target tuple model.
template<typename T, typename M>
concept EquivalentTuple = TupleLike<T> and EquivalentModel<typename T::Model, M>;

/// @brief Concept for tuples whose model is a sub-model of a target tuple model.
template<typename T, typename M>
concept SubTuple = TupleLike<T> and SubModel<typename T::Model, M>;

/// @brief Concept for tuples whose model is a super-model of a target tuple model.
template<typename T, typename M>
concept SuperTuple = TupleLike<T> and SuperModel<typename T::Model, M>;

/// @brief Concept for tuples whose model is a strict sub-model of a target tuple model.
template<typename T, typename M>
concept ProperSubTuple = TupleLike<T> and ProperSubModel<typename T::Model, M>;

/// @brief Concept for tuples whose model is a strict super-model of a target tuple model.
template<typename T, typename M>
concept ProperSuperTuple = TupleLike<T> and ProperSuperModel<typename T::Model, M>;

/// @brief Concept for tuples whose model covers another tuple model (super-model or equivalent model).
/// This is essentially an alias for SuperTuple, provided for better readability in some interfaces.
template<typename T, typename M>
concept TupleCoverable = SuperTuple<T, M>;

/// @brief Concept satisfied by automatically reference-counted tuple types.
/// i.e. ArcTuple and ArcTuple-compatible types.
template<typename A>
concept ArcTupleLike = requires(A arc) {
    typename A::element_type;
    requires TupleLike<typename A::element_type>;
    requires std::regular<A>;
    requires muc::boolean_testable<A>;
} and requires(A arc, const A carc, typename A::element_type* ptr) {
    requires std::constructible_from<A, decltype(ptr)>;
    requires std::assignable_from<A, decltype(ptr)>;
    { arc.reset() };
    { arc.reset(ptr) };
    { carc.get() } -> std::same_as<decltype(ptr)>;
    { arc.detach() } -> std::same_as<decltype(ptr)>;
    { *arc } -> std::same_as<decltype(*ptr)>;
    { arc.operator->() } -> std::same_as<decltype(ptr)>;
};

/// @brief Concept for arc tuples whose model is equivalent to a target tuple model.
template<typename T, typename M>
concept EquivalentArcTuple = ArcTupleLike<T> and EquivalentModel<typename T::element_type::Model, M>;

/// @brief Concept for arc tuples whose model is a sub-model of a target tuple model.
template<typename T, typename M>
concept SubArcTuple = ArcTupleLike<T> and SubModel<typename T::element_type::Model, M>;

/// @brief Concept for arc tuples whose model is a super-model of a target tuple model.
template<typename T, typename M>
concept SuperArcTuple = ArcTupleLike<T> and SuperModel<typename T::element_type::Model, M>;

/// @brief Concept for arc tuples whose model is a strict sub-model of a target tuple model.
template<typename T, typename M>
concept ProperSubArcTuple = ArcTupleLike<T> and ProperSubModel<typename T::element_type::Model, M>;

/// @brief Concept for arc tuples whose model is a strict super-model of a target tuple model.
template<typename T, typename M>
concept ProperSuperArcTuple = ArcTupleLike<T> and ProperSuperModel<typename T::element_type::Model, M>;

/// @brief Concept for arc tuples whose model covers another tuple model (super-model or equivalent model).
/// This is essentially an alias for SuperArcTuple, provided for better readability in some interfaces.
template<typename T, typename M>
concept ArcTupleCoverable = SuperArcTuple<T, M>;

} // namespace Mustard::Data::inline Object

namespace std {

/// @brief Enables tuple protocol support for Mustard tuple-like types.
template<typename T>
    requires requires { typename T::Model; } and
             Mustard::Data::Modelized<typename T::Model>
struct tuple_size<T>
    : tuple_size<typename T::Model::StdTuple> {};

/// @brief Exposes field type by index for Mustard tuple-like types.
template<std::size_t I, typename T>
    requires requires { typename T::Model; } and
             Mustard::Data::Modelized<typename T::Model>
struct tuple_element<I, T>
    : tuple_element<I, typename T::Model::StdTuple> {};

} // namespace std

namespace Mustard::Data::inline Object {

/// @brief A tuple of values, with named fields and a static schema defined by a Model. The main data structure in Mustard.
/// @tparam M Tuple model.
template<Modelized M>
class Tuple {
public:
    /// @brief Compile-time schema type carried by this tuple.
    using Model = M;

public:
    /// @brief Default-constructs all fields.
    constexpr Tuple() = default;

    /// @brief Constructs from another tuple when their underlying standard tuples are constructible.
    /// @tparam U Source tuple model.
    /// @param tuple Source tuple.
    template<Modelized U>
        requires std::constructible_from<typename M::StdTuple, const typename Tuple<U>::M::StdTuple&>
    constexpr Tuple(const Tuple<U>& tuple) noexcept(std::is_nothrow_constructible_v<typename M::StdTuple, const typename Tuple<U>::M::StdTuple&>) :
        fTuple{tuple.fTuple} {}

    /// @brief Move-constructs from another tuple when their underlying standard tuples are constructible.
    /// @tparam U Source tuple model.
    /// @param tuple Source tuple.
    template<Modelized U>
        requires std::constructible_from<typename M::StdTuple, typename Tuple<U>::M::StdTuple&&>
    constexpr Tuple(Tuple<U>&& tuple) noexcept(std::is_nothrow_constructible_v<typename M::StdTuple, typename Tuple<U>::M::StdTuple&&>) :
        fTuple{std::move(tuple.fTuple)} {}

    /// @brief Constructs from forwarded values matching the underlying schema tuple constructor.
    /// @tparam ...Us Value argument types.
    /// @param values Constructor arguments for the underlying storage tuple.
    template<typename... Us>
        requires std::constructible_from<typename M::StdTuple, Us&&...>
    constexpr explicit(sizeof...(Us) == 1) Tuple(Us&&... values) noexcept(std::is_nothrow_constructible_v<typename M::StdTuple, Us&&...>) :
        fTuple{std::forward<Us>(values)...} {}

    /// @brief Gets a field by name.
    /// @tparam AName The compile-time field name.
    /// @return Returns a field reference preserving value category.
    template<muc::ceta_string AName>
    constexpr auto F() const& -> decltype(auto) { return GetImpl<M::template Index<AName>()>(); }
    template<muc::ceta_string AName>
    constexpr auto F() & -> decltype(auto) { return GetImpl<M::template Index<AName>()>(); }
    template<muc::ceta_string AName>
    constexpr auto F() && -> decltype(auto) { return std::move(*this).template GetImpl<M::template Index<AName>()>(); }
    template<muc::ceta_string AName>
    constexpr auto F() const&& -> decltype(auto) { return std::move(*this).template GetImpl<M::template Index<AName>()>(); }

    /// @brief Gets a field by name and converts it to a different type via Value::As.
    /// @tparam U The target type.
    /// @tparam AName The compile-time field name.
    /// @return Converted value of the field.
    template<muc::ceta_string AName, typename U>
    constexpr auto F() const& -> decltype(auto) { return F<AName>().template As<U>(); }
    template<muc::ceta_string AName, typename U>
    constexpr auto F() && -> decltype(auto) { return std::move(*this).template F<AName>().template As<U>(); }

    /// @brief Converts this tuple to a compile-time sub-tuple type.
    /// @tparam ATuple Target tuple type.
    /// @return A tuple containing the fields required by the target type.
    template<SubTuple<M> ATuple>
    constexpr auto As() const -> auto { return AsImpl<ATuple>(); }
    /// @brief Identity conversion for the same tuple type.
    /// @tparam ATuple The same tuple type.
    /// @return A const reference to this tuple.
    template<std::same_as<Tuple<M>> ATuple>
    constexpr auto As() const -> const auto& { return *this; }

    /// @brief Compares tuples by field names and values when models are equivalent.
    /// @tparam ATuple Compared tuple type.
    /// @param that Compared tuple object.
    /// @return true if all corresponding named fields are equal; false otherwise.
    template<TupleLike ATuple>
    constexpr auto operator==(const ATuple& that) const -> bool;
    /// @brief Three-way comparison is intentionally disabled.
    template<TupleLike ATuple>
    constexpr auto operator<=>(const ATuple&) const -> auto = delete;

    /// @brief Visits the field identified by runtime name and invokes a callable with that field.
    /// @param name Runtime field name.
    /// @param F Callable to invoke with the selected field.
    auto Visit(std::string_view name, auto&& F) const -> void { VisitImpl<0, Size() - 1>(DynIndex(name), std::forward<decltype(F)>(F)); }
    auto Visit(std::string_view name, auto&& F) -> void { VisitImpl<0, Size() - 1>(DynIndex(name), std::forward<decltype(F)>(F)); }

    /// @brief Returns the number of fields in the compile-time model.
    static constexpr auto Size() -> auto { return M::Size(); }
    /// @brief Returns runtime field names in model order.
    static constexpr auto NameVector() -> const auto& { return M::NameVector(); }

    /// @brief ADL helper forwarding to member Get.
    template<muc::ceta_string AName>
    friend constexpr auto F(const Tuple<M>& t) -> decltype(auto) { return t.template F<AName>(); }
    template<muc::ceta_string AName>
    friend constexpr auto F(Tuple<M>& t) -> decltype(auto) { return t.template F<AName>(); }
    template<muc::ceta_string AName>
    friend constexpr auto F(Tuple<M>&& t) -> decltype(auto) { return std::move(t).template F<AName>(); }
    template<muc::ceta_string AName>
    friend constexpr auto F(const Tuple<M>&& t) -> decltype(auto) { return std::move(t).template F<AName>(); }

    /// @brief ADL helper returning a named field converted via Value::As.
    template<muc::ceta_string AName, typename U>
    friend constexpr auto F(const Tuple<M>& t) -> decltype(auto) { return t.template F<AName, U>(); }
    template<muc::ceta_string AName, typename U>
    friend constexpr auto F(Tuple<M>&& t) -> decltype(auto) { return std::move(t).template F<AName, U>(); }

    /// @brief std::get-compatible ADL helper by compile-time index.
    template<gsl::index I>
    friend constexpr auto get(const Tuple<M>& t) -> decltype(auto) { return t.template F<std::tuple_element_t<I, Tuple<M>>::Name()>(); }
    template<gsl::index I>
    friend constexpr auto get(Tuple<M>& t) -> decltype(auto) { return t.template F<std::tuple_element_t<I, Tuple<M>>::Name()>(); }
    template<gsl::index I>
    friend constexpr auto get(Tuple<M>&& t) -> decltype(auto) { return std::move(t).template F<std::tuple_element_t<I, Tuple<M>>::Name()>(); }
    template<gsl::index I>
    friend constexpr auto get(const Tuple<M>&& t) -> decltype(auto) { return std::move(t).template F<std::tuple_element_t<I, Tuple<M>>::Name()>(); }

    /// @brief ADL helper forwarding to member As.
    template<SubTuple<M> ATuple>
    friend constexpr auto As(const Tuple<M>& t) -> decltype(auto) { return t.template As<ATuple>(); }

    /// @brief ADL helper forwarding to member Visit.
    friend auto Visit(const Tuple<M>& t, std::string_view name, auto&& F) -> void { t.Visit(name, std::forward<decltype(F)>(F)); }
    friend auto Visit(Tuple<M>& t, std::string_view name, auto&& F) -> void { t.Visit(name, std::forward<decltype(F)>(F)); }

private:
    /// @brief Internal getter preserving value category.
    template<gsl::index I>
        requires(0 <= I and I < M::Size())
    constexpr auto GetImpl() const& -> const std::tuple_element_t<I, Tuple>& { return std::get<I>(fTuple); }
    template<gsl::index I>
        requires(0 <= I and I < M::Size())
    constexpr auto GetImpl() & -> std::tuple_element_t<I, Tuple>& { return std::get<I>(fTuple); }
    template<gsl::index I>
        requires(0 <= I and I < M::Size())
    constexpr auto GetImpl() && -> std::tuple_element_t<I, Tuple>&& { return std::get<I>(std::move(fTuple)); }
    template<gsl::index I>
        requires(0 <= I and I < M::Size())
    constexpr auto GetImpl() const&& -> const std::tuple_element_t<I, Tuple>&& { return std::get<I>(std::move(fTuple)); }

    /// @brief Internal implementation for compile-time tuple conversion.
    template<TupleLike ATuple>
    constexpr auto AsImpl() const -> ATuple;

    /// @brief Internal runtime-dispatch visit implementation over compile-time index range.
    template<gsl::index L, gsl::index R>
    MUSTARD_ALWAYS_INLINE auto VisitImpl(gsl::index i, auto&& F) const -> void;
    template<gsl::index L, gsl::index R>
    MUSTARD_ALWAYS_INLINE auto VisitImpl(gsl::index i, auto&& F) -> void;

    /// @brief Resolves runtime field name to compile-time field index.
    MUSTARD_ALWAYS_INLINE static auto DynIndex(std::string_view name) -> gsl::index;

private:
    /// @brief Underlying storage tuple following the static model layout.
    typename M::StdTuple fTuple;

    /// @brief Runtime lookup table from field name to model index.
    static const gtl::flat_hash_map<std::string_view, gsl::index> fgDynIndexMap;
};

/// @brief Class template argument deduction guide for Tuple.
template<typename M>
Tuple(M) -> Tuple<M>;

/// @brief Automatically reference-counted tuple type.
/// This is essentially an alias for Arc<Tuple<M>>, provided as a shorthand.
template<Modelized M>
using ArcTuple = Arc<Tuple<M>>;

/// @brief Construct an automatically reference-counted tuple.
/// @tparam M Tuple model.
/// @param ...args Arguments passed to tuple constructor.
/// @return The automatically reference-counted tuple.
template<Modelized M, typename... Us>
    requires std::constructible_from<Tuple<M>, Us&&...>
auto MakeArcTuple(Us&&... args) -> ArcTuple<M> {
    return MakeArc<Tuple<M>>(std::forward<Us>(args)...);
}

} // namespace Mustard::Data::inline Object

#include "Mustard/Data/Object/Tuple.inl"
