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

#pragma once

#include "Mustard/Data/impl/TypeTraits.h++"
#include "Mustard/ROOTX/RNTuplePersistableFundamental.h++"
#include "Mustard/gslx/index_sequence.h++"

#include "gsl/gsl"

#include <concepts>
#include <type_traits>

namespace Mustard::Data::inline Object {

namespace impl3 {

template<typename T>
consteval auto ValueAcceptableImpl() -> bool;

} // namespace impl3

/// @brief Concept for payload types storable in Field.
///
/// Accepted categories (recursively for nested value types):
/// - RNTuple-persistable fundamental types, std::string, and std::bitset.
/// - std::pair and std::tuple when all contained element types are acceptable.
/// - std::vector, std::array, and std::optional when their value_type is acceptable.
/// - std::set/std::unordered_set/std::multiset/std::unordered_multiset
///   when their key_type is acceptable.
/// - std::map/std::unordered_map/std::multimap/std::unordered_multimap
///   when both key_type and mapped_type are acceptable.
/// All other types are rejected by this concept.
/// This concept is used by Field and, transitively, by Model specializations that carry Field fields.
///
/// @note Literal long and unsigned long are not accepted semantically (as their sizes are platform-dependent).
/// @note All types accepted by this concept are accepted by ROOT RNTuple, but some of them may not be
/// accepted by TTree. Users should check ROOT documentation for details on supported types and their
/// limitations when using TTreeWriter.
///
/// @note Limitation notice (ROOT 6.36):
/// - TTree does not support std::int8_t.
/// - TTree does not support std::array<std::string, N>.
/// - TTree does not support some map/set families types, but key_type=std::string/int and
///     mapped_type=int/float/double/std::string types are generally safe.
/// - as well as some other types... you should test before applying to TTree.
/// - RNTuple support to map/set families are broken.
/// - RDataFrame does not support std::optional.
///
/// @see https://root.cern/doc/master/md_tree_2ntuple_2doc_2BinaryFormatSpecification.html
/// for all types persistable in RNTuple
template<typename T>
concept ValueAcceptable = impl3::ValueAcceptableImpl<T>();

} // namespace Mustard::Data::inline Object

#include "Mustard/Data/Object/ValueAcceptable.inl"
