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

namespace impl2 {

/// @brief Concept checking whether @c AFrom can be converted to @c ATo through Field's conversion pipeline.
///
/// The pipeline is split by value category of @c AFrom&&:
/// - For lvalue references: identity, @c static_cast, range construction, range copy, or VectorCast.
/// - For rvalue references: identity via move, @c static_cast from moved, move-iterator construction,
///   range move, or VectorCast from rvalue.
///
/// This concept is the building block for Field construction, assignment, and As() conversion.
///
/// @tparam AFrom Source type (decayed).
/// @tparam ATo   Target type (decayed).
template<typename AFrom, typename ATo>
concept FieldConvertibleTo = requires(AFrom src, ATo dest) {
    requires std::is_lvalue_reference_v<AFrom&&>;
    requires std::same_as<std::decay_t<AFrom>, ATo> or std::convertible_to<const AFrom&, ATo> or
                 requires { static_cast<ATo>(src); } or
                 requires { ATo(std::ranges::begin(src), std::ranges::end(src)); } or
                 (std::default_initializable<ATo> and
                  requires { std::ranges::copy(src, std::ranges::begin(dest)); } and
                  std::movable<ATo>) or
                 VectorConvertibleTo<const AFrom&, ATo>;
} or requires(AFrom src, ATo dest) {
    requires std::is_rvalue_reference_v<AFrom&&>;
    requires std::same_as<std::decay_t<AFrom>, ATo> or std::convertible_to<AFrom, ATo> or
                 requires { static_cast<ATo>(std::move(src)); } or
                 requires { ATo(std::move_iterator{std::ranges::begin(src)}, std::move_iterator{std::ranges::end(src)}); } or
                 (std::default_initializable<ATo> and
                  requires { std::ranges::move(src, std::ranges::begin(dest)); } and
                  std::movable<ATo>) or
                 VectorConvertibleTo<AFrom, ATo>;
};

/// @brief Single-dispatch conversion from @c AFrom to @c ATo, respecting value category.
///
/// Selects the cheapest applicable conversion path at compile time:
/// identity/move, @c static_cast, range construction, range copy/move, or VectorCast.
/// Identity conversion preserves the reference type via @c std::conditional_t.
///
/// @tparam ATo   Target type (decayed).
/// @tparam AFrom Source type, must satisfy @c FieldConvertibleTo<ATo>.
/// @param  src   Source object forwarded with its value category.
/// @return Converted value, or identity reference when @c AFrom and @c ATo are the same type.
template<typename ATo, FieldConvertibleTo<ATo> AFrom>
constexpr auto FieldObjectCast(AFrom&& src) -> std::conditional_t<std::same_as<AFrom, ATo>, AFrom&&, ATo>;

/// @brief Concept checking whether @c ALHS can be assigned from @c ARHS.
///
/// Satisfied when @c ALHS is an lvalue reference and either:
/// - direct @c assignable_from holds, or
/// - @c ALHS is movable and @c ARHS is @c FieldConvertibleTo the decayed lhs type.
/// @tparam ALHS Left-hand side type (expected lvalue reference).
/// @tparam ARHS Right-hand side source type.
template<typename ALHS, typename ARHS>
concept FieldAssignableFrom =
    std::is_lvalue_reference_v<ALHS> and
    (std::assignable_from<ALHS, ARHS> or
     (std::movable<std::decay_t<ALHS>> and
      FieldConvertibleTo<ARHS, std::decay_t<ALHS>>));

} // namespace impl2

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
    /// @brief Constructs payload from a forwarded source, dispatching via FieldObjectCast.
    /// @details Handles identity, @c static_cast, range construction, range copy/move, and VectorCast
    ///          through a single unified concept @c impl2::FieldConvertibleTo.
    /// @tparam V Source type constrained by @c impl2::FieldConvertibleTo<T>, defaulting to @c T.
    /// @param object Source object forwarded with its value category.
    template<impl2::FieldConvertibleTo<T> V = T>
    constexpr Field(V&& object) noexcept(std::is_nothrow_constructible_v<T, V&&>);

    /// @brief Assigns payload from a forwarded source, dispatching via FieldObjectCast when needed.
    /// @details Uses @c impl2::FieldAssignableFrom<T&, V&&> to unify direct assignment and
    ///          move+convert assignment paths.
    /// @tparam V Source type, defaulting to payload type.
    /// @param object Source object forwarded with its value category.
    /// @return This object by lvalue reference.
    template<typename V = T>
        requires impl2::FieldAssignableFrom<T&, V&&>
    constexpr auto operator=(V&& object) & noexcept(std::is_nothrow_assignable_v<T, V&&>) -> auto&;

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
        requires impl2::FieldConvertibleTo<T, V>
    constexpr auto As() const& -> decltype(auto) { return impl2::FieldObjectCast<V>(fObject); }
    /// @brief Converts payload to target type while consuming this Field.
    /// @tparam V Target type.
    /// @return Converted value.
    template<typename V>
        requires impl2::FieldConvertibleTo<T, V>
    constexpr auto As() && -> decltype(auto) { return impl2::FieldObjectCast<V>(std::move(fObject)); }

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
