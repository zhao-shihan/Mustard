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
#include "Mustard/Env/Memory/Singletonified.h++"
#include "Mustard/Env/Print.h++"
#include "Mustard/Utility/NonMoveableBase.h++"

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
    template<typename AValue, typename AReadAs = AValue, std::convertible_to<std::string>... AStrings>
        requires std::assignable_from<AValue&, AReadAs>
    auto ImportValue(const YAML::Node& node, AValue& value, AStrings&&... nodeNames) -> void;
    template<typename AReadAs, std::convertible_to<std::string>... AStrings>
    auto ImportValue(const YAML::Node& node, const std::regular_invocable<AReadAs> auto& ImportAction, AStrings&&... nodeNames) -> void;
    template<typename AValue, typename AWriteAs = AValue, std::convertible_to<std::string>... AStrings>
        requires std::convertible_to<const AValue&, AWriteAs>
    auto ExportValue(YAML::Node& node, const AValue& value, AStrings&&... nodeNames) const -> void;

private:
    virtual void ImportAllValue(const YAML::Node& node) = 0;
    virtual void ExportAllValue(YAML::Node& node) const = 0;

    template<std::convertible_to<std::string>... AStrings>
    auto UnpackToLeafNodeForImporting(const YAML::Node& node, AStrings&&... nodeNames) -> std::optional<const YAML::Node>;
    template<std::convertible_to<std::string>... AStrings>
    auto UnpackToLeafNodeForExporting(YAML::Node& node, AStrings&&... nodeNames) const -> YAML::Node;
    template<std::convertible_to<std::string>... AStrings>
    auto PrintNodeNotFoundNotice(AStrings&&... nodeNames) const -> void;

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
