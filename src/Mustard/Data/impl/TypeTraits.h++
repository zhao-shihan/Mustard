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

#include <array>
#include <bitset>
#include <map>
#include <optional>
#include <set>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace Mustard::Data::impl {

template<typename>
struct IsStdVector : std::false_type {};
template<typename T>
struct IsStdVector<std::vector<T>> : std::true_type {};

template<typename>
struct IsStdArray : std::false_type {};
template<typename T, std::size_t N>
struct IsStdArray<std::array<T, N>> : std::true_type {};

template<typename>
struct IsStdPair : std::false_type {};
template<typename T1, typename T2>
struct IsStdPair<std::pair<T1, T2>> : std::true_type {};

template<typename>
struct IsStdTuple : std::false_type {};
template<typename... Ts>
struct IsStdTuple<std::tuple<Ts...>> : std::true_type {};

template<typename>
struct IsStdBitset : std::false_type {};
template<std::size_t N>
struct IsStdBitset<std::bitset<N>> : std::true_type {};

template<typename>
struct IsStdOptional : std::false_type {};
template<typename T>
struct IsStdOptional<std::optional<T>> : std::true_type {};

template<typename>
struct IsStdSet : std::false_type {};
template<typename T>
struct IsStdSet<std::set<T>> : std::true_type {};

template<typename>
struct IsStdUnorderedSet : std::false_type {};
template<typename T>
struct IsStdUnorderedSet<std::unordered_set<T>> : std::true_type {};

template<typename>
struct IsStdMultiSet : std::false_type {};
template<typename T>
struct IsStdMultiSet<std::multiset<T>> : std::true_type {};

template<typename>
struct IsStdUnorderedMultiSet : std::false_type {};
template<typename T>
struct IsStdUnorderedMultiSet<std::unordered_multiset<T>> : std::true_type {};

template<typename>
struct IsStdMap : std::false_type {};
template<typename K, typename V>
struct IsStdMap<std::map<K, V>> : std::true_type {};

template<typename>
struct IsStdUnorderedMap : std::false_type {};
template<typename K, typename V>
struct IsStdUnorderedMap<std::unordered_map<K, V>> : std::true_type {};

template<typename>
struct IsStdMultiMap : std::false_type {};
template<typename K, typename V>
struct IsStdMultiMap<std::multimap<K, V>> : std::true_type {};

template<typename>
struct IsStdUnorderedMultiMap : std::false_type {};
template<typename K, typename V>
struct IsStdUnorderedMultiMap<std::unordered_multimap<K, V>> : std::true_type {};

} // namespace Mustard::Data::impl
