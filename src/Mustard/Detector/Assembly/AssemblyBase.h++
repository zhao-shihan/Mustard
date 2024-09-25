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

#include "Mustard/Utility/PrettyLog.h++"

#include "fmt/core.h"

#include <concepts>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>

namespace Mustard::Detector {

namespace Definition {
class DefinitionBase;
} // namespace Definition

namespace Assembly {

/// @brief Represents a set of `Detector::Definition`.
/// This is Basically a wrapper on multiple `Detector::Definition` objects that
/// have the same mother, but requires to have a type `DescriptionInUse`
/// including all affective Detector::Description type (see concept `Assembly`).
/// This provides a *pimpl* tool for complex geometries.
class AssemblyBase {
public:
    virtual ~AssemblyBase() = default;

    auto Top(std::type_index definition) const -> const auto& { return *fTop.at(definition); }
    auto Top(std::type_index definition) -> auto& { return *fTop.at(definition); }

    template<std::derived_from<Definition::DefinitionBase> ADefinition>
    auto Top() const -> const auto& { return dynamic_cast<const ADefinition&>(Top(typeid(ADefinition))); }
    template<std::derived_from<Definition::DefinitionBase> ADefinition>
    auto Top() -> auto& { return dynamic_cast<ADefinition&>(Top(typeid(ADefinition))); }

    auto Get(std::type_index definition) const -> const Definition::DefinitionBase&;
    auto Get(std::type_index definition) -> Definition::DefinitionBase&;

    template<std::derived_from<Definition::DefinitionBase> ADefinition>
    auto Get() const -> const auto& { return dynamic_cast<const ADefinition&>(Get(typeid(ADefinition))); }
    template<std::derived_from<Definition::DefinitionBase> ADefinition>
    auto Get() -> auto& { return dynamic_cast<ADefinition&>(Get(typeid(ADefinition))); }

protected:
    /// @brief Add a `Detector::Definition` object into this assembly. Should be called in
    /// constructor of derived class.
    /// @param top A top volume constructed from `Detector::Definition::NewDaughter`.
    template<std::derived_from<Definition::DefinitionBase> ADefinition>
        requires(not std::same_as<ADefinition, Definition::DefinitionBase>)
    auto AddTop(ADefinition& top) -> void;
    /// @brief Add a `Detector::Definition` object into this assembly. Should be called in
    /// constructor of derived class.
    /// @param top A top (and topmost!) volume constructed from `new`.
    template<std::derived_from<Definition::DefinitionBase> ADefinition>
        requires(not std::same_as<ADefinition, Definition::DefinitionBase>)
    auto AddTop(std::unique_ptr<ADefinition> top) -> void;
    /// @brief Inform `AssemblyBase` that all top definitions have been added. Should be
    /// called in constructor of derived class, after all `AddTop` call.
    auto TopComplete() -> void;

private:
    struct DeleteIfTopmost {
        auto operator()(Definition::DefinitionBase* ptr) const -> void;
    };

private:
    std::unordered_map<std::type_index, std::unique_ptr<Definition::DefinitionBase, DeleteIfTopmost>> fTop;
};

} // namespace Assembly

} // namespace Mustard::Detector

#include "Mustard/Detector/Assembly/AssemblyBase.inl"
