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
#include "Mustard/gslx/index_sequence.h++"

#include "gsl/gsl"

#include <concepts>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace Mustard::ROOTX {

/// @brief Concept defining fundamental types persistable in ROOT RNTuple fields.
///
/// This concept checks whether a type `T` is one of the core fundamental types that ROOT can
/// persist in RNTuple fields. It is primarily used for template constraints where only basic
/// data types are accepted (e.g., serialization, type checking).
///
/// @tparam T Type to check against
/// @see https://root.cern/doc/master/md_tree_2ntuple_2doc_2BinaryFormatSpecification.html
/// for ROOT fundamental types persistable in RNTuple
template<typename T>
concept RNTuplePersistableFundamental =
    std::same_as<T, bool> or
    std::same_as<T, char> or
    std::same_as<T, std::int8_t> or
    std::same_as<T, std::uint8_t> or
    std::same_as<T, std::int16_t> or
    std::same_as<T, std::uint16_t> or
    std::same_as<T, std::int32_t> or
    std::same_as<T, std::uint32_t> or
    std::same_as<T, std::int64_t> or
    std::same_as<T, std::uint64_t> or
    (std::same_as<T, float> and std::numeric_limits<float>::is_iec559) or
    (std::same_as<T, double> and std::numeric_limits<double>::is_iec559);

namespace impl {

template<typename T>
consteval auto RNTuplePersistableImpl() -> bool;

} // namespace impl

/// @brief Concept for types persistable in ROOT RNTuple fields.
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
concept RNTuplePersistable = impl::RNTuplePersistableImpl<T>();

} // namespace Mustard::ROOTX

#include "Mustard/ROOTX/RNTuplePersistable.inl"
