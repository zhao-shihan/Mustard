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

#include "Mustard/Data/Model.h++"
#include "Mustard/Data/Object/Tuple.h++"

#include <concepts>
#include <ranges>
#include <utility>

namespace Mustard::Data::inline Container {

/// @brief Concept for sequential containers storing ArcTuple-like objects
template<typename C>
concept SeqArcTupleCtnr = requires {
    typename C::Model;
    typename C::value_type;
    requires Modelized<typename C::Model>;
    requires ArcTupleLike<typename C::value_type>;
    requires std::same_as<typename C::Model, typename C::value_type::Model>;
    requires std::ranges::range<C>;
};

/// @brief Concept for associative containers storing ArcTuple-like objects
template<typename C>
concept AssocArcTupleCtnr = requires {
    typename C::Model;
    typename C::value_type;
    typename C::key_type;
    typename C::mapped_type;
    requires Modelized<typename C::Model>;
    requires std::same_as<typename C::value_type, std::pair<const typename C::key_type, typename C::mapped_type>> or
                 std::same_as<typename C::value_type, std::pair<typename C::key_type, typename C::mapped_type>>;
    requires ArcTupleLike<typename C::mapped_type>;
    requires std::same_as<typename C::Model, typename C::mapped_type::Model>;
    requires std::ranges::range<C>;
};

/// @brief Concept for either sequential or associative containers storing ArcTuple-like objects
template<typename C>
concept ArcTupleCtnr = SeqArcTupleCtnr<C> or AssocArcTupleCtnr<C>;

/// @brief Concept for ArcTupleCtnr whose Model is equivalent to M
template<typename C, typename M>
concept EquivalentArcTupleCtnrModel = ArcTupleCtnr<C> and EquivalentModel<typename C::Model, M>;

/// @brief Concept for ArcTupleCtnr whose Model is a sub-model of M
template<typename C, typename M>
concept SubArcTupleCtnrModel = ArcTupleCtnr<C> and SubModel<typename C::Model, M>;

/// @brief Concept for ArcTupleCtnr whose Model is a super-model of M
template<typename C, typename M>
concept SuperArcTupleCtnrModel = ArcTupleCtnr<C> and SuperModel<typename C::Model, M>;

/// @brief Concept for ArcTupleCtnr whose Model is a proper sub-model of M
template<typename C, typename M>
concept ProperSubArcTupleCtnrModel = ArcTupleCtnr<C> and ProperSubModel<typename C::Model, M>;

/// @brief Concept for ArcTupleCtnr whose Model is a proper super-model of M
template<typename C, typename M>
concept ProperSuperArcTupleCtnrModel = ArcTupleCtnr<C> and ProperSuperModel<typename C::Model, M>;

/// @brief Concept for ArcTupleCtnr that covers Model M
template<typename C, typename M>
concept ArcTupleCtnrModelCoverable = SuperArcTupleCtnrModel<C, M>;

/// @brief Concept for SeqArcTupleCtnr whose Model is equivalent to M
template<typename C, typename M>
concept EquivalentSeqArcTupleCtnrModel = SeqArcTupleCtnr<C> and EquivalentArcTupleCtnrModel<C, M>;

/// @brief Concept for SeqArcTupleCtnr whose Model is a sub-model of M
template<typename C, typename M>
concept SubSeqArcTupleCtnrModel = SeqArcTupleCtnr<C> and SubArcTupleCtnrModel<C, M>;

/// @brief Concept for SeqArcTupleCtnr whose Model is a super-model of M
template<typename C, typename M>
concept SuperSeqArcTupleCtnrModel = SeqArcTupleCtnr<C> and SuperArcTupleCtnrModel<C, M>;

/// @brief Concept for SeqArcTupleCtnr whose Model is a proper sub-model of M
template<typename C, typename M>
concept ProperSubSeqArcTupleCtnrModel = SeqArcTupleCtnr<C> and ProperSubArcTupleCtnrModel<C, M>;

/// @brief Concept for SeqArcTupleCtnr whose Model is a proper super-model of M
template<typename C, typename M>
concept ProperSuperSeqArcTupleCtnrModel = SeqArcTupleCtnr<C> and ProperSuperArcTupleCtnrModel<C, M>;

/// @brief Concept for SeqArcTupleCtnr that covers Model M
template<typename C, typename M>
concept SeqArcTupleCtnrModelCoverable = SuperSeqArcTupleCtnrModel<C, M>;

/// @brief Concept for AssocArcTupleCtnr whose Model is equivalent to M
template<typename C, typename M>
concept EquivalentAssocArcTupleCtnrModel = AssocArcTupleCtnr<C> and EquivalentArcTupleCtnrModel<C, M>;

/// @brief Concept for AssocArcTupleCtnr whose Model is a sub-model of M
template<typename C, typename M>
concept SubAssocArcTupleCtnrModel = AssocArcTupleCtnr<C> and SubArcTupleCtnrModel<C, M>;

/// @brief Concept for AssocArcTupleCtnr whose Model is a super-model of M
template<typename C, typename M>
concept SuperAssocArcTupleCtnrModel = AssocArcTupleCtnr<C> and SuperArcTupleCtnrModel<C, M>;

/// @brief Concept for AssocArcTupleCtnr whose Model is a proper sub-model of M
template<typename C, typename M>
concept ProperSubAssocArcTupleCtnrModel = AssocArcTupleCtnr<C> and ProperSubArcTupleCtnrModel<C, M>;

/// @brief Concept for AssocArcTupleCtnr whose Model is a proper super-model of M
template<typename C, typename M>
concept ProperSuperAssocArcTupleCtnrModel = AssocArcTupleCtnr<C> and ProperSuperArcTupleCtnrModel<C, M>;

/// @brief Concept for AssocArcTupleCtnr that covers Model M
template<typename C, typename M>
concept AssocArcTupleCtnrModelCoverable = SuperAssocArcTupleCtnrModel<C, M>;

} // namespace Mustard::Data::inline Container
