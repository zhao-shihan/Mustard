#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Env/Print.h++"
#include "Mustard/Extension/Geant4X/Interface/MPIExecutive.h++"

#include <ostream>
#include <stdexcept>

namespace Mustard::inline Extension::Geant4X::inline Interface {

auto MPIExecutive::CheckSequential() const -> void {
    const auto& mpiEnv = Env::MPIEnv::Instance();
    if (mpiEnv.Parallel()) {
        std::string where("Mustard::Geant4X::MPIExecutive::CheckSequential");
        if (mpiEnv.OnCommWorldMaster()) {
            G4Exception(where.c_str(),
                        "InteractiveSessionMustBeSequential",
                        JustWarning,
                        "Interactive session must be run with only 1 process.\nThrowing an instance of std::logic_error.");
        }
        throw std::logic_error(where.append(": Interactive session must be sequential"));
    }
}

auto MPIExecutive::ExecuteCommand(const std::string& command) -> bool {
    if (command.empty() or
        std::ranges::all_of(command.substr(0, command.find_first_of('#')),
                            [](unsigned char ch) {
                                return std::isspace(ch);
                            })) {
        Env::PrintLn(G4cout, "{}", command);
        return true;
    }
    if (const auto commandStatus = G4UImanager::GetUIpointer()->ApplyCommand(command);
        commandStatus == fCommandSucceeded) [[likely]] {
        return true;
    } else {
        Env::PrintLn(G4cerr, "Mustard::Geant4X::MPIExecutive::Execute: Command '{}' failed (G4UIcommandStatus: {})", command, commandStatus), flush(G4cerr);
        return false;
    }
}

} // namespace Mustard::inline Extension::Geant4X::inline Interface
