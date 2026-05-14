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

#include "Mustard/ROOTX/RNTuplePersistable.h++"
#include "Mustard/Utility/VectorAssign.h++"
#include "Mustard/Utility/VectorCast.h++"

#include "muc/ceta_string"
#include "muc/utility"

#include "gsl/gsl"

#include <concepts>
#include <iterator>
#include <type_traits>
#include <utility>

namespace Mustard::Data::inline Object {

/// @brief Named field wrapper used as the atomic schema element of Model and Tuple.
///
/// Field binds a runtime payload type and a persistent storage type to compile-time field metadata.
/// Model stores Field specializations in its StdTuple schema, and Tuple stores corresponding
/// runtime objects and exposes typed access through Get and Field::As.
///
/// The in-memory type may be any type, including types that are not directly persistable by ROOT
/// (e.g. plain @c int, @c long long). But if you wish to persist it, `As<PersistentType>()`
/// must be supported, where @c PersistentType is a directly persisted type.
///
/// @tparam T Wrapped payload type (in-memory type). Does not need to satisfy @c ROOTX::RNTuplePersistable.
/// @tparam U Persistent storage type used by Writer (TTree/RNTuple). Must satisfy @c ROOTX::RNTuplePersistable.
/// @tparam AName Compile-time field name.
/// @tparam ADescription Optional compile-time field description.
template<typename T, ROOTX::RNTuplePersistable U, muc::ceta_string AName, muc::ceta_string ADescription = nullptr>
class [[nodiscard]] Field final {
public:
    /// @brief Wrapped payload type alias (in-memory type).
    using Type = T;
    /// @brief Persistent storage type alias.
    using PersistentType = U;

public:
    /// @brief Default-constructs the wrapped payload.
    constexpr Field() = default;
    /// @brief Constructs payload directly from a forwarded source object.
    /// @tparam V Source type, defaulting to payload type.
    /// @param object Source object.
    template<typename V = T>
    constexpr Field(V&& object) noexcept(std::is_nothrow_constructible_v<T, V&&>);
    /// @brief Constructs payload via VectorCast for compatible vector-like sources.
    /// @tparam V Source type.
    /// @param object Source object.
    template<VectorConvertibleTo<T> V>
    constexpr Field(V&& object);
    /// @brief Assigns payload directly from a forwarded source object.
    /// @tparam V Source type, defaulting to payload type.
    /// @param object Source object.
    /// @return This object.
    template<typename V = T>
    constexpr auto operator=(V&& object) noexcept(std::is_nothrow_assignable_v<T, V&&>) -> auto&;
    /// @brief Assigns payload via VectorAssign for compatible vector-like sources.
    /// @tparam V Source type.
    /// @param object Source object.
    /// @return This object.
    template<typename V>
        requires VectorAssignableFrom<T&, V&&>
    constexpr auto operator=(V&& object) -> auto&;

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

    /// @brief Converts payload to target type without consuming this Field.
    /// @details Used by Tuple::F<AName, U>() for typed field extraction.
    /// @tparam V Target type.
    /// @return Converted value, or const payload reference for identity conversion.
    template<typename V>
    constexpr auto As() const& -> std::conditional_t<std::same_as<T, V>, const T&, V>;
    /// @brief Converts payload to target type while consuming this Field.
    /// @tparam V Target type.
    /// @return Converted value.
    template<typename V>
    constexpr auto As() && -> std::conditional_t<std::same_as<T, V>, T&&, V>;

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

    /// @brief Equality compares wrapped payloads across different Field field declarations.
    /// @tparam V Compared payload type (in-memory type).
    /// @tparam W Compared persistent storage type.
    /// @tparam N Compared field name.
    /// @tparam D Compared field description.
    /// @param that Compared Field.
    /// @return true when wrapped payloads are equal.
    template<ROOTX::RNTuplePersistable V, ROOTX::RNTuplePersistable W, muc::ceta_string N, muc::ceta_string D>
    constexpr auto operator==(const Field<V, W, N, D>& that) const -> bool { return **this == *that; }
    /// @brief Three-way compares wrapped payloads across different Field field declarations.
    /// @tparam V Compared payload type (in-memory type).
    /// @tparam W Compared persistent storage type.
    /// @tparam N Compared field name.
    /// @tparam D Compared field description.
    /// @param that Compared Field.
    /// @return Ordering result of wrapped payload comparison.
    template<ROOTX::RNTuplePersistable V, ROOTX::RNTuplePersistable W, muc::ceta_string N, muc::ceta_string D>
    constexpr auto operator<=>(const Field<V, W, N, D>& that) const -> auto { return **this <=> *that; }

    /// @brief Returns compile-time field name carried by this Field.
    /// @details This metadata is used by Model::Index and Tuple::F<AName>().
    static constexpr auto Name() -> const auto& { return AName; }
    /// @brief Returns compile-time field description carried by this Field.
    static constexpr auto Description() -> const auto& { return ADescription; }

private:
    /// @brief Wrapped runtime payload object.
    T fObject;
};

namespace impl2 {

/// @brief Type trait to identify Field specializations.
/// @tparam Any type to test.
template<typename>
struct IsField
    : std::false_type {};

/// @brief Type trait to identify Field specializations.
/// @tparam T Wrapped payload type (in-memory type).
/// @tparam U Persistent storage type.
/// @tparam AName Field name.
/// @tparam ADescription Field description.
template<typename T, ROOTX::RNTuplePersistable U, muc::ceta_string AName, muc::ceta_string ADescription>
struct IsField<Field<T, U, AName, ADescription>>
    : std::true_type {};

} // namespace impl2

} // namespace Mustard::Data::inline Object

#include "Mustard/Data/Object/Field.inl"
