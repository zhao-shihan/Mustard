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

#include "Mustard/CLI/CLI.h++"
#include "Mustard/CLI/Module/Geant4Module.h++"
#include "Mustard/CLI/Module/ModuleBase.h++"
#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/Env/Memory/WeakSingleton.h++"
#include "Mustard/IO/PrettyLog.h++"

#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"

#include "muc/type_traits"

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

    template<std::derived_from<CLI::ModuleBase>... Ms>
        requires muc::tuple_contains_v<std::tuple<Ms...>, CLI::Geant4Module>
    auto StartSession(const CLI::CLI<Ms...>& cli, auto&& macFileOrCmdList) -> void;
    template<std::derived_from<CLI::ModuleBase>... Ms, typename T>
        requires muc::tuple_contains_v<std::tuple<Ms...>, CLI::Geant4Module>
    auto StartSession(const CLI::CLI<Ms...>& cli, std::initializer_list<T> cmdList = {}) -> void;

    auto StartSession(int argc, char* argv[], auto&& macFileOrCmdList) -> void;
    template<typename T>
    auto StartSession(int argc, char* argv[], std::initializer_list<T> cmdList = {}) -> void;

    auto StartInteractiveSession(int argc, char* argv[], auto&& macFileOrCmdList) -> void;
    template<typename T>
    auto StartInteractiveSession(int argc, char* argv[], std::initializer_list<T> cmdList = {}) -> void;

    auto StartBatchSession(auto&& macFileOrCmdList) -> void;
    template<typename T>
    auto StartBatchSession(std::initializer_list<T> cmdList) -> void;

private:
    template<std::derived_from<CLI::ModuleBase>... Ms>
        requires muc::tuple_contains_v<std::tuple<Ms...>, CLI::Geant4Module>
    auto StartSessionImpl(const CLI::CLI<Ms...>& cli, auto&& macFileOrCmdList) -> void;
    auto StartSessionImpl(int argc, char* argv[], auto&& macFileOrCmdList) -> void;
    auto StartInteractiveSessionImpl(int argc, char* argv[], auto&& macFileOrCmdList) -> void;
    auto StartBatchSessionImpl(auto&& macFileOrCmdList) -> void;

    auto CheckSequential() const -> void;

    const auto Execute(const std::string& macro) -> void;
    const auto Execute(const std::ranges::input_range auto& cmdList) -> void
        requires std::convertible_to<typename std::decay_t<decltype(cmdList)>::value_type, std::string>;

    static auto ExecuteCommand(const std::string& command) -> bool;

private:
    bool fIsInteractive;
};

} // namespace Mustard::Geant4X::inline Interface

#include "Mustard/Geant4X/Interface/MPIExecutive.inl"
