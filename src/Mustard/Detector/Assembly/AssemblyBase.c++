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

#include "Mustard/Detector/Assembly/AssemblyBase.h++"
#include "Mustard/Detector/Definition/DefinitionBase.h++"
#include "Mustard/IO/PrettyLog.h++"

#include "muc/hash_set"

namespace Mustard::Detector::Assembly {

auto AssemblyBase::Get(std::type_index definition) const -> const Definition::DefinitionBase& {
    for (auto&& [topType, top] : fTop) {
        if (topType == definition) {
            return *top;
        }
        const auto descendant{top->FindDescendant(definition)};
        if (descendant) {
            return *descendant;
        }
    }
    Throw<std::logic_error>(fmt::format("No {} in assembly", muc::try_demangle(definition.name())));
}

auto AssemblyBase::Get(std::type_index definition) -> Definition::DefinitionBase& {
    for (auto&& [topType, top] : fTop) {
        if (topType == definition) {
            return *top;
        }
        const auto descendant{top->FindDescendant(definition)};
        if (descendant) {
            return *descendant;
        }
    }
    Throw<std::logic_error>(fmt::format("No {} in assembly", muc::try_demangle(definition.name())));
}

auto AssemblyBase::TopComplete() -> void {
    // check not empty
    if (fTop.empty()) {
        Throw<std::logic_error>("Empty assembly");
    }
    // check not mother-daughter
    for (auto&& [topType, pTop] : fTop) {
        const auto& top{*pTop};
        if (topType == typeid(top)) {
            continue;
        }
        for (auto&& [anotherTop, _] : fTop) {
            if (top.FindDaughter(anotherTop)) {
                Throw<std::logic_error>(fmt::format("{} is mother of {}",
                                                    muc::try_demangle(topType.name()),
                                                    muc::try_demangle(anotherTop.name())));
            }
        }
    }
    // check from same family
    muc::flat_hash_set<const Definition::DefinitionBase*> mother;
    mother.reserve(fTop.size());
    for (auto&& [_, top] : fTop) {
        const Definition::DefinitionBase* myMother{top.get()};
        while (not myMother->Topmost()) {
            myMother = &myMother->Mother();
        }
        mother.emplace(myMother);
    }
    if (mother.size() != 1) {
        Throw<std::logic_error>("Assembly has many first ancestors");
    }
}

auto AssemblyBase::DeleteIfTopmost::operator()(Definition::DefinitionBase* ptr) const -> void {
    if (ptr->Topmost()) {
        delete ptr;
    }
}

} // namespace Mustard::Detector::Assembly
