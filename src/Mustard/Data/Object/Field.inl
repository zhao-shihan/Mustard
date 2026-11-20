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

namespace impl2 {

template<typename ATo, FieldConvertibleTo<ATo> AFrom>
constexpr auto FieldObjectCast(AFrom&& src) -> std::conditional_t<std::same_as<AFrom, ATo>, AFrom&&, ATo> {
    if constexpr (std::is_lvalue_reference_v<AFrom&&>) {
        if constexpr (std::same_as<std::decay_t<AFrom>, ATo> or std::convertible_to<const AFrom&, ATo>) {
            return src;
        } else if constexpr (requires { static_cast<ATo>(src); }) {
            return static_cast<ATo>(src);
        } else if constexpr (requires { ATo(std::ranges::begin(src), std::ranges::end(src)); }) {
            return ATo(std::ranges::begin(src), std::ranges::end(src));
        } else if constexpr (std::default_initializable<ATo> and
                             requires(ATo dest) { std::ranges::copy(src, std::ranges::begin(dest)); } and
                             std::movable<ATo>) {
            ATo dest;
            std::ranges::copy(src, std::ranges::begin(dest));
            return dest;
        } else if constexpr (VectorConvertibleTo<const AFrom&, ATo>) {
            return VectorCast<ATo>(src);
        } else {
            static_assert(muc::dependent_false<ATo>, "FieldObjectCast cannot convert from the source type to the target type.");
        }
    } else if constexpr (std::is_rvalue_reference_v<AFrom&&>) {
        if constexpr (std::same_as<std::decay_t<AFrom>, ATo> or std::convertible_to<AFrom, ATo>) {
            return std::move(src);
        } else if constexpr (requires { static_cast<ATo>(std::move(src)); }) {
            return static_cast<ATo>(std::move(src));
        } else if constexpr (requires { ATo(std::move_iterator{std::ranges::begin(src)}, std::move_iterator{std::ranges::end(src)}); }) {
            return ATo(std::move_iterator{std::ranges::begin(src)}, std::move_iterator{std::ranges::end(src)});
        } else if constexpr (std::default_initializable<ATo> and
                             requires(ATo dest) { std::ranges::move(src, std::ranges::begin(dest)); } and
                             std::movable<ATo>) {
            ATo dest;
            std::ranges::move(src, std::ranges::begin(dest));
            return dest;
        } else if constexpr (VectorConvertibleTo<AFrom, ATo>) {
            return VectorCast<ATo>(std::forward<AFrom>(src));
        } else {
            static_assert(muc::dependent_false<ATo>, "FieldObjectCast cannot convert from the source type to the target type.");
        }
    } else {
        static_assert(muc::dependent_false<ATo>, "Is that a joke?");
    }
}

} // namespace impl2

template<typename T, muc::ceta_string AName, ROOTX::RNTuplePersistable U, muc::ceta_string ADescription>
template<impl2::FieldConvertibleTo<T> V>
constexpr Field<T, AName, U, ADescription>::Field(V&& object) noexcept(std::is_nothrow_constructible_v<T, V&&>) :
    fObject(impl2::FieldObjectCast<T>(std::forward<V>(object))) {}

template<typename T, muc::ceta_string AName, ROOTX::RNTuplePersistable U, muc::ceta_string ADescription>
template<typename V>
    requires impl2::FieldAssignableFrom<T&, V&&>
constexpr auto Field<T, AName, U, ADescription>::operator=(V&& object) & noexcept(std::is_nothrow_assignable_v<T, V&&>) -> auto& {
    if constexpr (std::assignable_from<T&, V&&>) {
        fObject = std::forward<V>(object);
    } else if constexpr (std::movable<T> and impl2::FieldConvertibleTo<V&&, T>) {
        fObject = impl2::FieldObjectCast<T>(std::forward<V>(object));
    } else {
        static_assert(muc::dependent_false<V>, "Field::operator= cannot assign from the source type.");
    }
    return *this;
}

} // namespace Mustard::Data::inline Object
