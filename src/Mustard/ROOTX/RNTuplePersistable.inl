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

namespace Mustard::ROOTX::impl {

template<typename T>
consteval auto RNTuplePersistableImpl() -> bool {
    if constexpr (RNTuplePersistableFundamental<T> or
                  std::same_as<T, std::string> or
                  Mustard::Data::impl::IsStdBitset<T>{}) {
        return true;
    } else if constexpr (Mustard::Data::impl::IsStdPair<T>{}) {
        return RNTuplePersistableImpl<typename T::first_type>() and
               RNTuplePersistableImpl<typename T::second_type>();
    } else if constexpr (Mustard::Data::impl::IsStdTuple<T>{}) {
        return []<gsl::index... Is>(gslx::index_sequence<Is...>) consteval {
            return (... and RNTuplePersistableImpl<std::tuple_element_t<Is, T>>());
        }(gslx::make_index_sequence<std::tuple_size_v<T>>{});
    } else if constexpr (Mustard::Data::impl::IsStdVector<T>{} or
                         Mustard::Data::impl::IsStdArray<T>{} or
                         Mustard::Data::impl::IsStdOptional<T>{}) {
        return RNTuplePersistableImpl<typename T::value_type>();
    } else if constexpr (Mustard::Data::impl::IsStdSet<T>{} or
                         Mustard::Data::impl::IsStdUnorderedSet<T>{} or
                         Mustard::Data::impl::IsStdMultiSet<T>{} or
                         Mustard::Data::impl::IsStdUnorderedMultiSet<T>{}) {
        return RNTuplePersistableImpl<typename T::key_type>();
    } else if constexpr (Mustard::Data::impl::IsStdMap<T>{} or
                         Mustard::Data::impl::IsStdUnorderedMap<T>{} or
                         Mustard::Data::impl::IsStdMultiMap<T>{} or
                         Mustard::Data::impl::IsStdUnorderedMultiMap<T>{}) {
        return RNTuplePersistableImpl<typename T::key_type>() and
               RNTuplePersistableImpl<typename T::mapped_type>();
    } else {
        return false;
    }
}

} // namespace Mustard::ROOTX::impl
