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

#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Extension/Geant4X/Interface/MPIExecutive.h++"
#include "Mustard/Utility/PrettyLog.h++"
#include "Mustard/Utility/Print.h++"

#include <ostream>
#include <source_location>
#include <stdexcept>

namespace Mustard::inline Extension::Geant4X::inline Interface {

auto MPIExecutive::CheckSequential() const -> void {
    const auto& mpiEnv = Env::MPIEnv::Instance();
    if (mpiEnv.Parallel()) {
        if (mpiEnv.OnCommWorldMaster()) {
            G4Exception(std::source_location::current().function_name(),
                        "InteractiveSessionMustBeSequential",
                        JustWarning,
                        "Interactive session must be run with only 1 process.\nThrowing an instance of std::logic_error.");
        }
        Throw<std::logic_error>("Interactive session must be sequential");
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

} // namespace Mustard::inline Extension::Geant4X::inline Interface
