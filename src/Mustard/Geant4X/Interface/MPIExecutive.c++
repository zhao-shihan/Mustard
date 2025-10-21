// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

#include "Mustard/Geant4X/Interface/MPIExecutive.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/IO/Print.h++"

#include "mplr/mplr.hpp"

#include <ostream>
#include <source_location>
#include <stdexcept>

namespace Mustard::Geant4X::inline Interface {

MPIExecutive::MPIExecutive() :
    WeakSingleton{this},
    fIsInteractive{} {}

auto MPIExecutive::CheckSequential() const -> void {
    const auto worldComm{mplr::comm_world()};
    if (worldComm.size() == 1) {
        return;
    }
    if (worldComm.rank() == 0) {
        G4Exception(std::source_location::current().function_name(),
                    "InteractiveSessionMustBeSequential",
                    JustWarning,
                    "Interactive session must be run with only 1 process.\nThrowing an instance of std::logic_error.");
    }
    Throw<std::logic_error>("Interactive session must be sequential");
}

auto MPIExecutive::Execute(const std::string& macro) -> void {
    G4UImanager::GetUIpointer()->ExecuteMacroFile(macro.c_str());
    if (G4UImanager::GetUIpointer()->GetLastReturnCode() == fParameterUnreadable) {
        if (not fIsInteractive) {
            Throw<std::runtime_error>("Executing macro file failed.");
        }
    }
}

auto MPIExecutive::ExecuteCommand(const std::string& command) -> bool {
    if (command.empty() or
        std::ranges::all_of(command.substr(0, command.find_first_of('#')),
                            [](unsigned char ch) {
                                return std::isspace(ch);
                            })) {
        PrintLn(G4cout, "{}", command);
        return true;
    }
    if (const auto commandStatus = G4UImanager::GetUIpointer()->ApplyCommand(command);
        commandStatus == fCommandSucceeded) [[likely]] {
        return true;
    } else {
        PrintLn(G4cerr, "Mustard::Geant4X::MPIExecutive::Execute: Command '{}' failed (G4UIcommandStatus: {})", command, commandStatus), flush(G4cerr);
        return false;
    }
}

} // namespace Mustard::Geant4X::inline Interface
