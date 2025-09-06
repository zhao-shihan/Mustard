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

#include "Mustard/Data/Tuple.h++"
#include "Mustard/Data/internal/TypeTraits.h++"
#include "Mustard/Utility/NonConstructibleBase.h++"

#include "ROOT/RVec.hxx"

#include "muc/concepts"

#include "gsl/gsl"

#include <algorithm>
#include <concepts>
#include <type_traits>
#include <vector>

namespace Mustard::Data::internal {

template<TupleModelizable... Ts>
class ReadHelper : public NonConstructibleBase {
private:
    template<typename T>
    struct ValueTypeHelper;

    template<typename T>
        requires std::is_class_v<T>
    struct ValueTypeHelper<T> {
        using Type = typename T::value_type;
    };

    template<typename T>
        requires(not std::is_class_v<T>)
    struct ValueTypeHelper<T> {
        using Type = int;
    };

public:
    template<gsl::index I>
    using TargetType = typename std::tuple_element_t<I, Tuple<Ts...>>::Type;

    template<gsl::index I>
    using ReadType = std::conditional_t<IsStdArray<TargetType<I>>{} or
                                            muc::instantiated_from<TargetType<I>, std::vector>,
                                        ROOT::RVec<typename ValueTypeHelper<TargetType<I>>::Type>,
                                        TargetType<I>>;

    template<typename>
    static auto As(auto&& value) -> decltype(auto) {
        return std::forward<decltype(value)>(value);
    }

    template<muc::instantiated_from<std::vector> T, typename U>
        requires std::same_as<typename T::value_type, U>
    static auto As(const ROOT::RVec<U>& src) -> T {
        T dest(src.size());
        std::ranges::copy(src, dest.begin());
        return dest;
    }

    template<typename T, typename U>
        requires IsStdArray<T>::value and std::same_as<typename T::value_type, U>
    static auto As(const ROOT::RVec<U>& src) -> T {
        T dest;
        std::ranges::copy(src, dest.begin());
        return dest;
    }
};

} // namespace Mustard::Data::internal
