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

namespace Mustard::Data {

template<TupleModelizable... Ts>
auto Take<Ts...>::From(ROOT::RDF::RNode rdf) -> muc::shared_ptrvec<Tuple<Ts...>> {
    muc::shared_ptrvec<Tuple<Ts...>> data;
    // data.reserve(*rdf.Count());   -- slow!
    rdf.Foreach(TakeOne{data, gslx::make_index_sequence<Tuple<Ts...>::Size()>{}},
                Tuple<Ts...>::NameVector());
    return data;
}

template<TupleModelizable... Ts>
template<gsl::index... Is>
class Take<Ts...>::TakeOne {
public:
    TakeOne(muc::shared_ptrvec<Tuple<Ts...>>& data, gslx::index_sequence<Is...>) :
        fData{data} {}

    auto operator()(const typename internal::ReadHelper<Ts...>::template ReadType<Is>&... value) -> void {
        fData.emplace_back(std::make_shared<Tuple<Ts...>>(
            internal::ReadHelper<Ts...>::template As<
                typename internal::ReadHelper<Ts...>::template TargetType<Is>>(value)...));
    }

private:
    muc::shared_ptrvec<Tuple<Ts...>>& fData;
};

} // namespace Mustard::Data
