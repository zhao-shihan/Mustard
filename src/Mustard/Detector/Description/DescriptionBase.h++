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

#pragma once

#include "Mustard/Detector/Description/Description.h++"
#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/Env/Memory/Singleton.h++"
#include "Mustard/Utility/NonMoveableBase.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "yaml-cpp/yaml.h"

#include "gsl/gsl"

#include <array>
#include <concepts>
#include <iostream>
#include <optional>
#include <string>
#include <utility>

namespace Mustard::Detector::Description {

template<typename... Ts>
    requires(sizeof...(Ts) <= 1)
class DescriptionBase;

template<>
class DescriptionBase<> : public NonMoveableBase {
protected:
    DescriptionBase(std::string name);
    ~DescriptionBase() = default;

public:
    const auto& Name() const { return fName; }

    auto Import(const YAML::Node& rootNode) -> void;
    auto Export(YAML::Node& rootNode) const -> void;

protected:
    template<typename AValue, typename AReadAs = AValue>
        requires std::assignable_from<AValue&, AReadAs>
    auto ImportValue(const YAML::Node& node, AValue& value, std::convertible_to<std::string> auto&&... names) -> void;
    template<typename AReadAs>
    auto ImportValue(const YAML::Node& node, std::invocable<AReadAs> auto&& ImportAction, std::convertible_to<std::string> auto&&... names) -> void;
    template<typename AValue, typename AWriteAs = AValue>
        requires std::convertible_to<const AValue&, AWriteAs>
    auto ExportValue(YAML::Node& node, const AValue& value, std::convertible_to<std::string> auto&&... names) const -> void;

private:
    virtual void ImportAllValue(const YAML::Node& node) = 0;
    virtual void ExportAllValue(YAML::Node& node) const = 0;

    auto UnpackToLeafNodeForImporting(const YAML::Node& node, std::convertible_to<std::string> auto&&... names) -> std::optional<const YAML::Node>;
    auto UnpackToLeafNodeForExporting(YAML::Node& node, std::convertible_to<std::string> auto&&... names) const -> YAML::Node;
    auto PrintNodeNotFoundNotice(std::convertible_to<std::string> auto&&... names) const -> void;

private:
    std::string fName;
};

template<typename ADerived>
class DescriptionBase<ADerived> : public Env::Memory::Singleton<ADerived>,
                                  public DescriptionBase<> {
protected:
    DescriptionBase(std::string name);
    ~DescriptionBase() = default;
};

} // namespace Mustard::Detector::Description

#include "Mustard/Detector/Description/DescriptionBase.inl"
