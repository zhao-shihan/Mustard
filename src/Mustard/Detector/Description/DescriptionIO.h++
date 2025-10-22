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

#include "Mustard/Detector/Description/Description.h++"
#include "Mustard/Detector/Description/DescriptionBase.h++"
#include "Mustard/IO/CreateTemporaryFile.h++"
#include "Mustard/IO/File.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Utility/NonConstructibleBase.h++"

#include "yaml-cpp/yaml.h"

#include "muc/tuple"

#include "gsl/gsl"

#include "fmt/format.h"

#include <algorithm>
#include <array>
#include <concepts>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace Mustard::Detector::Description {

class DescriptionIO final : public NonConstructibleBase {
public:
    template<Description... Ds>
    static auto Import(const std::filesystem::path& yamlPath) -> void;
    template<Description... Ds>
    static auto Export(const std::filesystem::path& yamlPath, const std::string& fileComment = {}) -> std::filesystem::path;
    template<Description... Ds>
    static auto Emport(const std::filesystem::path& yamlPath, const std::string& fileComment = {}) -> std::pair<std::filesystem::path, std::filesystem::path>;
    template<Description... Ds>
    static auto ToString() -> std::string;

    template<muc::tuple_like T>
    static auto Import(const std::filesystem::path& yamlPath) -> void;
    template<muc::tuple_like T>
    static auto Export(const std::filesystem::path& yamlPath, const std::string& fileComment = {}) -> std::filesystem::path;
    template<muc::tuple_like T>
    static auto Emport(const std::filesystem::path& yamlPath, const std::string& fileComment = {}) -> std::pair<std::filesystem::path, std::filesystem::path>;
    template<muc::tuple_like T>
    static auto ToString() -> std::string;

    template<typename... ArgsOfImport>
    static auto Import(const std::ranges::range auto& yamlText) -> void
        requires std::convertible_to<typename std::decay_t<decltype(yamlText)>::value_type, std::string>;

    static auto AddInstance(gsl::not_null<DescriptionBase<>*> instance) -> void;
    static auto ImportInstantiated(const std::filesystem::path& yamlPath) -> void;
    static auto ExportInstantiated(const std::filesystem::path& yamlPath, const std::string& fileComment = {}) -> std::filesystem::path;

private:
    static auto ImportImpl(const std::filesystem::path& yamlPath, std::ranges::input_range auto& descriptions) -> void;
    static auto ExportImpl(const std::filesystem::path& yamlPath, const std::string& fileComment, const std::ranges::input_range auto& descriptions) -> std::filesystem::path;
    static auto EmportImpl(const std::filesystem::path& yamlPath, const std::string& fileComment, const std::ranges::input_range auto& descriptions) -> std::pair<std::filesystem::path, std::filesystem::path>;
    static auto ToStringImpl(const std::ranges::input_range auto& descriptions) -> std::string;
    static auto EmitYAML(const YAML::Node& geomYaml, const std::string& fileComment, std::ostream& os) -> void;
    static auto EmitYAMLImpl(const YAML::Node& node, YAML::Emitter& emitter, bool inFlow = false) -> void;

private:
    static std::set<gsl::not_null<DescriptionBase<>*>> fgInstanceSet;
};

} // namespace Mustard::Detector::Description

#include "Mustard/Detector/Description/DescriptionIO.inl"
