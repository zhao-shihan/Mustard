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
template<typename U>
constexpr Value<T, AName, ADescription>::Value(U&& object) :
    fObject(std::forward<U>(object)) {}

template<ValueAcceptable T, muc::ceta_string AName, muc::ceta_string ADescription>
template<typename U>
    requires requires(U&& object) { VectorCast<T>(std::forward<U>(object)); }
constexpr Value<T, AName, ADescription>::Value(U&& object) :
    fObject(VectorCast<T>(std::forward<U>(object))) {}

template<ValueAcceptable T, muc::ceta_string AName, muc::ceta_string ADescription>
template<typename U>
constexpr auto Value<T, AName, ADescription>::operator=(U&& object) -> auto& {
    fObject = std::forward<U>(object);
    return *this;
}

template<ValueAcceptable T, muc::ceta_string AName, muc::ceta_string ADescription>
template<typename U>
    requires requires(T fObject, U&& object) { VectorAssign(fObject, std::forward<U>(object)); }
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
