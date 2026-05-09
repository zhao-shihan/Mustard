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

#include "Mustard/Data/Object/ValueAcceptable.h++"
#include "Mustard/Utility/VectorAssign.h++"
#include "Mustard/Utility/VectorCast.h++"

#include "muc/ceta_string"
#include "muc/utility"

#include "gsl/gsl"

#include <concepts>
#include <type_traits>
#include <utility>

namespace Mustard::Data::inline Object {

/// @brief Named field wrapper used as the atomic schema element of Model and Tuple.
/// @details
/// Value binds a runtime payload type to compile-time field metadata.
/// Model stores Value specializations in its StdTuple schema, and Tuple stores corresponding
/// runtime objects and exposes typed access through Get and Value::As.
/// @tparam T Wrapped payload type.
/// @tparam AName Compile-time field name.
/// @tparam ADescription Optional compile-time field description.
template<ValueAcceptable T, muc::ceta_string AName, muc::ceta_string ADescription = nullptr>
class [[nodiscard]] Value final {
public:
    /// @brief Wrapped payload type alias.
    using Type = T;

public:
    /// @brief Default-constructs the wrapped payload.
    constexpr Value() = default;

    /// @brief Constructs payload directly from a forwarded source object.
    /// @tparam U Source type, defaulting to payload type.
    /// @param object Source object.
    template<typename U = T>
    constexpr Value(U&& object) noexcept(std::is_nothrow_constructible_v<T, U&&>);

    /// @brief Constructs payload via VectorCast for compatible vector-like sources.
    /// @tparam U Source type.
    /// @param object Source object.
    template<typename U>
        requires requires(U&& object) { VectorCast<T>(std::forward<U>(object)); }
    constexpr Value(U&& object);

    /// @brief Assigns payload directly from a forwarded source object.
    /// @tparam U Source type, defaulting to payload type.
    /// @param object Source object.
    /// @return This object.
    template<typename U = T>
    constexpr auto operator=(U&& object) noexcept(std::is_nothrow_assignable_v<T, U&&>) -> auto&;

    /// @brief Assigns payload via VectorAssign for compatible vector-like sources.
    /// @tparam U Source type.
    /// @param object Source object.
    /// @return This object.
    template<typename U>
        requires requires(T fObject, U&& object) { VectorAssign(fObject, std::forward<U>(object)); }
    constexpr auto operator=(U&& object) -> auto&;

    /// @brief Implicit conversion to const lvalue reference of payload.
    constexpr operator const T&() const& { return fObject; }
    /// @brief Implicit conversion to lvalue reference of payload.
    constexpr operator T&() & { return fObject; }
    /// @brief Implicit conversion to rvalue reference of payload.
    constexpr operator T&&() && { return std::move(fObject); }
    /// @brief Implicit conversion to const rvalue reference of payload.
    constexpr operator const T&&() const&& { return std::move(fObject); }

    /// @brief Dereference-like access to payload.
    constexpr auto operator*() const& -> const T& { return fObject; }
    /// @brief Dereference-like access to payload.
    constexpr auto operator*() & -> T& { return fObject; }
    /// @brief Dereference-like access to moved payload.
    constexpr auto operator*() && -> T&& { return std::move(fObject); }
    /// @brief Dereference-like access to const moved payload.
    constexpr auto operator*() const&& -> const T&& { return std::move(fObject); }

    /// @brief Pointer-like access to payload.
    constexpr auto operator->() const -> const T* { return std::addressof(fObject); }
    /// @brief Pointer-like access to payload.
    constexpr auto operator->() -> T* { return std::addressof(fObject); }

    /// @brief Converts payload to target type without consuming this Value.
    /// @details Used by Tuple::Get<AName, U>() for typed field extraction.
    /// @tparam U Target type.
    /// @return Converted value, or const payload reference for identity conversion.
    template<typename U>
    constexpr auto As() const& -> std::conditional_t<std::same_as<T, U>, const T&, U>;

    /// @brief Converts payload to target type while consuming this Value.
    /// @tparam U Target type.
    /// @return Converted value.
    template<typename U>
    constexpr auto As() && -> U;

    /// @brief Forwards index operator to the wrapped payload.
    /// @param i Index or key forwarded to payload operator[].
    /// @return Element access result preserving value category.
    constexpr auto operator[](auto&& i) const& -> decltype(auto)
        requires requires(const T fObject) { fObject[std::forward<decltype(i)>(i)]; }
    { return fObject[std::forward<decltype(i)>(i)]; }
    /// @brief Forwards index operator to the wrapped payload.
    /// @param i Index or key forwarded to payload operator[].
    /// @return Element access result preserving value category.
    constexpr auto operator[](auto&& i) & -> decltype(auto)
        requires requires(T fObject) { fObject[std::forward<decltype(i)>(i)]; }
    { return fObject[std::forward<decltype(i)>(i)]; }
    /// @brief Forwards index operator to moved payload.
    /// @param i Index or key forwarded to payload operator[].
    /// @return Element access result preserving value category.
    constexpr auto operator[](auto&& i) && -> decltype(auto)
        requires requires(T fObject) { std::move(fObject)[std::forward<decltype(i)>(i)]; }
    { return std::move(fObject)[std::forward<decltype(i)>(i)]; }
    /// @brief Forwards index operator to const moved payload.
    /// @param i Index or key forwarded to payload operator[].
    /// @return Element access result preserving value category.
    constexpr auto operator[](auto&& i) const&& -> decltype(auto)
        requires requires(const T fObject) { std::move(fObject)[std::forward<decltype(i)>(i)]; }
    { return std::move(fObject)[std::forward<decltype(i)>(i)]; }

    /// @brief Equality compares wrapped payloads across different Value field declarations.
    /// @tparam U Compared payload type.
    /// @tparam N Compared field name.
    /// @tparam D Compared field description.
    /// @param that Compared Value.
    /// @return true when wrapped payloads are equal.
    template<ValueAcceptable U, muc::ceta_string N, muc::ceta_string D>
    constexpr auto operator==(const Value<U, N, D>& that) const -> bool { return **this == *that; }

    /// @brief Three-way compares wrapped payloads across different Value field declarations.
    /// @tparam U Compared payload type.
    /// @tparam N Compared field name.
    /// @tparam D Compared field description.
    /// @param that Compared Value.
    /// @return Ordering result of wrapped payload comparison.
    template<ValueAcceptable U, muc::ceta_string N, muc::ceta_string D>
    constexpr auto operator<=>(const Value<U, N, D>& that) const -> auto { return **this <=> *that; }

    /// @brief Returns compile-time field name carried by this Value.
    /// @details This metadata is used by Model::Index and Tuple::Get<AName>().
    static constexpr auto Name() -> const auto& { return AName; }

    /// @brief Returns compile-time field description carried by this Value.
    static constexpr auto Description() -> const auto& { return ADescription; }

private:
    /// @brief Wrapped runtime payload object.
    T fObject;
};

namespace impl2 {

/// @brief Type trait to identify Value specializations.
/// @tparam Any type to test.
template<typename>
struct IsValue
    : std::false_type {};

/// @brief Value specialization of IsValue.
/// @tparam T Wrapped payload type.
/// @tparam AName Field name.
/// @tparam ADescription Field description.
template<ValueAcceptable T, muc::ceta_string AName, muc::ceta_string ADescription>
struct IsValue<Value<T, AName, ADescription>>
    : std::true_type {};

} // namespace impl2

} // namespace Mustard::Data::inline Object

#include "Mustard/Data/Object/Value.inl"
