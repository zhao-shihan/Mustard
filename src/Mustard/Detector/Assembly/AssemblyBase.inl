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

namespace Mustard::Detector::Assembly {

template<std::derived_from<Definition::DefinitionBase> ADefinition>
    requires(not std::same_as<ADefinition, Definition::DefinitionBase>)
auto AssemblyBase::AddTop(ADefinition& top) -> void {
    const auto [_, inserted]{fTop.try_emplace(typeid(ADefinition), &top)};
    if (not inserted) {
        Throw<std::logic_error>(fmt::format("{} added twice", muc::try_demangle(typeid(ADefinition).name())));
    }
}

template<std::derived_from<Definition::DefinitionBase> ADefinition>
    requires(not std::same_as<ADefinition, Definition::DefinitionBase>)
auto AssemblyBase::AddTop(std::unique_ptr<ADefinition> top) -> void {
    if (not top->Topmost()) {
        Throw<std::logic_error>(fmt::format("{} is not topmost", muc::try_demangle(typeid(ADefinition).name())));
    }
    AddTop(*top.release());
}

} // namespace Mustard::Detector::Assembly
