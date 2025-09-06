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

#include "Mustard/Data/Value.h++"
#include "Mustard/Utility/NonConstructibleBase.h++"
#include "Mustard/gslx/index_sequence.h++"

#include "muc/ceta_string"
#include "muc/concepts"
#include "muc/tuple"

#include "gsl/gsl"

#include <algorithm>
#include <array>
#include <concepts>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>

namespace Mustard::Data {

namespace internal {

struct ModelSignature : NonConstructibleBase {};

namespace internal {

template<typename T>
concept UniqueStdTuple =
    requires {
        requires muc::instantiated_from<T, std::tuple>;
        requires([]<gsl::index... Is>(gslx::index_sequence<Is...>) {
            return (... and ([]<gsl::index... Js, gsl::index I>(gslx::index_sequence<Js...>, std::integral_constant<gsl::index, I>) {
                        return (... and static_cast<bool>((I != Js) xor (std::tuple_element_t<I, T>::Name() == std::tuple_element_t<Js, T>::Name())));
                    }(gslx::make_index_sequence<std::tuple_size_v<T>>{}, std::integral_constant<gsl::index, Is>{})));
        }(gslx::make_index_sequence<std::tuple_size_v<T>>{}));
    };

} // namespace internal

template<typename ADerived, internal::UniqueStdTuple AStdTuple>
struct ModelBase : ModelSignature {
    using StdTuple = AStdTuple;

    static constexpr auto Size() -> auto { return std::tuple_size_v<StdTuple>; }

    template<muc::ceta_string AName>
    static constexpr auto Index() -> auto { return IndexImpl<AName>(); }

    static auto NameVector() -> const auto& { return fNameVector; }

    template<muc::ceta_string AName>
    using ValueOf = std::tuple_element_t<Index<AName>(), StdTuple>;

private:
    static auto StopConsteval() -> gsl::index { return -1; }
    template<muc::ceta_string AName, gsl::index I = 0>
    static consteval auto IndexImpl() -> gsl::index;

private:
    static const std::vector<std::string> fNameVector;
};

} // namespace internal

template<typename T>
concept TupleModelizable = static_cast<bool>(std::derived_from<T, internal::ModelSignature> xor internal::IsValue<T>::value);

template<TupleModelizable...>
struct TupleModel final
    : internal::ModelBase<TupleModel<>,
                          std::tuple<>> {};

template<std::derived_from<internal::ModelSignature> AModel, TupleModelizable... AOthers>
struct TupleModel<AModel, AOthers...> final
    : internal::ModelBase<TupleModel<AModel, AOthers...>,
                          muc::tuple_concat_t<typename AModel::StdTuple,
                                              typename TupleModel<AOthers...>::StdTuple>> {};

template<ValueAcceptable T, muc::ceta_string AName, muc::ceta_string ADescription, TupleModelizable... AOthers>
struct TupleModel<Value<T, AName, ADescription>, AOthers...> final
    : internal::ModelBase<TupleModel<Value<T, AName, ADescription>, AOthers...>,
                          muc::tuple_concat_t<std::tuple<Value<T, AName, ADescription>>,
                                              typename TupleModel<AOthers...>::StdTuple>> {};

template<typename M1, typename M2>
concept SubTupleModel = requires {
    requires muc::instantiated_from<M1, TupleModel>;
    requires muc::instantiated_from<M2, TupleModel>;
    requires M1::Size() <= M2::Size();
    requires([]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        return (... and
                ([]<gsl::index... Js, gsl::index I>(gslx::index_sequence<Js...>, std::integral_constant<gsl::index, I>) {
                    return (... or
                            (std::tuple_element_t<I, typename M1::StdTuple>::Name() ==
                             std::tuple_element_t<Js, typename M2::StdTuple>::Name()));
                }(gslx::make_index_sequence<M2::Size()>{}, std::integral_constant<gsl::index, Is>{})));
    }(gslx::make_index_sequence<M1::Size()>{}));
};

template<typename M1, typename M2>
concept ProperSuperTupleModel = not SubTupleModel<M1, M2>;

template<typename M1, typename M2>
concept EquivalentTupleModel = SubTupleModel<M1, M2> and
                               SubTupleModel<M2, M1>;

template<typename M1, typename M2>
concept ProperSubTupleModel = SubTupleModel<M1, M2> and
                              not EquivalentTupleModel<M1, M2>;

template<typename M1, typename M2>
concept SuperTupleModel = ProperSuperTupleModel<M1, M2> or
                          EquivalentTupleModel<M1, M2>;

} // namespace Mustard::Data

#include "Mustard/Data/TupleModel.inl"
