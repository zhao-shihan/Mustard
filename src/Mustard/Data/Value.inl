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

template<ValueAcceptable T, muc::ceta_string AName, muc::ceta_string ADescription>
constexpr Value<T, AName, ADescription>::Value(const T& object) :
    fObject{object} {}

template<ValueAcceptable T, muc::ceta_string AName, muc::ceta_string ADescription>
constexpr Value<T, AName, ADescription>::Value(T&& object) noexcept :
    fObject{std::move_if_noexcept(object)} {}

// template<ValueAcceptable T, muc::ceta_string AName, muc::ceta_string ADescription>
// template<typename SF>
//     requires std::same_as<SF, typename Value<typename SF::Type>::template Tuple<SF::AModel>> and
//              std::constructible_from<T, const typename SF::Type&>
// constexpr Value<T, AName, ADescription>::Value(const SF& that) :
//     fObject{that} {}

// template<ValueAcceptable T, muc::ceta_string AName, muc::ceta_string ADescription>
// template<typename SF>
//     requires std::same_as<SF, typename Value<typename SF::Type>::template Tuple<SF::AModel>> and
//              std::constructible_from<T, const typename SF::Type&>
// constexpr Value<T, AName, ADescription>::Value(SF&& that) :
//     fObject{std::move(that)} {}

// template<ValueAcceptable T, muc::ceta_string AName, muc::ceta_string ADescription>
// constexpr Value<T, AName, ADescription>::Value(auto&& v) // clang-format off
//     requires requires { VectorCast<T>(std::forward<decltype(v)>(v)); } : // clang-format on
//     fObject{VectorCast<T>(std::forward<decltype(v)>(v))} {}

template<ValueAcceptable T, muc::ceta_string AName, muc::ceta_string ADescription>
template<typename U>
    requires(std::constructible_from<T, U> and not std::same_as<std::remove_cvref_t<U>, T>)
constexpr Value<T, AName, ADescription>::Value(U&& object) :
    fObject{static_cast<T>(std::forward<U>(object))} {}

template<ValueAcceptable T, muc::ceta_string AName, muc::ceta_string ADescription>
template<typename U>
    requires(internal::IsStdArray<T>::value and not std::constructible_from<T, U> and not std::same_as<std::remove_cvref_t<U>, T> and
             requires(U&& object) { VectorCast<T>(std::forward<U>(object)); })
constexpr Value<T, AName, ADescription>::Value(U&& object) :
    fObject{VectorCast<T>(std::forward<U>(object))} {}

template<ValueAcceptable T, muc::ceta_string AName, muc::ceta_string ADescription>
template<typename U>
    requires(internal::IsStdArray<T>::value and not std::constructible_from<T, U> and not std::same_as<std::remove_cvref_t<U>, T> and
             requires(T fObject, U&& object) { VectorAssign(fObject, std::forward<U>(object)); })
constexpr auto Value<T, AName, ADescription>::operator=(U&& object) -> auto& {
    VectorAssign(fObject, std::forward<U>(object));
    return *this;
}

template<ValueAcceptable T, muc::ceta_string AName, muc::ceta_string ADescription>
template<typename U>
constexpr auto Value<T, AName, ADescription>::As() const& -> std::conditional_t<std::same_as<T, U>, const T&, U> {
    if constexpr (std::same_as<T, U> or std::convertible_to<const T&, U>) {
        return fObject;
    } else if constexpr (std::convertible_to<const T&, U>) {
        return static_cast<U>(fObject);
    } else if constexpr (requires { VectorCast<U>(fObject); }) {
        return VectorCast<U>(fObject);
    } else {
        struct {
        } failed;
        return failed;
    }
}

template<ValueAcceptable T, muc::ceta_string AName, muc::ceta_string ADescription>
template<typename U>
constexpr auto Value<T, AName, ADescription>::As() && -> U {
    if constexpr (std::convertible_to<T, U>) {
        return std::move(fObject);
    } else if constexpr (requires { static_cast<U>(std::move(fObject)); }) {
        return static_cast<U>(std::move(fObject));
    } else if constexpr (requires { VectorCast<U>(std::move(fObject)); }) {
        return VectorCast<U>(std::move(fObject));
    } else {
        struct {
        } failed;
        return failed;
    }
}

} // namespace Mustard::Data
