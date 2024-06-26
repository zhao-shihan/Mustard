// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
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
auto Take<Ts...>::From(ROOTX::RDataFrame auto&& rdf) -> std::vector<std::shared_ptr<Tuple<Ts...>>> {
    std::vector<std::shared_ptr<Tuple<Ts...>>> data;
    rdf.Foreach(TakeOne{data, gslx::make_index_sequence<Tuple<Ts...>::Size()>{}},
                []<gsl::index... Is>(gslx::index_sequence<Is...>) -> std::vector<std::string> {
                    return {std::tuple_element_t<Is, Tuple<Ts...>>::Name().s()...};
                }(gslx::make_index_sequence<Tuple<Ts...>::Size()>{}));
    return data;
}

template<TupleModelizable... Ts>
template<gsl::index... Is>
class Take<Ts...>::TakeOne {
private:
    template<typename T>
    struct ValueTypeHelper;

    template<typename T>
        requires std::is_class_v<T>
    struct ValueTypeHelper<T> {
        using Type = typename T::value_type;
    };

    template<typename T>
        requires(not std::is_class_v<T>)
    struct ValueTypeHelper<T> {
        using Type = int;
    };

    template<gsl::index I>
    using ReadType = std::conditional_t<internal::IsStdArray<typename std::tuple_element_t<I, Tuple<Ts...>>::Type>{} or
                                            muc::instantiated_from<typename std::tuple_element_t<I, Tuple<Ts...>>::Type, std::vector>,
                                        ROOT::RVec<typename ValueTypeHelper<typename std::tuple_element_t<I, Tuple<Ts...>>::Type>::Type>,
                                        typename std::tuple_element_t<I, Tuple<Ts...>>::Type>;

public:
    TakeOne(std::vector<std::shared_ptr<Tuple<Ts...>>>& data, gslx::index_sequence<Is...>) :
        fData{data} {}

    auto operator()(const ReadType<Is>&... value) -> void {
        fData.emplace_back(std::make_shared<Tuple<Ts...>>(value...));
    }

private:
    std::vector<std::shared_ptr<Tuple<Ts...>>>& fData;
};

} // namespace Mustard::Data
