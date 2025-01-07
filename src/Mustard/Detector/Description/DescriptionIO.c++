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

#include "Mustard/Detector/Description/DescriptionIO.h++"

namespace Mustard::Detector::Description {

std::set<gsl::not_null<DescriptionBase<>*>> DescriptionIO::fgInstanceSet{};

auto DescriptionIO::Output(const YAML::Node& geomYaml, const std::string& fileComment, std::ostream& os) -> void {
    YAML::Emitter yamlEmitter{os};
    yamlEmitter << YAML::Block;
    if (not fileComment.empty()) {
        yamlEmitter << YAML::Comment(fileComment)
                    << YAML::Newline;
    }
    yamlEmitter << geomYaml
                << YAML::Newline;
}

} // namespace Mustard::Detector::Description
