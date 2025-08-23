// -*- C++ -*-
//
// Copyright 2020-2025  The Mustard development team
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

namespace Mustard::Data::internal {

template<typename ADerived, internal::UniqueStdTuple AStdTuple>
const std::vector<std::string> ModelBase<ADerived, AStdTuple>::fNameVector{
    []<gsl::index... Is>(gslx::index_sequence<Is...>) -> std::vector<std::string> {
        return {std::tuple_element_t<Is, typename ModelBase<ADerived, AStdTuple>::StdTuple>::Name().s()...};
    }(gslx::make_index_sequence<ModelBase<ADerived, AStdTuple>::Size()>{})};

template<typename ADerived, internal::UniqueStdTuple AStdTuple>
template<muc::ceta_string AName, gsl::index I>
consteval auto ModelBase<ADerived, AStdTuple>::IndexImpl() -> gsl::index {
    if constexpr (I == Size()) {
        static_assert(I < Size(), "no such value of this name within this data model");
        return StopConsteval();
    } else if constexpr (std::tuple_element_t<I, AStdTuple>::Name() == AName) {
        return I;
    } else {
        return IndexImpl<AName, I + 1>();
    }
}

} // namespace Mustard::Data::internal
