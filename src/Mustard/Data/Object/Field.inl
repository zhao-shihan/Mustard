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

template<typename T, ROOTX::RNTuplePersistable U, muc::ceta_string AName, muc::ceta_string ADescription>
template<typename V>
constexpr Field<T, U, AName, ADescription>::Field(V&& object) noexcept(std::is_nothrow_constructible_v<T, V&&>) :
    fObject(std::forward<V>(object)) {}

template<typename T, ROOTX::RNTuplePersistable U, muc::ceta_string AName, muc::ceta_string ADescription>
template<VectorConvertibleTo<T> V>
constexpr Field<T, U, AName, ADescription>::Field(V&& object) :
    fObject(VectorCast<T>(std::forward<V>(object))) {}

template<typename T, ROOTX::RNTuplePersistable U, muc::ceta_string AName, muc::ceta_string ADescription>
template<typename V>
constexpr auto Field<T, U, AName, ADescription>::operator=(V&& object) noexcept(std::is_nothrow_assignable_v<T, V&&>) -> auto& {
    fObject = std::forward<V>(object);
    return *this;
}

template<typename T, ROOTX::RNTuplePersistable U, muc::ceta_string AName, muc::ceta_string ADescription>
template<typename V>
    requires VectorAssignableFrom<T&, V&&>
constexpr auto Field<T, U, AName, ADescription>::operator=(V&& object) -> auto& {
    VectorAssign(fObject, std::forward<V>(object));
    return *this;
}

template<typename T, ROOTX::RNTuplePersistable U, muc::ceta_string AName, muc::ceta_string ADescription>
template<typename V>
constexpr auto Field<T, U, AName, ADescription>::As() const& -> std::conditional_t<std::same_as<T, V>, const T&, V> {
    if constexpr (std::same_as<T, V> or std::convertible_to<const T&, V>) {
        return fObject;
    } else if constexpr (std::constructible_from<V, const T&>) {
        return static_cast<V>(fObject);
    } else if constexpr (requires { V(std::ranges::begin(fObject),
                                      std::ranges::end(fObject)); }) {
        return V(std::ranges::begin(fObject),
                 std::ranges::end(fObject));
    } else if constexpr (std::default_initializable<V> and
                         requires(V object) { std::ranges::copy(fObject, std::ranges::begin(object)); }) {
        V object;
        std::ranges::copy(fObject, std::ranges::begin(object));
        return object;
    } else if constexpr (VectorConvertibleTo<const T&, V>) {
        return VectorCast<V>(fObject);
    } else {
        static_assert(muc::dependent_false<V>, "Field::As<V>() cannot convert payload to the target type V.");
    }
}

template<typename T, ROOTX::RNTuplePersistable U, muc::ceta_string AName, muc::ceta_string ADescription>
template<typename V>
constexpr auto Field<T, U, AName, ADescription>::As() && -> std::conditional_t<std::same_as<T, V>, T&&, V> {
    if constexpr (std::same_as<T, V> or std::convertible_to<T, V>) {
        return std::move(fObject);
    } else if constexpr (std::constructible_from<V, T>) {
        return static_cast<V>(std::move(fObject));
    } else if constexpr (requires { V(std::move_iterator{std::ranges::begin(fObject)},
                                      std::move_iterator{std::ranges::end(fObject)}); }) {
        return V(std::move_iterator{std::ranges::begin(fObject)},
                 std::move_iterator{std::ranges::end(fObject)});
    } else if constexpr (std::default_initializable<V> and
                         requires(V object) { std::ranges::move(fObject, std::ranges::begin(object)); }) {
        V object;
        std::ranges::move(fObject, std::ranges::begin(object));
        return object;
    } else if constexpr (VectorConvertibleTo<T, V>) {
        return VectorCast<V>(std::move(fObject));
    } else {
        static_assert(muc::dependent_false<V>, "Field::As<V>() cannot convert payload to the target type V.");
    }
}

} // namespace Mustard::Data::inline Object
