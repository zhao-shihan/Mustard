// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

#include "Mustard/Concept/Subscriptable.h++"

#include "muc/concepts"

#include <concepts>
#include <cstddef>
#include <limits>
#include <type_traits>

namespace Mustard::Concept {

template<typename T, typename F, std::size_t N = std::numeric_limits<std::size_t>::max()>
concept InputVector =
    requires {
        requires(N >= 2);
        requires muc::arithmetic<F>;
        requires(std::is_class_v<T> and
                 std::default_initializable<T>) or
                    (std::is_pointer_v<T> or
                     std::is_array_v<T>);
        requires SubscriptableToMaybeConstReferenced<T, F>;
    };

template<typename T, typename F>
concept InputVector2 = InputVector<T, F, 2>;
template<typename T, typename F>
concept InputVector3 = InputVector<T, F, 3>;
template<typename T, typename F>
concept InputVector4 = InputVector<T, F, 4>;
template<typename T>
concept InputVector2F = InputVector2<T, float>;
template<typename T>
concept InputVector3F = InputVector3<T, float>;
template<typename T>
concept InputVector4F = InputVector4<T, float>;
template<typename T>
concept InputVector2D = InputVector2<T, double>;
template<typename T>
concept InputVector3D = InputVector3<T, double>;
template<typename T>
concept InputVector4D = InputVector4<T, double>;

template<typename T, std::size_t N = std::numeric_limits<std::size_t>::max()>
concept InputVectorIntegral = InputVector<T, bool, N> or
                              InputVector<T, signed char, N> or
                              InputVector<T, unsigned char, N> or
                              InputVector<T, char, N> or
                              InputVector<T, char8_t, N> or
                              InputVector<T, char16_t, N> or
                              InputVector<T, char32_t, N> or
                              InputVector<T, wchar_t, N> or
                              InputVector<T, short, N> or
                              InputVector<T, int, N> or
                              InputVector<T, long, N> or
                              InputVector<T, long long, N> or
                              InputVector<T, unsigned short, N> or
                              InputVector<T, unsigned int, N> or
                              InputVector<T, unsigned long, N> or
                              InputVector<T, unsigned long long, N>;

template<typename T>
concept InputVector2Integral = InputVectorIntegral<T, 2>;
template<typename T>
concept InputVector3Integral = InputVectorIntegral<T, 3>;
template<typename T>
concept InputVector4Integral = InputVectorIntegral<T, 4>;

template<typename T, std::size_t N = std::numeric_limits<std::size_t>::max()>
concept InputVectorFloatingPoint = InputVector<T, float, N> or
                                   InputVector<T, double, N> or
                                   InputVector<T, long double, N>;

template<typename T>
concept InputVector2FloatingPoint = InputVectorFloatingPoint<T, 2>;
template<typename T>
concept InputVector3FloatingPoint = InputVectorFloatingPoint<T, 3>;
template<typename T>
concept InputVector4FloatingPoint = InputVectorFloatingPoint<T, 4>;

template<typename T, std::size_t N = std::numeric_limits<std::size_t>::max()>
concept InputVectorAny = InputVectorIntegral<T, N> or
                         InputVectorFloatingPoint<T, N>;

template<typename T>
concept InputVector2Any = InputVectorAny<T, 2>;
template<typename T>
concept InputVector3Any = InputVectorAny<T, 3>;
template<typename T>
concept InputVector4Any = InputVectorAny<T, 4>;

} // namespace Mustard::Concept
