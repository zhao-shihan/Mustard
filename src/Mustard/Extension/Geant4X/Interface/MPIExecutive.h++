#pragma once

#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/Env/CLI/Geant4CLI.h++"
#include "Mustard/Env/Memory/WeakSingleton.h++"

#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"

#include <algorithm>
#include <initializer_list>
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>

namespace Mustard::inline Extension::Geant4X::inline Interface {

using Env::CLI::Geant4CLI;

class MPIExecutive final : public Env::Memory::WeakSingleton<MPIExecutive> {
public:
    MPIExecutive() = default; // prevent aggregate initialization

    auto StartSession(const Geant4CLI& cli, auto&& macFileOrCmdList) const -> void;
    template<typename T>
    auto StartSession(const Geant4CLI& cli, std::initializer_list<T> cmdList = {}) const -> void;

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
    auto StartSessionImpl(const Geant4CLI& cli, auto&& macFileOrCmdList) const -> void;
    auto StartSessionImpl(int argc, char* argv[], auto&& macFileOrCmdList) const -> void;
    auto StartInteractiveSessionImpl(int argc, char* argv[], auto&& macFileOrCmdList) const -> void;
    auto StartBatchSessionImpl(auto&& macFileOrCmdList) const -> void;

    auto CheckSequential() const -> void;

    static auto ExecuteCommand(const std::string& command) -> bool;

    static auto Execute(const std::string& macro) -> void { G4UImanager::GetUIpointer()->ExecuteMacroFile(macro.c_str()); }
    static auto Execute(const std::ranges::input_range auto& cmdList) -> void
        requires std::convertible_to<typename std::decay_t<decltype(cmdList)>::value_type, std::string>;
};

} // namespace Mustard::inline Extension::Geant4X::inline Interface

#include "Mustard/Extension/Geant4X/Interface/MPIExecutive.inl"
