#include "Mustard/Extension/Geant4X/Run/MPIRunManager.h++"
#include "Mustard/Extension/Geant4X/Run/MPIRunMessenger.h++"

#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcommand.hh"
#include "G4UIdirectory.hh"

namespace Mustard::inline Extension::Geant4X::inline Run {

MPIRunMessenger::MPIRunMessenger() :
    SingletonMessenger{},
    fDirectory{},
    fPrintProgress{},
    fPrintProgressModulo{},
    fPrintRunSummary{} {

    fDirectory = std::make_unique<G4UIdirectory>("/MPIRun/");
    fDirectory->SetGuidance("Specialized settings for MPIRunManager.");

    fPrintProgress = std::make_unique<G4UIcmdWithABool>("/MPIRun/PrintProgress", this);
    fPrintProgress->SetGuidance("Set whether to display the run progress. /run/printprogress is disabled once this is set.");
    fPrintProgress->SetParameterName("b", false);
    fPrintProgress->AvailableForStates(G4State_PreInit, G4State_Idle);

    fPrintProgressModulo = std::make_unique<G4UIcmdWithAnInteger>("/MPIRun/PrintProgressModulo", this);
    fPrintProgressModulo->SetGuidance("Set display frequency of run progress. If set to 0, the frequency is adaptive. Progress will not be displayed if set to <0. /run/printprogress is disabled once this is set.");
    fPrintProgressModulo->SetParameterName("modulo", false);
    fPrintProgressModulo->AvailableForStates(G4State_PreInit, G4State_Idle);

    fPrintRunSummary = std::make_unique<G4UIcommand>("/MPIRun/PrintRunSummary", this);
    fPrintRunSummary->SetGuidance("Print MPI run performace summary.");
    fPrintRunSummary->AvailableForStates(G4State_Idle);
}

MPIRunMessenger::~MPIRunMessenger() = default;

auto MPIRunMessenger::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fPrintProgress.get()) {
        Deliver<MPIRunManager>([&](auto&& r) {
            r.PrintProgress(fPrintProgress->GetNewBoolValue(value));
        });
    } else if (command == fPrintProgressModulo.get()) {
        Deliver<MPIRunManager>([&](auto&& r) {
            r.PrintProgressModulo(fPrintProgressModulo->GetNewIntValue(value));
        });
    } else if (command == fPrintRunSummary.get()) {
        Deliver<MPIRunManager>([&](auto&& r) {
            r.PrintRunSummary();
        });
    }
}

} // namespace Mustard::inline Extension::Geant4X::inline Run
