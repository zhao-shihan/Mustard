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

namespace Mustard::Geant4X::inline Interface {

template<std::derived_from<Env::CLI::ModuleBase>... Ms>
    requires muc::tuple_contains_v<std::tuple<Ms...>, Env::CLI::Geant4Module>
auto MPIExecutive::StartSession(const Env::CLI::CLI<Ms...>& cli, auto&& macFileOrCmdList) const -> void {
    StartSessionImpl(cli, std::forward<decltype(macFileOrCmdList)>(macFileOrCmdList));
}

template<std::derived_from<Env::CLI::ModuleBase>... Ms, typename T>
    requires muc::tuple_contains_v<std::tuple<Ms...>, Env::CLI::Geant4Module>
auto MPIExecutive::StartSession(const Env::CLI::CLI<Ms...>& cli, std::initializer_list<T> cmdList) const -> void {
    StartSessionImpl(cli, cmdList);
}

auto MPIExecutive::StartSession(int argc, char* argv[], auto&& macFileOrCmdList) const -> void {
    StartSessionImpl(argc, argv, std::forward<decltype(macFileOrCmdList)>(macFileOrCmdList));
}

template<typename T>
auto MPIExecutive::StartSession(int argc, char* argv[], std::initializer_list<T> cmdList) const -> void {
    StartSessionImpl(argc, argv, cmdList);
}

auto MPIExecutive::StartInteractiveSession(int argc, char* argv[], auto&& macFileOrCmdList) const -> void {
    StartInteractiveSessionImpl(argc, argv, std::forward<decltype(macFileOrCmdList)>(macFileOrCmdList));
}

template<typename T>
auto MPIExecutive::StartInteractiveSession(int argc, char* argv[], std::initializer_list<T> cmdList) const -> void {
    StartInteractiveSessionImpl(argc, argv, cmdList);
}

auto MPIExecutive::StartBatchSession(auto&& macFileOrCmdList) const -> void {
    StartBatchSessionImpl(std::forward<decltype(macFileOrCmdList)>(macFileOrCmdList));
}

template<typename T>
auto MPIExecutive::StartBatchSession(std::initializer_list<T> cmdList) const -> void {
    StartBatchSessionImpl(cmdList);
}

template<std::derived_from<Env::CLI::ModuleBase>... Ms>
    requires muc::tuple_contains_v<std::tuple<Ms...>, Env::CLI::Geant4Module>
auto MPIExecutive::StartSessionImpl(const Env::CLI::CLI<Ms...>& cli, auto&& macFileOrCmdList) const -> void {
    if (cli.IsInteractive()) {
        const auto [argc, argv]{cli.ArgcArgv()};
        auto macro{cli.Macro()};
        if (macro) {
            StartInteractiveSessionImpl(argc, argv, std::move(*macro));
        } else {
            StartInteractiveSessionImpl(argc, argv, std::forward<decltype(macFileOrCmdList)>(macFileOrCmdList));
        }
    } else {
        StartBatchSessionImpl(*cli.Macro());
    }
}

auto MPIExecutive::StartSessionImpl(int argc, char* argv[], auto&& macFileOrCmdList) const -> void {
    if (argc == 1) {
        StartInteractiveSessionImpl(argc, argv, std::forward<decltype(macFileOrCmdList)>(macFileOrCmdList));
    } else {
        StartBatchSessionImpl(argv[1]);
    }
}

auto MPIExecutive::StartInteractiveSessionImpl(int argc, char* argv[], auto&& macFileOrCmdList) const -> void {
    CheckSequential();
#if MUSTARD_USE_G4VIS
    G4UIExecutive uiExecutive{argc, argv};
    G4VisExecutive visExecutive;
    visExecutive.Initialize();
#else
    G4UIExecutive uiExecutive{argc, argv, "tcsh"};
#endif
    Execute(std::forward<decltype(macFileOrCmdList)>(macFileOrCmdList));
    uiExecutive.SessionStart();
}

auto MPIExecutive::StartBatchSessionImpl(auto&& macFileOrCmdList) const -> void {
    Execute(std::forward<decltype(macFileOrCmdList)>(macFileOrCmdList));
}

auto MPIExecutive::Execute(const std::ranges::input_range auto& cmdList) -> void
    requires std::convertible_to<typename std::decay_t<decltype(cmdList)>::value_type, std::string>
{
    for (auto&& command : cmdList) {
        if (const auto success{ExecuteCommand(std::forward<decltype(command)>(command))};
            not success) {
            break;
        }
    }
}

} // namespace Mustard::Geant4X::inline Interface
