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

#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/Env/CLI/CLI.h++"
#include "Mustard/Env/CLI/Module/Geant4Module.h++"
#include "Mustard/Env/CLI/Module/ModuleBase.h++"
#include "Mustard/Env/Memory/WeakSingleton.h++"

#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"

#include "muc/tuple"

#include <algorithm>
#include <concepts>
#include <initializer_list>
#include <iostream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

namespace Mustard::Geant4X::inline Interface {

class MPIExecutive : public Env::Memory::WeakSingleton<MPIExecutive> {
public:
    MPIExecutive();

    template<std::derived_from<Env::CLI::ModuleBase>... Ms>
        requires muc::tuple_contains_v<std::tuple<Ms...>, Env::CLI::Geant4Module>
    auto StartSession(const Env::CLI::CLI<Ms...>& cli, auto&& macFileOrCmdList) const -> void;
    template<std::derived_from<Env::CLI::ModuleBase>... Ms, typename T>
        requires muc::tuple_contains_v<std::tuple<Ms...>, Env::CLI::Geant4Module>
    auto StartSession(const Env::CLI::CLI<Ms...>& cli, std::initializer_list<T> cmdList = {}) const -> void;

    auto StartSession(int argc, char* argv[], auto&& macFileOrCmdList) const -> void;
    template<typename T>
    auto StartSession(int argc, char* argv[], std::initializer_list<T> cmdList = {}) const -> void;

    auto StartInteractiveSession(int argc, char* argv[], auto&& macFileOrCmdList) const -> void;
    template<typename T>
    auto StartInteractiveSession(int argc, char* argv[], std::initializer_list<T> cmdList = {}) const -> void;

    auto StartBatchSession(auto&& macFileOrCmdList) const -> void;
    template<typename T>
    auto StartBatchSession(std::initializer_list<T> cmdList) const -> void;

private:
    template<std::derived_from<Env::CLI::ModuleBase>... Ms>
        requires muc::tuple_contains_v<std::tuple<Ms...>, Env::CLI::Geant4Module>
    auto StartSessionImpl(const Env::CLI::CLI<Ms...>& cli, auto&& macFileOrCmdList) const -> void;
    auto StartSessionImpl(int argc, char* argv[], auto&& macFileOrCmdList) const -> void;
    auto StartInteractiveSessionImpl(int argc, char* argv[], auto&& macFileOrCmdList) const -> void;
    auto StartBatchSessionImpl(auto&& macFileOrCmdList) const -> void;

    auto CheckSequential() const -> void;

    static auto ExecuteCommand(const std::string& command) -> bool;

    static auto Execute(const std::string& macro) -> void { G4UImanager::GetUIpointer()->ExecuteMacroFile(macro.c_str()); }
    static auto Execute(const std::ranges::input_range auto& cmdList) -> void
        requires std::convertible_to<typename std::decay_t<decltype(cmdList)>::value_type, std::string>;
};

} // namespace Mustard::Geant4X::inline Interface

#include "Mustard/Geant4X/Interface/MPIExecutive.inl"
