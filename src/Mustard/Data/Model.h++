// -*- C++ -*-
//
// Copyright (C) 2020-2025  Mustard developers
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

#include "Mustard/Data/Object/Value.h++"
#include "Mustard/Utility/NonConstructibleBase.h++"
#include "Mustard/gslx/index_sequence.h++"

#include "muc/ceta_string"
#include "muc/concepts"
#include "muc/tuple"

#include "gsl/gsl"

#include <concepts>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

namespace Mustard::Data {

namespace internal {

/// @brief Marker base type used to identify tuple model types.
struct ModelSignature : NonConstructibleBase {};

namespace internal {

/// @brief Concept requiring a std::tuple whose element field names are unique.
/// @details Each tuple element type is expected to expose a static Name used for uniqueness checks.
template<typename T>
concept UniqueStdTuple = requires {
    requires muc::instantiated_from<T, std::tuple>;
    requires([]<gsl::index... Is>(gslx::index_sequence<Is...>) consteval {
        return (... and ([]<gsl::index... Js, gsl::index I>(gslx::index_sequence<Js...>, std::integral_constant<gsl::index, I>) consteval {
                    return (... and static_cast<bool>((I != Js) xor (std::tuple_element_t<I, T>::Name() == std::tuple_element_t<Js, T>::Name())));
                }(gslx::make_index_sequence<std::tuple_size_v<T>>{}, std::integral_constant<gsl::index, Is>{})));
    }(gslx::make_index_sequence<std::tuple_size_v<T>>{}));
};

} // namespace internal

/// @brief Common implementation base for all Model specializations.
/// @tparam ADerived Concrete model type.
/// @tparam AStdTuple Underlying schema tuple type.
template<typename ADerived, internal::UniqueStdTuple AStdTuple>
class ModelBase : public ModelSignature {
public:
    /// @brief Underlying schema tuple type.
    using StdTuple = AStdTuple;

public:
    /// @brief Returns field count in the schema.
    static constexpr auto Size() -> auto { return std::tuple_size_v<StdTuple>; }

    /// @brief Looks up field index by compile-time field name.
    /// @tparam AName Field name.
    /// @return Zero-based field index in StdTuple.
    template<muc::ceta_string AName>
    static constexpr auto Index() -> auto { return IndexImpl<AName>(); }

    /// @brief Returns runtime vector of field names in declaration order.
    static auto NameVector() -> const auto& { return fNameVector; }

    /// @brief Field type alias resolved by compile-time field name.
    /// @tparam AName Field name.
    template<muc::ceta_string AName>
    using ValueAt = std::tuple_element_t<Index<AName>(), StdTuple>;

private:
    /// @brief Helper path used to terminate constexpr recursion flow.
    static auto StopConsteval() -> gsl::index { return -1; }

    /// @brief Recursive implementation of Index.
    /// @tparam AName Field name to find.
    /// @tparam I Current recursion index.
    template<muc::ceta_string AName, gsl::index I = 0>
    static consteval auto IndexImpl() -> gsl::index;

private:
    /// @brief Cached runtime field-name vector.
    static const std::vector<std::string> fNameVector;
};

} // namespace internal

/// @brief Types allowed as Model parameters.
/// @details A parameter can be an existing model type or a Value type.
template<typename T>
concept Modelizable = std::derived_from<T, internal::ModelSignature> or
                      internal::IsValue<T>::value;

/// @brief Empty Model specialization.
template<Modelizable...>
struct Model
    : internal::ModelBase<Model<>,
                          std::tuple<>> {};

/// @brief Model specialization that prepends an existing model.
/// @tparam AModel Existing model.
/// @tparam AOthers Remaining modelizable items.
template<std::derived_from<internal::ModelSignature> AModel, Modelizable... AOthers>
struct Model<AModel, AOthers...>
    : internal::ModelBase<Model<AModel, AOthers...>,
                          muc::tuple_concat_t<typename AModel::StdTuple,
                                              typename Model<AOthers...>::StdTuple>> {};

/// @brief Model specialization that prepends one Value field.
/// @tparam T Value payload type.
/// @tparam AName Field name.
/// @tparam ADescription Field description.
/// @tparam AOthers Remaining modelizable items.
template<ValueAcceptable T, muc::ceta_string AName, muc::ceta_string ADescription, Modelizable... AOthers>
struct Model<Value<T, AName, ADescription>, AOthers...>
    : internal::ModelBase<Model<Value<T, AName, ADescription>, AOthers...>,
                          muc::tuple_concat_t<std::tuple<Value<T, AName, ADescription>>,
                                              typename Model<AOthers...>::StdTuple>> {};

/// @brief Concept for concrete model types.
/// @note To define a model, inherit from a Model directly via the Model template.
/// The purpose of inheriting from Model and create a new type is to produce
/// a more human-readable compiler error/warning messages.
template<typename M>
concept Modelized = std::derived_from<M, internal::ModelSignature> and
                    not muc::instantiated_from<M, Model>;

/// @brief Take fields from a model by field name, producing a new model.
/// @tparam M Source model type.
/// @tparam ...ANames Compile-time field names to extract.
template<Modelized M, muc::ceta_string... ANames>
struct Take
    : decltype(std::apply(
          []<typename... Vs>(std::type_identity<Vs>...) -> std::type_identity<Model<Vs...>> { return {}; },
          muc::tuple_concat_t<std::conditional_t<
              M::template ValueAt<ANames>::Name() == ANames,
              std::tuple<std::type_identity<typename M::template ValueAt<ANames>>>,
              std::tuple<>>...>{}))::type{};

/// @brief Drop fields from a model by field name, producing a new model.
/// @tparam M Source model type.
/// @tparam ...ANames Compile-time field names to drop.
template<Modelized M, muc::ceta_string... ANames>
struct Drop
    : decltype(std::apply(
          []<typename... Vs>(std::type_identity<Vs>...) -> std::type_identity<Model<Vs...>> { return {}; },
          muc::tuple_concat_t<std::conditional_t<
              M::template ValueAt<ANames>::Name() == ANames,
              std::tuple<>,
              std::tuple<std::type_identity<typename M::template ValueAt<ANames>>>>...>{}))::type{};

/// @brief Concept requiring that model M1 is a sub-model of M2 by field-name inclusion.
template<typename M1, typename M2>
concept SubModel = requires {
    requires Modelized<M1>;
    requires Modelized<M2>;
    requires M1::Size() <= M2::Size();
    requires([]<gsl::index... Is>(gslx::index_sequence<Is...>) consteval {
        const auto compare{[]<gsl::index I, gsl::index... Js>(gslx::index_sequence<Js...>) consteval {
            return (... or (std::tuple_element_t<I, typename M1::StdTuple>::Name() ==
                            std::tuple_element_t<Js, typename M2::StdTuple>::Name()));
        }};
        return (... and compare.template operator()<Is>(gslx::make_index_sequence<M2::Size()>{}));
    }(gslx::make_index_sequence<M1::Size()>{}));
};

/// @brief Concept requiring that model M1 is a strict super-model of M2.
template<typename M1, typename M2>
concept ProperSuperModel = not SubModel<M1, M2>;

/// @brief Concept requiring that model M1 and M2 are equivalent.
template<typename M1, typename M2>
concept EquivalentModel = SubModel<M1, M2> and
                          SubModel<M2, M1>;

/// @brief Concept requiring that model M1 is a strict sub-model of M2.
template<typename M1, typename M2>
concept ProperSubModel = SubModel<M1, M2> and
                         not EquivalentModel<M1, M2>;

/// @brief Concept requiring that model M1 is a super-model of M2.
template<typename M1, typename M2>
concept SuperModel = ProperSuperModel<M1, M2> or
                     EquivalentModel<M1, M2>;

/// @brief Concept for model covers another model.
/// This is essentially an alias for SuperModel, provided for better readability in some contexts.
template<typename M1, typename M2>
concept ModelCoverable = SuperModel<M1, M2>;

} // namespace Mustard::Data

#include "Mustard/Data/Model.inl"
