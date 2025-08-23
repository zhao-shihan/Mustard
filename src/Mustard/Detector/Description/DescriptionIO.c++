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

#include "Mustard/Detector/Description/DescriptionIO.h++"

namespace Mustard::Detector::Description {

std::set<gsl::not_null<DescriptionBase<>*>> DescriptionIO::fgInstanceSet{};

auto DescriptionIO::EmitYAML(const YAML::Node& geomYaml, const std::string& fileComment, std::ostream& os) -> void {
    YAML::Emitter yamlEmitter{os};
    if (not fileComment.empty()) {
        yamlEmitter << YAML::Comment(fileComment)
                    << YAML::Newline;
    }
    EmitYAMLImpl(geomYaml, yamlEmitter);
    yamlEmitter << YAML::Newline;
}

auto DescriptionIO::EmitYAMLImpl(const YAML::Node& node, YAML::Emitter& emitter, bool inFlow) -> void {
    if (not node.IsDefined() or node.IsNull()) {
        emitter << YAML::Null;
    } else if (node.IsSequence()) {
        emitter << YAML::Flow;
        emitter << YAML::BeginSeq;
        for (auto&& child : node) {
            EmitYAMLImpl(child, emitter, true);
        }
        emitter << YAML::EndSeq;
    } else if (node.IsMap()) {
        if (inFlow) { // if we are in flow, be in flow!
            emitter << YAML::Flow;
        }
        emitter << YAML::BeginMap;
        for (auto&& child : node) {
            emitter << YAML::Key;
            EmitYAMLImpl(child.first, emitter);
            emitter << YAML::Value;
            EmitYAMLImpl(child.second, emitter);
        }
        emitter << YAML::EndMap;
    } else if (node.IsScalar()) {
        emitter << node;
    }
}

} // namespace Mustard::Detector::Description
