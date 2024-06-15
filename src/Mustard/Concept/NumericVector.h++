#pragma once

#include "Mustard/Concept/InputVector.h++"
#include "Mustard/Concept/Subscriptable.h++"
#include "Mustard/Extension/gslx/index_sequence.h++"

#include "gsl/gsl"

#include <array>
#include <concepts>
#include <cstddef>
#include <limits>
#include <type_traits>

namespace Mustard::Concept {

template<typename T, typename F, std::size_t N = std::numeric_limits<std::size_t>::max()>
concept NumericVector =
    requires {
        requires InputVector<T, F, N>;
        requires std::is_standard_layout_v<T>;
        requires std::is_class_v<T>;
        requires std::regular<T>;
        requires SubscriptableTo<T, F&>;
        requires(SubscriptableTo<std::add_const_t<T>, const F&> or
                 SubscriptableTo<std::add_const_t<T>, F>);
        requires sizeof(T) % sizeof(F) == 0;
        requires([]<gsl::index... Is>(gslx::index_sequence<Is...>) {
            return requires(T v, std::array<F, sizeof...(Is)> u) {
                ::delete ::new T{std::get<Is>(u)...};
                v = T{std::get<Is>(u)...};
                v = {std::get<Is>(u)...};
            };
        }(gslx::make_index_sequence<sizeof(T) / sizeof(F)>{}));
        requires(N == std::numeric_limits<std::size_t>::max() or
                 sizeof(T) / sizeof(F) == N);
    };

template<typename T, typename F>
concept NumericVector2 = NumericVector<T, F, 2>;
template<typename T, typename F>
concept NumericVector3 = NumericVector<T, F, 3>;
template<typename T, typename F>
concept NumericVector4 = NumericVector<T, F, 4>;
template<typename T>
concept NumericVector2F = NumericVector2<T, float>;
template<typename T>
concept NumericVector3F = NumericVector3<T, float>;
template<typename T>
concept NumericVector4F = NumericVector4<T, float>;
template<typename T>
concept NumericVector2D = NumericVector2<T, double>;
template<typename T>
concept NumericVector3D = NumericVector3<T, double>;
template<typename T>
concept NumericVector4D = NumericVector4<T, double>;

template<typename T, std::size_t N = std::numeric_limits<std::size_t>::max()>
concept NumericVectorIntegral = NumericVector<T, bool, N> or
                                NumericVector<T, signed char, N> or
                                NumericVector<T, unsigned char, N> or
                                NumericVector<T, char, N> or
                                NumericVector<T, char8_t, N> or
                                NumericVector<T, char16_t, N> or
                                NumericVector<T, char32_t, N> or
                                NumericVector<T, wchar_t, N> or
                                NumericVector<T, short, N> or
                                NumericVector<T, int, N> or
                                NumericVector<T, long, N> or
                                NumericVector<T, long long, N> or
                                NumericVector<T, unsigned short, N> or
                                NumericVector<T, unsigned int, N> or
                                NumericVector<T, unsigned long, N> or
                                NumericVector<T, unsigned long long, N>;

template<typename T>
concept NumericVector2Integral = NumericVectorIntegral<T, 2>;
template<typename T>
concept NumericVector3Integral = NumericVectorIntegral<T, 3>;
template<typename T>
concept NumericVector4Integral = NumericVectorIntegral<T, 4>;

template<typename T, std::size_t N = std::numeric_limits<std::size_t>::max()>
concept NumericVectorFloatingPoint = NumericVector<T, float, N> or
                                     NumericVector<T, double, N> or
                                     NumericVector<T, long double, N>;

template<typename T>
concept NumericVector2FloatingPoint = NumericVectorFloatingPoint<T, 2>;
template<typename T>
concept NumericVector3FloatingPoint = NumericVectorFloatingPoint<T, 3>;
template<typename T>
concept NumericVector4FloatingPoint = NumericVectorFloatingPoint<T, 4>;

template<typename T, std::size_t N = std::numeric_limits<std::size_t>::max()>
concept NumericVectorAny = NumericVectorIntegral<T, N> or
                           NumericVectorFloatingPoint<T, N>;

template<typename T>
concept NumericVector2Any = NumericVectorAny<T, 2>;
template<typename T>
concept NumericVector3Any = NumericVectorAny<T, 3>;
template<typename T>
concept NumericVector4Any = NumericVectorAny<T, 4>;

} // namespace Mustard::Concept
