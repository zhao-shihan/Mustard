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

namespace Mustard::Data::inline Object::impl3 {

template<typename T>
consteval auto ValueAcceptableImpl() -> bool {
    if constexpr (ROOTX::RNTuplePersistableFundamental<T> or
                  std::same_as<T, std::string> or
                  impl::IsStdBitset<T>{}) {
        return true;
    } else if constexpr (impl::IsStdPair<T>{}) {
        return ValueAcceptableImpl<typename T::first_type>() and
               ValueAcceptableImpl<typename T::second_type>();
    } else if constexpr (impl::IsStdTuple<T>{}) {
        return []<gsl::index... Is>(gslx::index_sequence<Is...>) consteval {
            return (... and ValueAcceptableImpl<std::tuple_element_t<Is, T>>());
        }(gslx::make_index_sequence<std::tuple_size_v<T>>{});
    } else if constexpr (impl::IsStdVector<T>{} or
                         impl::IsStdArray<T>{} or
                         impl::IsStdOptional<T>{}) {
        return ValueAcceptableImpl<typename T::value_type>();
    } else if constexpr (impl::IsStdSet<T>{} or
                         impl::IsStdUnorderedSet<T>{} or
                         impl::IsStdMultiSet<T>{} or
                         impl::IsStdUnorderedMultiSet<T>{}) {
        return ValueAcceptableImpl<typename T::key_type>();
    } else if constexpr (impl::IsStdMap<T>{} or
                         impl::IsStdUnorderedMap<T>{} or
                         impl::IsStdMultiMap<T>{} or
                         impl::IsStdUnorderedMultiMap<T>{}) {
        return ValueAcceptableImpl<typename T::key_type>() and
               ValueAcceptableImpl<typename T::mapped_type>();
    } else {
        return false;
    }
}

} // namespace Mustard::Data::inline Object::impl3
