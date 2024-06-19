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

#include "Mustard/Detector/Description/DescriptionBase.h++"
#include "Mustard/Detector/Description/DescriptionIO.h++"

namespace Mustard::Detector::Description {

DescriptionBase<>::DescriptionBase(std::string name) :
    NonMoveableBase{},
    fName{std::move(name)} {
    DescriptionIO::AddInstance(this);
}

auto DescriptionBase<>::Import(const YAML::Node& rootNode) -> void {
    const auto node{rootNode[fName]};
    if (node.IsDefined()) {
        ImportAllValue(node);
    } else {
        PrintNodeNotFoundWarning();
    }
}

auto DescriptionBase<>::Export(YAML::Node& rootNode) const -> void {
    auto node{rootNode[fName]};
    ExportAllValue(node);
}

} // namespace Mustard::Detector::Description
