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

namespace Mustard::Data::internal {

template<muc::instantiated_from<Tuple> ATuple>
BranchHelper<ATuple>::BranchHelper(ATuple& tuple) :
    fTuple{&tuple},
    fClassPointer{} {}

template<muc::instantiated_from<Tuple> ATuple>
template<muc::ceta_string AName>
auto BranchHelper<ATuple>::CreateBranch(std::derived_from<TTree> auto& tree) -> TBranch* {
    using ObjectType = typename ATuple::Model::template ValueOf<AName>::Type;
    ObjectType& object{*Get<AName>(*fTuple)};
    if constexpr (Concept::ROOTFundamental<ObjectType> or IsStdArray<ObjectType>{}) {
        return tree.Branch(AName, &object);
    } else if constexpr (std::is_class_v<ObjectType>) {
        constexpr auto i = ATuple::Model::template Index<AName>();
        return tree.Branch(AName, &(std::get<i>(fClassPointer) = std::addressof(object)));
    }
}

template<muc::instantiated_from<Tuple> ATuple>
template<muc::ceta_string AName>
auto BranchHelper<ATuple>::ConnectBranch(std::derived_from<TTree> auto& tree) -> std::pair<int, TBranch*> {
    using ObjectType = typename ATuple::Model::template ValueOf<AName>::Type;
    ObjectType& object{*Get<AName>(*fTuple)};
    int ec{};
    TBranch* branch{};
    if constexpr (Concept::ROOTFundamental<ObjectType> or IsStdArray<ObjectType>{}) {
        ec = tree.SetBranchAddress(AName, &object, &branch);
    } else if constexpr (std::is_class_v<ObjectType>) {
        constexpr auto i{ATuple::Model::template Index<AName>()};
        ec = tree.SetBranchAddress(AName, &(std::get<i>(fClassPointer) = std::addressof(object)), &branch);
    }
    return {ec, branch};
}

template<muc::instantiated_from<Tuple> ATuple>
template<muc::ceta_string AName>
auto BranchHelper<ATuple>::ConnectBranchNoCheck(std::derived_from<TTree> auto& tree) -> TBranch* {
    using ObjectType = typename ATuple::Model::template ValueOf<AName>::Type;
    ObjectType& object{*Get<AName>(*fTuple)};
    void* objectPointer{};
    if constexpr (Concept::ROOTFundamental<ObjectType> or IsStdArray<ObjectType>{}) {
        objectPointer = &object;
    } else if constexpr (std::is_class_v<ObjectType>) {
        constexpr auto i{ATuple::Model::template Index<AName>()};
        objectPointer = &(std::get<i>(fClassPointer) = std::addressof(object));
    }
    TBranch* branch{};
    tree.SetBranchAddress(AName, objectPointer, &branch);
    return branch;
}

} // namespace Mustard::Data::internal
