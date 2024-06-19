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

#include "Mustard/Data/internal/TypeTraits.h++"
#include "Mustard/Utility/NonConstructibleBase.h++"
#include "Mustard/Utility/VectorAssign.h++"
#include "Mustard/Utility/VectorCast.h++"

#include "muc/ceta_string"

#include "gsl/gsl"

#include "fmt/format.h"

#include <array>
#include <concepts>
#include <memory>
#include <type_traits>
#include <utility>

namespace Mustard::Data {

template<typename T>
concept ValueAcceptable =
    [] {
        if constexpr (Concept::ROOTFundamental<T>) {
            return not std::same_as<std::decay_t<T>, gsl::zstring>;
        }
        if constexpr (internal::IsStdArray<T>{}) {
            return Concept::ROOTFundamental<typename T::value_type> and
                   not std::same_as<std::decay_t<typename T::value_type>, gsl::zstring>;
        }
        return std::is_class_v<T>;
    }();

template<ValueAcceptable T, muc::ceta_string AName, muc::ceta_string ADescription = nullptr>
class [[nodiscard]] Value final {
public:
    using Type = T;

public:
    constexpr Value() = default;
    constexpr Value(const T& object);
    constexpr Value(T&& object) noexcept;
    template<typename U>
        requires(std::constructible_from<T, U> and not std::same_as<std::remove_cvref_t<U>, T>)
    constexpr Value(U&& object);

    template<typename U>
        requires(internal::IsStdArray<T>::value and not std::constructible_from<T, U> and not std::same_as<std::remove_cvref_t<U>, T> and
                 requires(U&& object) { VectorCast<T>(std::forward<U>(object)); })
    constexpr Value(U&& object);
    template<typename U>
        requires(internal::IsStdArray<T>::value and not std::constructible_from<T, U> and not std::same_as<std::remove_cvref_t<U>, T> and
                 requires(T fObject, U&& object) { VectorAssign(fObject, std::forward<U>(object)); })
    constexpr auto operator=(U&& object) -> auto&;

    constexpr operator const T&() const& noexcept { return fObject; }
    constexpr operator T&() & noexcept { return fObject; }
    constexpr operator T&&() && noexcept { return std::move(fObject); }
    constexpr operator const T&&() const&& noexcept { return std::move(fObject); }

    constexpr auto operator*() const& noexcept -> const T& { return fObject; }
    constexpr auto operator*() & noexcept -> T& { return fObject; }
    constexpr auto operator*() && noexcept -> T&& { return std::move(fObject); }
    constexpr auto operator*() const&& noexcept -> const T&& { return std::move(fObject); }

    constexpr auto operator->() const noexcept -> const T* { return std::addressof(fObject); }
    constexpr auto operator->() noexcept -> T* { return std::addressof(fObject); }

    template<typename U>
    constexpr auto As() const& -> std::conditional_t<std::same_as<T, U>, const T&, U>;
    template<typename U>
    constexpr auto As() && -> U;

    constexpr auto operator[](auto&& i) const& -> decltype(auto)
        requires requires(const T fObject) { fObject[std::forward<decltype(i)>(i)]; }
    { return fObject[std::forward<decltype(i)>(i)]; }
    constexpr auto operator[](auto&& i) & -> decltype(auto)
        requires requires(T fObject) { fObject[std::forward<decltype(i)>(i)]; }
    { return fObject[std::forward<decltype(i)>(i)]; }
    constexpr auto operator[](auto&& i) && -> decltype(auto)
        requires requires(T fObject) { std::move(fObject)[std::forward<decltype(i)>(i)]; }
    { return std::move(fObject)[std::forward<decltype(i)>(i)]; }
    constexpr auto operator[](auto&& i) const&& -> decltype(auto)
        requires requires(const T fObject) { std::move(fObject)[std::forward<decltype(i)>(i)]; }
    { return std::move(fObject)[std::forward<decltype(i)>(i)]; }

    template<ValueAcceptable U, muc::ceta_string N, muc::ceta_string D>
    constexpr auto operator==(const Value<U, N, D>& that) const -> auto { return **this == *that; }
    template<ValueAcceptable U, muc::ceta_string N, muc::ceta_string D>
    constexpr auto operator<=>(const Value<U, N, D>& that) const -> auto { return **this <=> *that; }

    static constexpr auto Name() { return AName; }
    static constexpr auto Description() { return ADescription; }

private:
    T fObject;
};

namespace internal {

template<typename>
struct IsValue
    : std::false_type {};

template<ValueAcceptable T, muc::ceta_string AName, muc::ceta_string ADescription>
struct IsValue<Value<T, AName, ADescription>>
    : std::true_type {};

} // namespace internal

} // namespace Mustard::Data

#include "Mustard/Data/Value.inl"
